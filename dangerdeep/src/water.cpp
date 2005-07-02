// (ocean) water simulation and display (OpenGL)
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "oglext/OglExt.h"
#include <glu.h>
#include <SDL.h>

#include "water.h"
#include "texture.h"
#include "global_data.h"
#include "matrix4.h"
#include "cfg.h"
#include "system.h"
#include <fstream>

// compute projected grid efficiency, it should be 50-95%
#undef  COMPUTE_EFFICIENCY
//fixme: the wave sub detail causes ripples in water height leading to the specular map spots
//and other visual errors.
//solution: check bugs, use perlin noise for sub detail, not fft, especially noise with
//low variation of values, i.e. ripples/waves with roughly the same height not large waves and small
//waves as with fft.

// for testing
//#define DRAW_WATER_AS_GRID

// some more interesting values: phase 256, facesperwave 64+,
// wavelength 256+,
#define WAVE_PHASES 256		// no. of phases for wave animation
#define WAVE_LENGTH 128.0	// in meters, total length of one wave tile, fixme with heigher values (>= 512) waves look
                                // MUCH more realistic. especially with longer times and dynamic computation waves could
                                // become much nicer. noise can be used for the smaller detail in the near then.
#define TIDECYCLE_TIME 120.0	// seconds

#define FOAM_VANISH_FACTOR 0.1	// 1/second until foam goes from 1 to 0.
#define FOAM_SPAWN_FACTOR 0.2	// 1/second until full foam reached. maybe should be equal to vanish factor

#define REFRAC_COLOR_RES 32
#define FRESNEL_FCT_RES 256
#define WATER_BUMP_DETAIL_HEIGHT 4.0

#define WAVE_TILE_GEN_TIME 0.04		// 25fps
#define SUBDETAIL_GEN_TIME 0.066667	// 15fps

#define FOAMAMOUNTRES 256

/*
	2004/05/06
	The foam problem.
	Earlier plans were that foam is just white color added to the water. That way we could
	keep an texture that is updated every frame and holds information in a rectangular grid
	wether there is foam or not. That way we don't need to send foam data via colors,
	texture coordinates etc. This is no good idea: the foam looks too bad, and we would need
	an texture that covers the whole water area (texture would be too big). No way.
	So the foam must be mixed into the water somehow. Let's store a foam texture in the
	second texture unit. How do we mix it in? GL_INTERPOLATE only works with source color's
	alpha channel, and the only remaining source is the primary color. Interpolating it
	with the primary color's color channels works on Geforce cards but is not OpenGL standard
	conform. But let's assume we could mix it somehow. Where do we get the amout of foam to
	mix in? 1) from the wave animation 2) from the ship's wakes etc.
	1) with projected grid this would look ugly, because in the distance the grid becomes
	very coarse. Just apply foam for water nearer than some distance value? difficult.
	2) There are two possibilities.
	a) use one large 2d array that holds the foam levels for the world. Each frame foam
	values must get decreased by a constant amount. The array would be too large.
	b) compute the amount of foam for each drawn vertex. To do that we assume that each
	foam source influenced each vertex.
	Simply write a function that gives the amount of foam at any position x caused by a
	foam source s_i. For each vertex sum up the foam values of all sources:
	foam_strength(x) := sum_i foam_strength_at(x, i), clamp the result at 1.
	Maybe add some heuristics: if foam source i is too far away, ignore it.
	Nontheless, this is very costly. We have at least 128x128 vertices to draw. If there
	are twenty ships (a common value!) we have to compute the function 20 times for each
	of the 16k vertices, and each computation would probably need a sqrt (for distance).
	THIS is really much for a PC.
	Another way doing b): clear the screen with black. Draw the ship's trails as quad strips
	with white quads with varying alpha values (amount of foam = alpha value). The GPU
	computes the mixing for us. Finally we have an grey value image. Read it to CPU memory.
	We may now downsample it (faked Anti-aliasing). Now look up the amount of foam on each
	grid point in the image (we could render the image witht the same resolution as the
	grid, or the double resolution for faked AA). And voila. But how to mix the foam? all
	Alpha channels are gone ... no! Store these values in the Alpha channel of the reflection
	map. The reflection map is computed the same way as the grey value map. Good! The only
	problem that remains is how to create the RGBA reflection map from RGB values of one
	scene and the A values of another. Use color masking for that (Mask bits before writing
	in the buffer).
	That way we don't need to copy the values back from GPU to CPU.
	Take the foam image (b/w with alpha, only alpha is interesting) as texture and
	project it onto the screen like the reflections (proj-modelview-mat as texture-matrix)
	mix foam according to alpha value. Problem: while drawing waves the foam is shifted
	up or down, per pixel foam lookup doesn't work right. so the texture coordinates
	for foam lookup must get transformed according to triangulation.
	Don't take real 3d coords as texcoords for foam tex, but keep x,y, set z to zero.
	Then it works.
	Would need 2 passes with new fresnel technique (2nd unit trashed).
	Would need 2 passes ANYWAY because foam tex coords are different from reflection
	tex coords!
	So do: 1) draw mirror image -> texture R
	2) draw foam image -> texture F
	3) draw world
	4) draw water first pass (reflection, fresnel)
	5) draw water second pass (foam) (maybe some extra color..., fine foam lines texture ?)
*/	

/*
2005/05/30
water rendering and shaders.
it works but there are two major problems:
1) faces that are nearly perpendicular to the viewer with normal mapping applied can have
normals that are facing away from the viewer, which leads to larger unichrome surfaces,
because of the saturation when computing E*N. Taking the absolute value of the E*N helps
to avoid these surfaces but the result is also not satisfying.
2) Much worse, either texture coordinate interpolation or texture value interpolation
is not precise enough, leading to large steps from black to white when drawing specular
highlight grey gradients. Lookup textures do not help, because POW is not the problem here.

solution: no real one yet.

Only way: do NOT use normal maps, but more faces. Higher face count leads to more CPU
usage, which can be cut down with shaders, AGP bus load will be similar as without shaders,
because the GPU can compute many values the CPU would have to compute without shaders.

Seems to be the only way out... use perlin noise for additional detail and use 128x256 or more
faces.
AND more important: increase efficiency when drawing faces, at the moment it is about 50%,
increasing that to 80% or more will improve rendering quality, no matter if shaders are used or
not.
*/

water::water(unsigned xres_, unsigned yres_, double tm) :
	mytime(tm),
	xres(xres_),
	yres(yres_),
	wavetileheights(3),//3 phases needed
	wavetiledisplacements(3),//3 phases needed
	last_light_brightness(-10000),
	wave_resolution(nextgteqpow2(cfg::instance().geti("wave_fft_res"))),
	wave_resolution_shift(ulog2(wave_resolution)),
	wave_subdetail(cfg::instance().getb("wave_subdetail")),
	subdetail_size(cfg::instance().geti("wave_subdetail_size")),
	subdetail_size_shift(ulog2(subdetail_size)),
	owg(wave_resolution, vector2f(1,1), 20 /*10*/ /*31*/, 2e-6 /* 5e-6 */, WAVE_LENGTH, TIDECYCLE_TIME),
	vertex_program_supported(false),
	fragment_program_supported(false),
	compiled_vertex_arrays_supported(false),
	use_shaders(false),
	water_vertex_program(0),
	water_fragment_program(0),
	png(subdetail_size, 8, subdetail_size),
	last_wave_gen_time(tm),
	last_subdetail_gen_time(tm)
{

	// 2004/04/25 Note! decreasing the size of the reflection map improves performance
	// on a gf4mx! (23fps to 28fps with a 128x128 map to a 512x512 map)
	// Maybe this is because of some bandwidth limit or cache efficiency of the gf4mx.
	// Reflections don't look very good on ocean waves because they're too high for
	// the distorsion to work correct. They need real environment mapping, but this lacks
	// local reflections. Best example: big waves in front of the coast should hide the
	// coast (view is blocked) but instead mirror it. Maybe this is because we don't clip
	// the mirror image at z=0 ? No, test showed that it isn't looking right anyway!
	// In fact, terrain is seen only at rare moments, vessels' reflections can't be seen
	// very well in rough seas and sun/moon/stars don't give large reflections.
	// They're used as fake specular mapping for now. Only explosions/fires at night would
	// be seen as reflections. So why use them at all? they're rather costly!
	// make them configureable? fixme
	// fixme: make size configurable in parts of screen resolution
	unsigned rx = sys().get_res_x();
	unsigned ry = sys().get_res_y();
	unsigned vps = texture::get_max_size();
	if (ry < vps)
		for (unsigned i = 1; i < ry; i *= 2) vps = i;
	// fixme: make ^ that configureable! reflection doesn't need to have that high detail...
	// fixme: auto mipmap?
	reflectiontex.reset(new texture(vps, vps, GL_RGB, texture::LINEAR, texture::CLAMP_TO_EDGE));

	sys().add_console("water rendering resolution %i x %i", xres, yres);
	sys().add_console("wave resolution %u (%u)",wave_resolution,wave_resolution_shift);
	sys().add_console("using subdetail: %s", wave_subdetail ? "yes" : "no");
	sys().add_console("subdetail size %u (%u)",subdetail_size,subdetail_size_shift);
	sys().add_console("reflection image size %u",vps);

	vertex_program_supported = sys().extension_supported("GL_ARB_vertex_program");
	fragment_program_supported = sys().extension_supported("GL_ARB_fragment_program");
	compiled_vertex_arrays_supported = sys().extension_supported("GL_EXT_compiled_vertex_array");

	use_shaders = vertex_program_supported && fragment_program_supported &&
		cfg::instance().getb("use_shaders") && cfg::instance().getb("use_shaders_for_water");

	// initialize shaders if wanted
	if (use_shaders) {
		water_fragment_program =
			texture::create_shader(GL_FRAGMENT_PROGRAM_ARB,
					       get_shader_dir() + "water_fp.shader");
		water_vertex_program =
			texture::create_shader(GL_VERTEX_PROGRAM_ARB,
					       get_shader_dir() + "water_vp.shader");
	}

	coords.resize((xres+1)*(yres+1));
	uv0.resize((xres+1)*(yres+1));
	uv1.resize((xres+1)*(yres+1));
	normals.resize((xres+1)*(yres+1));

	foamtex.reset(new texture(get_texture_dir() + "foam.png", texture::LINEAR, texture::REPEAT));//fixme maybe mipmap it

	foamamounttex.reset(new texture(FOAMAMOUNTRES, FOAMAMOUNTRES, GL_RGB, texture::LINEAR, texture::REPEAT));//CLAMP_TO_EDGE));

	foamamounttrail.reset(new texture(get_texture_dir() + "foamamounttrail.png", texture::LINEAR, texture::CLAMP_TO_EDGE));//fixme maybe mipmap it

	fresnelcolortexd.resize(FRESNEL_FCT_RES*REFRAC_COLOR_RES*4);
	for (unsigned f = 0; f < FRESNEL_FCT_RES; ++f) {
		float ff = float(f)/(FRESNEL_FCT_RES-1);
		//maybe reduce reflections by using 192 or 224 instead of 255 here
		//looks better! sea water shows less reflections in reality
		//because it is so rough.
		Uint8 a = Uint8(255 /*192*/ * exact_fresnel(ff)+0.5f);
		for (unsigned s = 0; s < REFRAC_COLOR_RES; ++s) {
			fresnelcolortexd[(s*FRESNEL_FCT_RES+f)*4+3] = a;
		}
	}
	
	// connectivity data is the same for all meshes and thus is reused
#ifdef DRAW_WATER_AS_GRID
	gridindices2.reserve(xres*yres*4);
	for (unsigned y = 0; y < yres; ++y) {
		unsigned y2 = y+1;
		for (unsigned x = 0; x < xres; ++x) {
			unsigned x2 = x+1;
			gridindices2.push_back(x +y *(xres+1));
			gridindices2.push_back(x2+y *(xres+1));
			gridindices2.push_back(x +y *(xres+1));
			gridindices2.push_back(x +y2*(xres+1));
		}
	}
#else
	gridindices.reserve(xres*yres*4);
	for (unsigned y = 0; y < yres; ++y) {
		unsigned y2 = y+1;
		for (unsigned x = 0; x < xres; ++x) {
			unsigned x2 = x+1;
			gridindices.push_back(x +y *(xres+1));
			gridindices.push_back(x2+y *(xres+1));
			gridindices.push_back(x2+y2*(xres+1));
			gridindices.push_back(x +y2*(xres+1));
		}
	}
#endif

	add_loading_screen("water maps inited");

	/*
	  Idea:
	  Computing one Height map with FFT takes roughly 2 ms on a 1800Mhz PC (64x64).
	  It has to be done 25 times per second, taking just 50ms or 5% of all time.
	  So the FFT heights could get computed on the fly, leading to more realistic
	  results, since they don't need to be cyclic. And we can change the fft parameters
	  at run-time (like switching weather).
	  Also much memory is saved. With 256 Phases of 64x64 each we have 1M with 1 byte per
	  height or 4M as we have now. With 128x128 fft resolution (much better than 64x64)
	  that would be already 4M/16M.
	  Heigher resolution fft could also be used as sub-noise for shader display
	  or as additional sub-detail (self-similar noise).
	  With on-the-fly fft we could give a cyclic value of 1-2 minutes.
	  Just blend the fft coefficients between two levels for weather changes, like
	  with the clouds.
	*/
	generate_wavetile();

	add_loading_screen("water height data computed");

	generate_subdetail_and_bumpmap();

	add_loading_screen("water bumpmap data computed");

	// compute specular lookup texture
#if 0
	const unsigned waterspecularlookup_res = 512;
	vector<Uint8> waterspecularlookup_tmp(waterspecularlookup_res);
	for (unsigned i = 0; i < waterspecularlookup_res; ++i)
		waterspecularlookup_tmp[i] = Uint8(255*pow((double(i)/(waterspecularlookup_res-1)), 50)+0.5);
	waterspecularlookup.reset(new texture(waterspecularlookup_tmp, waterspecularlookup_res, 1, GL_LUMINANCE,
					      texture::LINEAR, texture::CLAMP_TO_EDGE));
#endif
}


water::~water()
{
	if (use_shaders) {
		texture::delete_shader(water_fragment_program);
		texture::delete_shader(water_vertex_program);
	}
}


void water::setup_textures(const matrix4& reflection_projmvmat, const vector2f& transl) const
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glDisable(GL_LIGHTING);

	if (use_shaders) {
		glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, water_fragment_program);
		glEnable(GL_FRAGMENT_PROGRAM_ARB);
		glBindProgramARB(GL_VERTEX_PROGRAM_ARB, water_vertex_program);
		glEnable(GL_VERTEX_PROGRAM_ARB);

		glActiveTexture(GL_TEXTURE2);
		foamtex->set_gl_texture();

		glActiveTexture(GL_TEXTURE3);
		foamamounttex->set_gl_texture();


#if 0
		glActiveTexture(GL_TEXTURE2);
		waterspecularlookup->set_gl_texture();
#endif

		// texture units / coordinates:
		// tex0: noise map (color normals) / matching texcoords
		// tex1: reflection map / matching texcoords
		// tex2: foam
		// tex3: amount of foam
		glActiveTexture(GL_TEXTURE0);

		// set up texture matrix, so that texture coordinates can be computed from position.
		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		float noisetilescale = 32.0f;//meters
		glScalef(1.0f/noisetilescale,1.0f/noisetilescale,1);	// fixme adjust scale
		glTranslatef(transl.x, transl.y, 0);
		glMatrixMode(GL_MODELVIEW);

		float bt = myfmod(mytime, 10.0) / 10.0f;// seconds, fixme
		if (bt >= 0.5f) bt = 1.0f - bt;
		bt *= 2.0f;
#if 1
		water_bumpmap->set_gl_texture();
#endif
		
		// local parameters:
		// local 0 : upwelling color
		glProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 0,
					     18.0f/255, 73.0f/255, 107.0f/255, 1.0);//fixme test
	} else {
		// standard code path, no fragment programs
	
		//tex0: get alpha from fresnelcolortex, just pass color from primary color
		//tex1: interpolate between previous color and tex1 with previous alpha (fresnel)
		glActiveTexture(GL_TEXTURE0);
		fresnelcolortex->set_gl_texture();
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
		glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
		glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
		glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
		glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_TEXTURE);
		glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
	}

	/* 2005/06/28:
	   fixme: with shaders there are some errors, reflections look strange (dark blue
	   reflection in the near etc. foam experiments showed that the texcoords used
	   to fetch the reflection texels are wrong when looking to a certain direction.
	   foam texture coordinates are also wrong then (use a checker pattern for foam
	   amount to easily see it).
	   maybe the matrices are wrong? yes reflection_projmvmat is not right, some
	   matrix changes happen AFTER it is computed so that it doesn't match the current
	   proj/modl matrices. display() gets the refl_pm parameter, but would not need it. fix that matrix!
	   it appears only in freeview mode, not in bridge/uzo/glasses mode!
	*/

	/* 2005/06/28
	   reflection map can be much smaller than screen res, because ocean water reflects the scene
	   mostly very fuzzy, details can not be seen... fixme
	*/

	glActiveTexture(GL_TEXTURE1);
	glEnable(GL_TEXTURE_2D);
	reflectiontex->set_gl_texture();
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	// rescale coordinates [-1,1] to [0,1]
	glTranslated(0.5,0.5,0);
	glScaled(0.5,0.5,1.0);
	reflection_projmvmat.multiply_gl();
	glMatrixMode(GL_MODELVIEW);

	if (!use_shaders) {
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_INTERPOLATE);
		glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
		glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
		glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_PREVIOUS);
		glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
		glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE2_RGB, GL_PREVIOUS);
		glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND2_RGB, GL_SRC_ALPHA);
		glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
		glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_TEXTURE);
		glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
	}
/*
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PRIMARY_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_CONSTANT);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);
*/

#if 0 //old! (foam)
	glActiveTexture(GL_TEXTURE1);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, foamtex->get_opengl_name());
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
	glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);//GL_INTERPOLATE);	// BLEND?//fixme: foam mixing is disabled for now
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PREVIOUS);
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE);
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE2_RGB, GL_PRIMARY_COLOR);
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND2_RGB, GL_SRC_COLOR);//ALPHA

	glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_TEXTURE);//CONSTANT);//texture1 has alpha 1.0 because it's an RGB texture.
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);//GL_ONE_MINUS_SRC_ALPHA);//this alpha value isn't taken from constant color! where does it come from? fixme

	// fixme: automatic generated texture coordinates for foam may be not realistic enough (foam doesn't move then with displacements)

	GLfloat scalefac1 = 32.0f/WAVE_LENGTH;//fixme: scale to realistic foam size
	GLfloat plane_s1[4] = { scalefac1, 0.0f, 0.0f, 0.0f };
	GLfloat plane_t1[4] = { 0.0f, scalefac1, 0.0f, 0.0f };
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGenfv(GL_S, GL_OBJECT_PLANE, plane_s1);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGenfv(GL_T, GL_OBJECT_PLANE, plane_t1);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
#endif
}



void water::cleanup_textures(void) const
{
	if (use_shaders) {
		glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, 0);
		glDisable(GL_FRAGMENT_PROGRAM_ARB);
		glBindProgramARB(GL_VERTEX_PROGRAM_ARB, 0);
		glDisable(GL_VERTEX_PROGRAM_ARB);
	}
	
	glColor4f(1,1,1,1);

	glActiveTexture(GL_TEXTURE0);
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);

	glActiveTexture(GL_TEXTURE1);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glActiveTexture(GL_TEXTURE0);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glEnable(GL_LIGHTING);

	glPopAttrib();
}



//function is nearly the same as get_height, it just adds extra detail
#define FOO {}
//#define FOO foo();
extern void foo();
vector3f water::compute_coord(const vector3f& xyzpos, const vector2f& transl) const
{
//	return xyzpos;//fixme

	FOO
	// generate values with mipmap function (needs viewer pos.)
	float xfrac = myfrac(float((xyzpos.x + transl.x) / WAVE_LENGTH)) * wave_resolution;
	float yfrac = myfrac(float((xyzpos.y + transl.y) / WAVE_LENGTH)) * wave_resolution;
	unsigned x0 = unsigned(floor(xfrac));
	unsigned y0 = unsigned(floor(yfrac));
	unsigned x1 = (x0 + 1) & (wave_resolution-1);
	unsigned y1 = (y0 + 1) & (wave_resolution-1);
	float xfrac2 = myfrac(xfrac);
	float yfrac2 = myfrac(yfrac);
	unsigned i0 = x0 + (y0 << wave_resolution_shift);
	unsigned i1 = x1 + (y0 << wave_resolution_shift);
	unsigned i2 = x0 + (y1 << wave_resolution_shift);
	unsigned i3 = x1 + (y1 << wave_resolution_shift);
	FOO
	// z distance (for triliniar filtering) is constant along one projected grid line
	// so compute it per line. BUT ONLY FOR OLD RECTANGLULAR WATER PATCH CODE
	// if mipmap level is < 0, we may add some extra detail (noise)
	// this could be fft itself (height scaled) or some perlin noise
	// take x,y fraction as texture coordinates in perlin map.
	// linear filtering should be neccessary -> expensive!

	// fixme: make trilinear
	// fixme: unite displacements and height in one vector3f for simplicity's sake and performance gain
	// bilinear interpolation of displacement and height	
	// fixme: try to tweak height map with displacements, so we need to store only heights...
	// maybe that isn't possible, maybe it looks good enough, but saves computations and memory
	vector3f ca(wavetiledisplacements[i0].x, wavetiledisplacements[i0].y, wavetileheights[i0]);
	vector3f cb(wavetiledisplacements[i1].x, wavetiledisplacements[i1].y, wavetileheights[i1]);
	vector3f cc(wavetiledisplacements[i2].x, wavetiledisplacements[i2].y, wavetileheights[i2]);
	vector3f cd(wavetiledisplacements[i3].x, wavetiledisplacements[i3].y, wavetileheights[i3]);
	FOO
	float fac0 = (1.0f-xfrac2)*(1.0f-yfrac2);
	float fac1 = xfrac2*(1.0f-yfrac2);
	float fac2 = (1.0f-xfrac2)*yfrac2;
	float fac3 = xfrac2*yfrac2;
	vector3f coord = (ca*fac0 + cb*fac1 + cc*fac2 + cd*fac3) + xyzpos;
	FOO
//return coord;

	if (wave_subdetail) {
		// fixme: try to add perlin noise as sub noise, use phase as phase shift, xfrac/yfrac as coordinate
#define SUBDETAIL_PER_TILE 4
		xfrac2 = subdetail_size * myfrac(xfrac * SUBDETAIL_PER_TILE / wave_resolution);
		yfrac2 = subdetail_size * myfrac(yfrac * SUBDETAIL_PER_TILE / wave_resolution);
		x0 = unsigned(floor(xfrac2));
		y0 = unsigned(floor(yfrac2));
		x1 = (x0 + 1) & (subdetail_size-1);
		y1 = (y0 + 1) & (subdetail_size-1);
		xfrac2 = myfrac(xfrac2);
		yfrac2 = myfrac(yfrac2);
		fac0 = (1.0f-xfrac2)*(1.0f-yfrac2);
		fac1 = xfrac2*(1.0f-yfrac2);
		fac2 = (1.0f-xfrac2)*yfrac2;
		fac3 = xfrac2*yfrac2;
		FOO
			float h = (int(waveheight_subdetail[(y0<<subdetail_size_shift) + x0]) - 128) * fac0;
		h += (int(waveheight_subdetail[(y0<<subdetail_size_shift) + x1]) - 128) * fac1;
		h += (int(waveheight_subdetail[(y1<<subdetail_size_shift) + x0]) - 128) * fac2;
		h += (int(waveheight_subdetail[(y1<<subdetail_size_shift) + x1]) - 128) * fac3;
		coord.z += h * (1.0f/512);
		FOO
		// the projgrid code from Claes just maps each vertex to one pixel of a perlin noise map, so he has maximum
		// detail without need to compute filtering. But a that's a very cheap and ugly trick.

	} // subdetail

	return coord;
}



static vector<unsigned> convex_hull(const vector<vector2>& pts)
{
	// find one point that is on hull, that with smallest x
	unsigned lastidx = 0;
	float xmin = pts[0].x;
	for (unsigned i = 1; i < pts.size(); ++i) {
		if (pts[i].x < xmin) {
			xmin = pts[i].x;
			lastidx = i;
		}
	}
	vector<unsigned> result;
	result.push_back(lastidx);

	// now find successor until hull is closed
	while (true) {
		unsigned nextidx = lastidx;
		vector2 base = pts[lastidx];
		// check for all other points if a line to them is on the hull
		for (unsigned i = 0; i < pts.size(); ++i) {
			if (i == lastidx) continue;
			vector2 delta = pts[i] - base;
			// now check if all other points are left of base+t*delta
			bool allleft = true;
			for (unsigned j = 0; j < pts.size(); ++j) {
				if (j == i || j == lastidx) continue;
				vector2 pb = pts[j] - base;
				if (pb.y * delta.x < pb.x * delta.y) {
					// point is right of line, abort
					allleft = false;
					break;
				}
			}
			if (allleft) {
				nextidx = i;
				break;
			}
		}
		sys().myassert(nextidx != lastidx, "no successor found for convex hull point");

		if (nextidx == result.front())
			break;
		result.push_back(nextidx);
		lastidx = nextidx;
	}

	return result;
}


// it seems efficiency rates below 90% are caused by many near triangles that are not shown...
// the trapezoid looks optimal even at 60% efficiency, so it must be something else.
// fixme: try raising the elevator. lower elevator gives less efficiency. 50+ seems ok, with higher values
// the near water looks awful
#undef  DEBUG_TRAPEZOID
vector<vector2> find_smallest_trapezoid(const vector<vector2>& hull)
{
	vector<vector2> trapezoid(4);
	double trapezoid_area = 1e30;
	bool trapset = false;
	// try all hull edges as trapezoid base
	for (unsigned i = 0; i < hull.size(); ++i) {
#ifdef DEBUG_TRAPEZOID
		cout << "trying trapezoid " << i << "\n";
#endif
		vector2 base = hull[i];
		vector2 delta = hull[(i+1) % hull.size()] - base;
		double baselength = delta.length();
#ifdef DEBUG_TRAPEZOID
		cout << "base " << base << " delta " << delta << " baselength " << baselength << "\n";
#endif
		delta = delta * (1.0/baselength);
		vector2 deltaorth = delta.orthogonal();
		// now base + t*delta is base line. compute height of trapez.
		double height = 0;
		unsigned idxtop = 0;
		for (unsigned j = 2; j < hull.size(); ++j) {
			vector2 pb = hull[(i+j) % hull.size()] - base;
			double h = pb * deltaorth;
			sys().myassert(h >= 0, "paranoia chull");
			if (h > height) {
				height = h;
				idxtop = j;
			}
		}
		// compute how much points are on upper line, there must be at least two,
		// or the trapez degenerates to a triangle!
		unsigned onupperline = 0;
		for (unsigned j = 2; j < hull.size(); ++j) {
			vector2 pb = hull[(i+j) % hull.size()] - base;
			double h = pb * deltaorth;
			sys().myassert(h >= 0, "paranoia chull");
#ifdef DEBUG_TRAPEZOID
			cout << "onupperline eps check " << height - h << "\n";
#endif
			if (height - h < h * 0.05 /* EPS */) {
				++onupperline;
			}
		}
#ifdef DEBUG_TRAPEZOID
		cout << "onupperline: [[ " << onupperline << " ]]\n";
#endif
		if (onupperline < 2)
			continue;

		vector2 deltainvx(delta.x, -delta.y);
		vector2 deltainvy(delta.y,  delta.x);
//		cout << "height of trapez is " << height << "\n";
		// now find left+right edge so that area is minimal
		// try out all hull lines as edges, one of them is the solution (proofed).
		// compute line through hull line and the crossing with the baseline and topline,
		// that gives coordinates a,b (on base/top line). the solution line is that with
		// a+b mimimal. left solution can be lines with delta.y < 0, right: delta.y > 0
		double maxa = -1e30, maxb = -1e30, mina = 1e30, minb = 1e30;
		bool maxset = false, minset = false;
		for (unsigned j = 1; j < hull.size(); ++j) {
			vector2 p0 = hull[(i+j) % hull.size()] - base;
			vector2 p1 = hull[(i+j+1) % hull.size()] - base;
			vector2 df = p1 - p0;
			p0 = p0.matrixmul(deltainvx, deltainvy);
			df = df.matrixmul(deltainvx, deltainvy);
			if (fabs(df.y) < 0.001)	// avoid lines nearly parallel to baseline
				continue;
#ifdef DEBUG_TRAPEZOID
			cout << "p0 : " << p0 << " df: " << df << "\n";
#endif
			//manchmal ist das negativ, also falsch...
			double a = p0.x - p0.y * df.x / df.y;
			double b = p0.x + (height - p0.y) * df.x / df.y;
#ifdef DEBUG_TRAPEZOID
			cout << "tried line " << j << " a " << a << " b " << b << "\n";
#endif
			if (df.y > 0) {
				// line is on right border of trapezoid
				if (a+b < mina+minb) {
					mina = a;
					minb = b;
					minset = true;
				}
			} else {
				// line is on left border of trapezoid
				if (a+b > maxa+maxb) {
					maxa = a;
					maxb = b;
					maxset = true;
				}
			}
		}
		//fixme: this can happen sometimes!
		if (!(maxset && minset)) {
			sys().add_console("WARNING: no min/max found in find_trapez");
			continue;
		}
		//sys().myassert(maxset && minset, "ERROR!!!! no min/max found in find trapez");
#ifdef DEBUG_TRAPEZOID
		cout << "mina " << mina << " minb " << minb << " maxa " << maxa << " maxb " << maxb << "\n";
#endif
		double a2 = mina - maxa, b2 = minb - maxb;
		double area = (a2+b2)*height/2;
#ifdef DEBUG_TRAPEZOID
		cout << "trapez area: " << area << "\n";
#endif
		// more an hack, only take trapezoids in one direction, do not cound
		// the same trapzeoid upsidedown to avoid tremor effect. fixme, real reason unknown
		// when camera is moved around, tremor effect disappears after some time.
		// maybe still a rounding error?
		if (area < trapezoid_area + 0.5 /* EPS */) {
			unsigned idx[4] = { 0, 1, 2, 3 };
			if (delta.x < 0) {
				// trapzeoid is upsidedown, flip it
				idx[0] = 2;
				idx[1] = 3;
				idx[2] = 0;
				idx[3] = 1;
			}
			trapezoid_area = area;
			trapezoid[idx[0]] = base + delta * maxa;
			trapezoid[idx[1]] = base + delta * mina;
			trapezoid[idx[2]] = base + delta * minb + deltaorth * height;
			trapezoid[idx[3]] = base + delta * maxb + deltaorth * height;
			trapset = true;
		}
	}
#ifdef DEBUG_TRAPEZOID
	cout << "MINIMAL trapez, area " << trapezoid_area << "\n";
	cout << trapezoid[0] << " | " << trapezoid[1] << " | " << trapezoid[2] << " | " << trapezoid[3] << "\n";
#endif
	if (!trapset) {
		sys().add_console("WARNING: no trapezoid found!");
		trapezoid.clear();
	}
	//sys().myassert(trapset, "no trapez found?!");
	return trapezoid;
}


static unsigned nrfm=0;
void water::compute_amount_of_foam_texture(const vector3& viewpos,
					   const matrix4& reflection_projmvmat,
					   const vector<ship*>& allships) const
{
	if (!use_shaders)
		return;	// no foam without shaders

//	glPushMatrix();

	unsigned afs = FOAMAMOUNTRES;	// size of amount of foam size, depends on water grid detail, fixme
	// foam is drawn the the same matrices as water, but with a smaller viewport
	// set up viewport
	glViewport(0, 0, afs, afs);
	// clear it , fixme: must be done earlier, there is already drawn something inside the viewport
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);

	// draw trails / wave tops
	//fixme: man sieht nix, auch ohne face cull. irgendwie stimmt die viewmatrix nicht.
	//die entspricht noch nicht dem endergebnis. außerdem sind alle positionne hier
	//absolut, was wohl auch falsch ist (playerpos muß abgezogen werden?) mit water quer checken
	//passiert schon, aber die koordinaten sind immer gleich unabhängig vom drehwinkel.
	//das erklärt viele fehler.
	//viewmatrix falsch oder sowas. sogar in freeview
//	glDisable(GL_CULL_FACE);
	// as first trails of all ships
/*
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
*/
	foamamounttrail->set_gl_texture();
//	glBindTexture(GL_TEXTURE_2D, 0);
//	glDisable(GL_TEXTURE_2D);
//fixme: farbwerte sind zu dunkel, liegt nicht am blending
	glDisable(GL_LIGHTING);
	// fixme: texture mapping seems to be wrong.
	for (vector<ship*>::const_iterator it = allships.begin(); it != allships.end(); ++it) {
		vector2 spos = (*it)->get_pos().xy() - viewpos.xy();
		vector2 sdir = (*it)->get_heading().direction();
		vector2 pdir = sdir.orthogonal();
		const list<vector2>& prevpos = (*it)->get_previous_positions();
		float sw = (*it)->get_width();
		float sl = (*it)->get_length();
		vector2 p0 = spos + sdir * (sl/2 /*fixme should be 2 but... the whole thing seems to be scaled by factor 2!*/);
		//especially in freeview mode, but the bow caused foam seems also to be too long
		glColor4f(1, 1, 1, 1);
		glBegin(GL_QUAD_STRIP);
		vector2 pl = p0 + pdir * (-1.0f);
		vector2 pr = p0 + pdir * ( 1.0f);
		glTexCoord2f(0, 0);
		glVertex3d(pl.x, pl.y, -viewpos.z);
		glTexCoord2f(1, 0);
		glVertex3d(pr.x, pr.y, -viewpos.z);
		pl = spos + pdir * (-sw/2);
		pr = spos + pdir * ( sw/2);
		glTexCoord2f(0, 1);
		glVertex3d(pl.x, pl.y, -viewpos.z);
		glTexCoord2f(1, 1);
		glVertex3d(pr.x, pr.y, -viewpos.z);
		// use 1.0/x, but test hack, take 3 for using 1/3 of prevpos.
		float fadd = (prevpos.size() < 2) ? 0.0f : -3.0f/(prevpos.size() - 1);
		float fcnt = 1.0f;
		//fixme: the amount of foam decreases too slowly, use less prevpos,
		//about 1/2 or even 1/4 should be enough
		float v = 2.0f;
		for (list<vector2>::const_iterator lt = prevpos.begin(); lt != prevpos.end(); ++lt) {
			// fixme. we need new ortho vector here, depends on prevpos!
			p0 = *lt - viewpos.xy();
			pl = p0 + pdir * (-sw/2);
			pr = p0 + pdir * ( sw/2);
			glColor3f(fcnt, fcnt, fcnt);
			glTexCoord2f(0, v);
			glVertex3d(pl.x, pl.y, -viewpos.z);
			glTexCoord2f(1, v);
			glVertex3d(pr.x, pr.y, -viewpos.z);
			fcnt += fadd;
			if (fcnt <= 0) break;
			v += 1.0f;
		}
		glEnd();
	}
	glEnable(GL_LIGHTING);
//	glEnable(GL_TEXTURE_2D);
//	glEnable(GL_CULL_FACE);
	// ship->get_heading(),
	// get_pos()
	// list<vector2> get_previous_positions
	// fixme: needs infos about ships/subs/torps/shells? only accessible in user_interface/freeview_display
	// ship holds previous positions (list of vector2 that are needed for display)
	//as first only subs, draw quad strip along the trail
	//first strip width depends on bow width, then model width, constant with with decaying strength
	//maybe with a texture of grey/black lines or mix to simulate screw foam/hull foam/randomness etc.
/*
	glRasterPos2i(50, 50);
	vector<Uint8> tmp(8*8);
	for (int i = 0; i < 8*8; ++i) if (((i/8)+(i%8))&1) tmp[i] = 4*i;
	glDrawPixels(8, 8, GL_LUMINANCE, GL_UNSIGNED_BYTE, &tmp[0]);
	glRasterPos2i(0, 0);
*/
	
#if 0
	vector<Uint8> data(afs*afs*3);
	glReadPixels(0, 0, afs, afs, GL_RGB, GL_UNSIGNED_BYTE, &data[0]);
        ostringstream osgname;
        osgname << "foamamount" << nrfm++ << ".ppm";
        ofstream osg(osgname.str().c_str());
        osg << "P6\n"<<afs<<" "<<afs<<"\n255\n";
        osg.write((const char*)(&data[0]), afs*afs*3);
#endif

	// copy viewport data to foam-amount texture
	foamamounttex->set_gl_texture();
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, afs, afs, 0);

	// clean up
	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, sys().get_res_x(), sys().get_res_y());

//	glPopMatrix();
}



void water::display(const vector3& viewpos, angle dir, double max_view_dist, const matrix4& reflection_projmvmat) const
{
	//fixme: we don't need the reflection_projmvmat parameter, we can compute the proj*modl matrix by
	//asking opengl...

	const float VIRTUAL_PLANE_HEIGHT = 25.0f;	// fixme experiment, amount of reflection distorsion, 30.0f seems ok, maybe a bit too much

	// maximum height of waves (half amplitude)
	const double WAVE_HEIGHT = std::max(maxh, fabs(minh));

//	cout << "Wave height is: " << WAVE_HEIGHT << "\n";

	// fixme: theory: keep the projector above some minimum z value, that is higher than WAVE_HEIGHT
	// that will help keeping some of the detail in the distance and avoids drawing to much near waves.
	// together with a trapezoidical form of the projection, efficiency should be good...
	// this value plus WAVE_HEIGHT is the minimum height.
	// hmm, with elev 30, efficiency goes down to 70%. Trapezoid is optimal,
	// but maybe too many triangles are lost in the near
	const double ELEVATION = 40.0; // fixme experiment, try 1...30

	// fixme: displacements must be used to enlarge projector area or else holes will be visible at the border
	// of the screen (left and right), especially on glasses mode

	// get projection and modelview matrix
	matrix4 proj = matrix4::get_gl(GL_PROJECTION_MATRIX);
	matrix4 modl = matrix4::get_gl(GL_MODELVIEW_MATRIX);
	matrix4 inv_modl = modl.inverse();

	// modify modelview matrix so that viewer is at (0,0,h) with h in |R with h = cameraheight
	vector3 correction = inv_modl.column3(3);

	// set gl matrix so that viewer is at 0, 0, 0
	glPushMatrix();
	glTranslated(correction.x, correction.y, correction.z);

	correction.z -= viewpos.z;
	modl = modl * matrix4::trans(correction);
	inv_modl = matrix4::trans(-correction) * inv_modl;

	matrix4 world2camera = proj * modl;
	matrix4 camera2world = world2camera.inverse();

	// transform frustum corners of rendering camera to world space
	vector3 frustum[8];
	for (unsigned i = 0; i < 8; ++i) {
		vector3 fc((i & 1) ? 1 : -1, (i & 2) ? 1 : -1, (i & 4) ? 1 : -1);
		frustum[i] = camera2world * fc;
	}
	
	// check intersections of frustum with water volume
	vector<vector3> proj_points;
	unsigned cube[24] = { 0,1, 0,2, 2,3, 1,3, 0,4, 2,6, 3,7, 1,5, 4,6, 4,5, 5,7, 6,7 };
	for (unsigned i = 0; i < 12; ++i) {
		unsigned src = cube[i*2], dst = cube[i*2+1];
		if ((frustum[src].z - WAVE_HEIGHT) / (frustum[dst].z - WAVE_HEIGHT) < 0.0) {
			double t = (WAVE_HEIGHT - frustum[src].z) / (frustum[dst].z - frustum[src].z);
			proj_points.push_back(frustum[src] * (1.0-t) + frustum[dst] * t);
		}
		if ((frustum[src].z + WAVE_HEIGHT) / (frustum[dst].z + WAVE_HEIGHT) < 0.0) {
			double t = (-WAVE_HEIGHT - frustum[src].z) / (frustum[dst].z - frustum[src].z);
			proj_points.push_back(frustum[src] * (1.0-t) + frustum[dst] * t);
		}
	}
	// check if any frustum points are inside the water volume
	for (unsigned i = 0; i < 8; ++i) {
		if (frustum[i].z <= WAVE_HEIGHT && frustum[i].z >= -WAVE_HEIGHT)
			proj_points.push_back(frustum[i]);
	}
	
	// compute projector matrix
	// the last column of the inverse modelview matrix is the camera position
	// the upper left 3x3 matrix is the camera rotation, so the columns hold the view vectors
	vector3 camerapos = vector3(0, 0, viewpos.z);
	vector3 cameraforward = -inv_modl.column3(2); // camera is facing along negative z-axis
//cout << "camerapos " << camerapos << "\n";
//cout << "camera forward " << cameraforward << "\n";
	vector3 projectorpos = camerapos, projectorforward = cameraforward;

	vector3 aimpoint, aimpoint2;
	
	// make sure projector is high enough above the plane
	// mirror it if camera is below water surface
	if (projectorpos.z < 0)
		projectorpos.z = -projectorpos.z;
	if (projectorpos.z < WAVE_HEIGHT + ELEVATION)
		projectorpos.z = WAVE_HEIGHT + ELEVATION;

	// compute intersection of forward vector with plane (fixme forward.z == 0 -> NaN)
	if ((cameraforward.z < 0.0 && camerapos.z >= 0.0) || (cameraforward.z >= 0.0 && camerapos.z < 0.0)) {
		double t = -camerapos.z / cameraforward.z;
		aimpoint = camerapos + t * cameraforward;
	} else {
		vector3 flipped = cameraforward;
		flipped.z = -flipped.z;
		double t = -camerapos.z / flipped.z;
		aimpoint = camerapos + t * flipped;
	}
	
	aimpoint2 = camerapos + 10.0 * cameraforward;
	aimpoint2.z = 0.0;
	
	// fade between points depending on angle
	// when camera points to the horizon, aimpoint is very far away, we have to shift
	// to aimpoint2 which is in the near. The resulting detail depends also on the projector
	// elevation. The higher the elevation the lesser the detail is in the near distance.
	double af = fabs(cameraforward.z);
	projectorforward = (aimpoint * af + aimpoint2 * (1.0-af)) - projectorpos;

//cout << "projectorpos " << projectorpos << "\n";
//cout << "projector forward " << projectorforward << "\n";
	
	// compute rest of the projector matrix from pos and forward vector
	vector3 pjz = -projectorforward.normal();
	vector3 pjx = vector3(0, 0, 1).cross(pjz); // fixme: what if pjz==up vector (or very near it) then errors occour, they're visible!, this workaround seems ok. What did Claes in that case?
	if (pjx.length() < 0.001)
		pjx = vector3(1, 0 , 0);
	else
		pjx = pjx.normal();
	vector3 pjy = pjz.cross(pjx);
//cout << "pjx " << pjx << " pjy " << pjy << " pjz " << pjz << "\n";
//cout << "lengths " << pjx.length() << "," << pjy.length() << "," << pjz.length() << "\n";

	matrix4 inv_modl_projector(
		pjx.x, pjy.x, pjz.x, projectorpos.x,
		pjx.y, pjy.y, pjz.y, projectorpos.y,
		pjx.z, pjy.z, pjz.z, projectorpos.z,
		0, 0, 0, 1);
	matrix4 world2projector = proj * inv_modl_projector.inverse();
	matrix4 projector2world = world2projector.inverse();
	
	// project frustum intersection points to z=0 plane
	// transform their coords to projector space
	for (vector<vector3>::iterator it = proj_points.begin(); it != proj_points.end(); ++it) {
		it->z = 0;
		*it = world2projector * *it;
	}
	
	// To increase efficiency we compute a trapezoid around the projected points.
	// Mostly they form a trapezoidical shape. This increases the number of vertices
	// that are drawn inside the frustum dramatically.
	vector<vector2> trapezoid;
	if (proj_points.size() == 0) {
		// nothing to draw.
		glPopMatrix();
		return;
	} else {
		// scale coordinates to compensate for wave displacement here? fixme

		// find convex hull of points.
		//   collect unique points
		vector<vector2> proj_points_2d;
		for (unsigned i = 0; i < proj_points.size(); ++i) {
			bool insert = true;
			for (unsigned j = 0; j < proj_points_2d.size(); ++j) {
				if (proj_points[i].xy().square_distance(proj_points_2d[j]) < 0.0001) {
					insert = false;
					break;
				}
			}
			if (insert) proj_points_2d.push_back(proj_points[i].xy());
		}

		//   compute hull
		vector<unsigned> idx = convex_hull(proj_points_2d);
		vector<vector2> chull(idx.size());
#ifdef DEBUG_TRAPEZOID
		cout << "convex hull:\n";
#endif
		for (unsigned i = 0; i < idx.size(); ++i) {
			chull[i] = proj_points_2d[idx[i]];
#ifdef DEBUG_TRAPEZOID
			cout << i << ": " << proj_points_2d[idx[i]] << "\n";
#endif
		}
#ifdef DEBUG_TRAPEZOID
		for (unsigned i = 0; i < proj_points.size(); ++i)
			cout << "pp: " << proj_points[i] << "\n";
#endif
		// find smallest trapezoid surrounding the hull (try with all hull lines as base)
		//   trapezoid area = (a+b)*h/2
		trapezoid = find_smallest_trapezoid(chull);
		if (trapezoid.empty()) {
			// sometimes trapez construction fails.
			glPopMatrix();
			return;
		}
	} // else return;

#if 0
	// show projector frustum as test
	glColor3f(1,0,0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBegin(GL_LINES);
	vector3 prfrustum[8];
	for (unsigned i = 0; i < 8; ++i) {
		vector3 fc((i & 1) ? 1 : -1, (i & 2) ? 1 : -1, (i & 4) ? 1 : -1);
		prfrustum[i] = projector2world * fc;	// without range matrix!
	}
	for (unsigned i = 0; i < 12; ++i) {
		glVertex3dv(&prfrustum[cube[2*i]].x);
		glVertex3dv(&prfrustum[cube[2*i+1]].x);
	}
	glEnd();
	glColor3f(1,1,1);
	// show camera frustum as test
	glColor3f(1,1,0);
	glBegin(GL_LINES);
	for (unsigned i = 0; i < 12; ++i) {
		glVertex3dv(&frustum[cube[2*i]].x);
		glVertex3dv(&frustum[cube[2*i+1]].x);
	}
	glEnd();
	glColor3f(1,1,1);
#endif

	// compute coordinates

	// this loop takes ~ 6ms of full 19ms per frame.
	// we could let the GPU compute the loop:
	// store fft heights as 8bit luminance texture
	// project it to a gridsize * gridsize framebuffer
	// using a single quad textured with this texture and trilinear filtering
	// read in the pixel data as heights.
	// with 50% time save (3ms) fps would go from 51 to 60.
	// earth curvature could be simulated by darkening the texture in distance (darker = lower!)

#ifdef COMPUTE_EFFICIENCY
	int vertices = 0, vertices_inside = 0;
#endif	
	vector2f transl(myfmod(viewpos.x, WAVE_LENGTH), myfmod(viewpos.y, WAVE_LENGTH));
	for (unsigned yy = 0, ptr = 0; yy <= yres; ++yy) {
		double y = double(yy)/yres;
		// vertices for start and end of two lines are computed and projected
		vector2 trapleft  = trapezoid[3] * y + trapezoid[0] * (1.0 - y);
		vector2 trapright = trapezoid[2] * y + trapezoid[1] * (1.0 - y);
		vector3 v1 = projector2world *  trapleft.xyz(-1);
		vector3 v2 = projector2world *  trapleft.xyz(+1);
		vector3 v3 = projector2world * trapright.xyz(-1);
		vector3 v4 = projector2world * trapright.xyz(+1);

		// compute intersection with z = 0 plane here
		// we could compute intersection with earth's sphere here for a curved display
		// of water to the horizon, fixme
		double t1 = -v1.z/(v2.z-v1.z), t2 = -v3.z/(v4.z-v3.z);
		vector2 va = v1.xy() * (1-t1) + v2.xy() * t1;
		vector2 vb = v3.xy() * (1-t2) + v4.xy() * t2;
		//fixme: we could move/change vy/vy/vxadd/vyadd here to compensate for displacement
		//fixme: water display tremors, this depends on the direction of the trapezoid
		//when its painted from top to bottom or bottom to top (two trapezoids with same
		//area and shape, but with the upper line of the first being the base line of the
		//second and vice versa). Rounding problem??? error seems to large for that.
		vector2 diff = vb - va;
		double difflen = diff.length();
		diff = diff * (1.0/difflen);
		double displ = 5.0;	// meters, max. displacement of waves, fixme compute!
		vb = va + diff * (difflen + displ);
		va = va - diff * displ;

		double vx = va.x;
		double vy = va.y;
		double vxadd = (vb.x - va.x)/xres;
		double vyadd = (vb.y - va.y)/xres;
		for (unsigned xx = 0; xx <= xres; ++xx, ++ptr) {
			vector3f v(vx, vy, -viewpos.z);
			coords[ptr] = compute_coord(v, transl);
			vx += vxadd;
			vy += vyadd;
#ifdef COMPUTE_EFFICIENCY
			vector3 tmp = world2camera * vector3(coords[ptr].x, coords[ptr].y, coords[ptr].z);
			// fixme: for a better efficiency analysis we have to store WHERE the vertices
			// are outside (near/far/left/right)
			if (fabs(tmp.x) <= 1.0 && fabs(tmp.y) <= 1.0 && fabs(tmp.z) <= 1.0)
				++vertices_inside;
			++vertices;
#endif
		}
	}

	// this takes ~13ms per frame with 128x256. a bit costly
	//fixme: maybe choose only trapzeoid with baseline parallel zu x-axis? but this doesn't save any computations
	//wave sub detail should bring more display quality than higher resolution....
//	cout << "coord computation took " << tm2-tm1 << " ms.\n";

#ifdef COMPUTE_EFFICIENCY
	cout << "drawn " << vertices << " vertices, " << vertices_inside << " were inside frustum ("
		<< vertices_inside*100.0f/vertices << " % )\n";
#endif

	// compute dynamic normals
	// clear values from last frame
	fill(normals.begin(), normals.end(), vector3f());

	// compute normals for all faces, add them to vertex normals
	// fixme: angles at vertices are ignored yet
	for (unsigned y = 0; y < yres; ++y) {
		unsigned y2 = y+1;
		for (unsigned x = 0; x < xres; ++x) {
			unsigned x2 = x+1;
			unsigned i0 = x +y *(xres+1);
			unsigned i1 = x2+y *(xres+1);
			unsigned i2 = x2+y2*(xres+1);
			unsigned i3 = x +y2*(xres+1);
			// scheme: 3 2
			//         0 1
			// triangulation: 0,1,2 and 0,2,3 (fixme: check if that matches OpenGL order)
			const vector3f& p0 = coords[i0];
			const vector3f& p1 = coords[i1];
			const vector3f& p2 = coords[i2];
			const vector3f& p3 = coords[i3];
			//fixme: it seems that the normals are not smooth over the faces...
			//the surface seems to be facetted, not smooth.
			// better compute normals from four surrounding faces.
			//it would not suffice to compute the normals from the height differences,
			//because the x/y coords do not vary uniformly.
			vector3f n0 = (p1 - p0).cross(p2 - p0);
#if 0
			normals[i0] += n0;
#else
			// fixme: the n1 vector seems to be the reason for the facetted surface normals
//			vector3f n1 = (p2 - p0).cross(p3 - p0);
			normals[i0] += n0;
			normals[i1] += n0;
			normals[i2] += n0;
/*
			normals[i0] += n1;
			normals[i2] += n1;
			normals[i3] += n1;
*/
#endif
		}
	}

	// make normals normal ;-)
	// with shaders, we don't need to normalize the normals, because the shader does it
	if (!use_shaders)
		for (unsigned i = 0; i < (xres+1)*(yres+1); ++i)
			normals[i].normalize();

	// compute remaining data (Fresnel etc.)
	//fixme: the whole loop could be done on the GPU with vertex shaders.
	//coords and normals change per frame, so no display lists can be used.
	//simple vertex arrays with locking should do the trick, maybe use
	//(locked) quadstrips, if they're faster than compiled vertex arrays, test it!

	vector<vector3f> uv2;
	if (use_shaders) {
		// foam:
		//uv2.resize(uv0.size());
	}

	// no tex coords must be given to vertex shaders, fresnel texc's are computed from the
	// position, same for reflection texc. only foam is more difficult
	// foam is mapped like a great plane, depends on player's viewing angle and position...

	if (use_shaders) {
		//fixme: uv0/uv1 is not needed here and thus needs not be resized/created.
		// enable coordinates and normals, but disable texture coordinates.
		glDisableClientState(GL_COLOR_ARRAY);
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, sizeof(vector3f), &coords[0].x);
		glClientActiveTexture(GL_TEXTURE0);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glClientActiveTexture(GL_TEXTURE1);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);
		glNormalPointer(GL_FLOAT, sizeof(vector3f), &normals[0].x);
		/* foam:
		glClientActiveTexture(GL_TEXTURE2);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(3, GL_FLOAT, sizeof(vector3f), &uv2[0].x);
		*/
	} else {
		// normalize normals.
		for (unsigned i = 0; i < (xres+1)*(yres+1); ++i)
			normals[i].normalize();
		for (unsigned yy = 0, ptr = 0; yy <= yres; ++yy) {
			for (unsigned xx = 0; xx <= xres; ++xx, ++ptr) {
				// the coordinate is the same as the relative coordinate, because the viewer is at 0,0,0
				const vector3f& coord = coords[ptr];
				const vector3f& N = normals[ptr];
				float rel_coord_length = coord.length();
				vector3f E = -coord * (1.0f/rel_coord_length); // viewer is in (0,0,0)
				float F = E*N;		// compute Fresnel term F(x) = ~ 1/(x+1)^8
				// make water less reflective in the distance to simulate
				// the fact that we look not on a flat plane in the distance
				// but mostly wave sides,
				// but the water display is similar to such a plane
				if (rel_coord_length > 500) {
					float tmp = (30000 - rel_coord_length)/29500;
					tmp = tmp * tmp;
					// 0.09051=x gives fresnel term 0.5
					F = F * tmp + 0.09051f * (1 - tmp);
				}
				//fixme: far water reflects atmosphere seen from a farer distance
				//the reflected ray goes right up into the atmosphere (earth
				//surface is curved!) so reflected color would be rather blue
				//not the horizon like greyish fog...
				// value clamping is done by texture unit.
				// water color depends on height of wave and slope
				// slope (N.z) it mostly > 0.8
				float colorfac = (coord.z + viewpos.z + 3) / 9 + (N.z - 0.8f);
				uv0[ptr] = vector2f(F, colorfac);	// set fresnel and water color
				// reflection texture coordinates (should be tweaked per pixel with fp, fixme)
				// they are broken with fp, reason unknown
				vector3f texc = coord + N * (VIRTUAL_PLANE_HEIGHT * N.z);
				texc.z -= VIRTUAL_PLANE_HEIGHT;
				uv1[ptr] = texc;
			}
		}
		// draw elements (fixed index list) with vertex arrays using coords,uv0,normals,uv1
		glDisableClientState(GL_COLOR_ARRAY);
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, sizeof(vector3f), &coords[0].x);
		glClientActiveTexture(GL_TEXTURE0);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, sizeof(vector2f), &uv0[0].x);
		glClientActiveTexture(GL_TEXTURE1);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(3, GL_FLOAT, sizeof(vector3f), &uv1[0].x);
		glDisableClientState(GL_NORMAL_ARRAY);
	}

	// set up textures
	setup_textures(reflection_projmvmat, transl); //fixme test

	glColor4f(1,1,1,1);

//	unsigned t0 = sys().millisec();

	// lock Arrays for extra performance.
	if (compiled_vertex_arrays_supported)
		glLockArraysEXT(0, (xres+1)*(yres+1));
#ifdef DRAW_WATER_AS_GRID
	glDrawElements(GL_LINES, gridindices2.size(), GL_UNSIGNED_INT, &(gridindices2[0]));
#else
	glDrawElements(GL_QUADS, gridindices.size(), GL_UNSIGNED_INT, &(gridindices[0]));
#endif

	if (compiled_vertex_arrays_supported)
		glUnlockArraysEXT();

//	unsigned t2 = sys().millisec();
//	drawing takes ~28ms with linux/athlon2200+/gf4mx. That would be ~32mb/sec with AGP4x!?
//	why is it SO SLOW?!
//	surprising result: fillrate limit! with 512x384 we have 50fps, with 800x600 35fps,
//	with 1024x768 25fps. Water has approximatly 1Mpixels/frame. Bad for a gf4mx!!
//	ATI Radeon 9600 Pro: 35fps with catalyst 3.7.6, 14.4 (!!!) with catalyst 3.2.8. WHY?!
//	cout << "t0 " << t0 << " t1 " << t1 << " diff " << t1-t0 << "\n";

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	if (use_shaders) {
		glClientActiveTexture(GL_TEXTURE2);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}
	glClientActiveTexture(GL_TEXTURE1);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glClientActiveTexture(GL_TEXTURE0);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glPopMatrix();

	// clean up textures
	cleanup_textures();
}



	// How we render the water:
	// Per pixel effects are not possible on geforce2. We could do dot3 bump mapping,
	// but this gives diffuse lighting effects, and water is mostly reflective, refractive
	// or specular shaded. Dot3 bump mapping could be used to disturb the color values
	// a bit, if it would look good is unknown and has to be tested.
	// We can do specular lighting per vertex only (per pixel would need pixel shaders or
	// register combiners, which is not compatible to ATI cards). This could be done
	// even by OpenGL itself. But the triangles are too large for realistic specular
	// lighting, it just looks very ugly. So we don't use specular lighting.
	// for each vertex do:
	// get height/coordinate from precomputed heights/translation pos/displacements
	// compute normalized vector to watcher = E
	// get normal vector from precomputed data = N
	// compute E*N, compute Fresnel(E*N). Fresnel(x) gives a value between 0 and 1
	// determining how much reflection and how much refraction contributes to the water
	// color. Fresnel(x) ~ 1/(x+1)^8 =: F. There is no way to compute this per pixel on a gf2
	// not even an approximation (linear approximation is way too bad).
	// And we can't use color values as texture indices on a gf2, so per pixel Fresnel
	// is impossible, although it would increase the realism of the water.
	// Based on F the water color is: refractive color * (1-F) + reflective color * F
	// refractive color is const (rgb 18, 73, 107), reflective color is const (sky color)
	// or better retrieved from a reflection map.
	// So we have: primary color gives fresnel term (in color or alpha, 1 channel)
	// Texture0 is reflection map.
	// Texture1 is bump map (for fresnel fakes) or foam.
	// Primary color (alpha or color) can give amount of foam (1 channel)
	// Constant environment color is color of refraction.
	// A word about automatic Fresnel computation: spheremaps or cubemaps won't work
	// since reflections are view dependent (most important, they are view *angle*
	// dependent), recomputation of sphere maps every frame won't work in a way that we
	// can simulate a view independent value. Next possible way: lighting! set up the light
	// source at the viewer's pos., give normals for faces -> they're lighted in grey
	// like the E*N term (diffuse lighting). But we can't get F(E*N) per vertex automatically
	// so we're stuck.
	
	// high frequency waves could be done with perlin noise (i.e. bump maps)
	
	// Filling the gaps between various detail tiles:
	// We could make real meshes by tweaking the indices so that we have more faces in
	// the last line of the lower detailed tile matching the first line of the following
	// higher detailed tile. So we need precomputed indices for all cases. Expensive.
	// Alternative solution: tweak the vertices' positions, not the indices. This leads to
	// gaps in the mesh, but the faces' edges match, so no gaps can be seen. Just tweak
	// the values of every second vertex on the last line of the higher detailed tile:
	// for three vertices a,b,c in a line on the higher detailed tile, a and c will be
	// also in the lower detailed tile. Set position of b as (a+c)/2, and do the same with
	// b's colors and texture coordinates. Simple.
	// Precondition: Adjacent tiles must not differ by more than one LOD. But this must not be
	// either for the index tweaking alternative.
	// Alternative II: draw n*n verts, fill inner parts of tiles only (with varying detail)
	// fill seams with faces adapting the neighbouring detail levels.
	// Alternative III: use projective grid

	// Water drawing is slow. It seems that the per vertex computations are not the problem
	// nor is it the gap filling. Most probable reason is the amount of data that
	// has to be transferred each frame.
	// We have 4 colors, 3 coords, 2 texcoords per vertex (=4+12+8=24bytes)
	// with 65*65 verts per tile, 21*21 tiles, mean resolution, say, 20 we have
	// 21*21*21*21*24bytes/frame = ~4.7mb/frame, with 15frames ~ 70mb/second.
	// VBOs won't help much, they're also seem to be buggy on a gf4mx.
	// Reason: a gf2mx can handle at most 16384 triangles.
	// A gf4mx may handle more, general limit is 65536.

	// timing: calculation of vertex data ~ 1/3-1/4 compared to DrawElements()
	// for a large tile DrawElements needs up to 6.2ms per call!
	// (two of all tiles need ~6ms, some 4, some 2, most of them need less time)
	// speedups: 1) draw only visible tiles (~1/4 of them can be seen)
	// 2) use vertex programs.
	// data per tile (max): 65*65*((3+3)*4+4) (3xcoord,3xtexc,1xcolor4)
	// = 4225 * 28 = 118300 = ~115.5kb, in 6222us -> ~18.13mb/sec.
	// even if you assume some overhead, 18mb/sec is VERY slow.
	// and this is why water display is so horribly slow.
	// if we wouldn't need to modify 3d data (make adjacent tiles with index manipulation only)
	// we could store coords and normals in gpu memory (~24mb for all 256frames or store it
	// once per frame) and upload only the color values (1/7 of space).




void water::update_foam(double deltat)
{
/*
	float foamvanish = deltat * FOAM_VANISH_FACTOR;
	for (unsigned k = 0; k < FACES_PER_AXIS*FACES_PER_AXIS; ++k) {
		float& foam = wavefoam[k];
		foam -= foamvanish;
		if (foam < 0.0f) foam = 0.0f;
		wavefoamtexdata[k] = Uint8(255*foam);
	}
	glBindTexture(GL_TEXTURE_2D, wavefoamtex);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, FACES_PER_AXIS, FACES_PER_AXIS, GL_COLOR_INDEX, GL_UNSIGNED_BYTE, &wavefoamtexdata[0]);
*/	
}

void water::spawn_foam(const vector2& pos)
{
/*
	// compute texel position from pos here
	unsigned texel = FACES_PER_AXIS*FACES_PER_AXIS/2+FACES_PER_AXIS/2;
	float& f = wavefoam[texel];
	f += 0.1;
	if (f > 1.0f) f = 1.0f;
	wavefoamtexdata[texel] = Uint8(255*f);
*/	
}



float water::get_height(const vector2& pos) const
{
	float ffac = wave_resolution/WAVE_LENGTH;
	float x = float(myfmod(pos.x, WAVE_LENGTH)) * ffac;
	float y = float(myfmod(pos.y, WAVE_LENGTH)) * ffac;
	int ix = int(floor(x));
	int iy = int(floor(y));
	int ix2 = (ix+1) & (wave_resolution-1);
	int iy2 = (iy+1) & (wave_resolution-1);
	float fracx = x - ix;
	float fracy = y - iy;
	float a = wavetileheights[ix +(iy <<wave_resolution_shift)];
	float b = wavetileheights[ix2+(iy <<wave_resolution_shift)];
	float c = wavetileheights[ix +(iy2<<wave_resolution_shift)];
	float d = wavetileheights[ix2+(iy2<<wave_resolution_shift)];
	float e = a * (1.0f-fracx) + b * fracx;
	float f = c * (1.0f-fracx) + d * fracx;
	return (1.0f-fracy) * e + fracy * f;
}



vector3f water::get_wave_normal_at(unsigned x, unsigned y) const
{
	unsigned x1 = (x + wave_resolution - 1) & (wave_resolution-1);
	unsigned x2 = (x                   + 1) & (wave_resolution-1);
	unsigned y1 = (y + wave_resolution - 1) & (wave_resolution-1);
	unsigned y2 = (y                   + 1) & (wave_resolution-1);
	float hdx = wavetileheights[x2+ (y <<wave_resolution_shift)]
		-   wavetileheights[x1+ (y <<wave_resolution_shift)];
	float hdy = wavetileheights[x + (y2<<wave_resolution_shift)]
		-   wavetileheights[x + (y1<<wave_resolution_shift)];
	return vector3f(-hdx, -hdy, 1).normal();
}


//fixme: the correctness of the result of this function and the one above is not fully tested.
//with a realistic buoyancy model we don't need that function any longer!
vector3f water::get_normal(const vector2& pos, double rollfac) const
{
	float ffac = wave_resolution/WAVE_LENGTH;
	float x = float(myfmod(pos.x, WAVE_LENGTH)) * ffac;
	float y = float(myfmod(pos.y, WAVE_LENGTH)) * ffac;
	int ix = int(floor(x));
	int iy = int(floor(y));
	int ix2 = (ix+1) & (wave_resolution-1);
	int iy2 = (iy+1) & (wave_resolution-1);
	float fracx = x - ix;
	float fracy = y - iy;
	// compute the four normals at the corner points and interpolate between them according to
	// fracx/y
	vector3f a = get_wave_normal_at(ix , iy );
	vector3f b = get_wave_normal_at(ix2, iy );
	vector3f c = get_wave_normal_at(ix , iy2);
	vector3f d = get_wave_normal_at(ix2, iy2);
	vector3f e = a * (1.0f-fracx) + b * fracx;
	vector3f f = c * (1.0f-fracx) + d * fracx;
	vector3f g = e * (1.0f-fracy) + f * fracy;
	g.z *= (1.0f/rollfac);
	return g.normal();
}



void water::generate_wavetile()
{
//	cout << "time " << mytime << " rest " << myfmod(mytime, TIDECYCLE_TIME) << "\n";
	owg.set_time(myfmod(mytime, TIDECYCLE_TIME));
	owg.compute_heights(wavetileheights);
	minh = 1e10;
	maxh = -1e10;
	for (vector<float>::const_iterator it = wavetileheights.begin(); it != wavetileheights.end(); ++it) {
		minh = fmin(minh, *it);
		maxh = fmax(maxh, *it);
	}
	// choppy factor: formula from "waterengine": 0.5*WX/N = 0.5*wavelength/waveres, here = 1.0
	// fixme 5.0 default? - it seems that choppy waves don't look right. bug? fixme, with negative values it seems right. check this!
	// -2.0f also looks nice, -5.0f is too much. -1.0f should be ok
	owg.compute_displacements(-2.0f, wavetiledisplacements);
}



void water::generate_subdetail_and_bumpmap()
{
	float phase = myfrac(mytime / 20);
	for (unsigned k = 0; k < png.get_number_of_levels(); ++k) {
		// fixme: move each level with different speed...
		png.set_phase(0, phase, phase);	// fixme: depends on wind direction
	}
	waveheight_subdetail = png.generate();
#if 0
	ostringstream osgname;
	osgname << "noisemap" << mytime << ".pgm";
	ofstream osg(osgname.str().c_str());
	osg << "P5\n" << subdetail_size << " " << subdetail_size << "\n255\n";
	osg.write((const char*)(&waveheight_subdetail[0]), subdetail_size*subdetail_size);
#endif

	water_bumpmap.reset(new texture(waveheight_subdetail, subdetail_size, subdetail_size,
					GL_LUMINANCE,
#if 0
					texture::LINEAR,
#else
					texture::LINEAR_MIPMAP_LINEAR,
#endif
					texture::REPEAT,
					true, 1.0f));
			    //fixme: mipmap levels of normal map should be computed
			    //by this class, not glu!
			    //mipmap scaling of a normal map is not the same as the normal version
			    //of a mipmapped height map!
			    //really? the mipmapped normalmap values are not of unit length,
			    //but the direction should be kept, and they're normalized anyway.
			    //so mipmapping should do no harm...
}


void water::set_time(double tm)
{
	// do all the tasks here that should happen regularly, like recomputing new water or noisemaps
	mytime = tm;

	// water bumpmaps: ~15 times a second
	if (mytime >= last_subdetail_gen_time + SUBDETAIL_GEN_TIME) {
		generate_subdetail_and_bumpmap();
		last_subdetail_gen_time = mytime;
	}

	if (mytime >= last_wave_gen_time + WAVE_TILE_GEN_TIME) {
		generate_wavetile();
		last_wave_gen_time = mytime;
	}
}



float water::exact_fresnel(float x)
{
	// the real formula (recheck it!)
/*
	float g = 1.333f + x*x - 1;
	float z1 = g-x;
	float z2 = g+x;
	return (z1*z1)*(1+((x*z2-1)*(x*z2-1))/((x*z1+1)*(x*z1+1)))/(2*z2*z2);
*/

/*
	// a very crude guess
	float tmp = 1-4*x;
	if (tmp < 0.0f) tmp = 0.0f;
	return tmp;
*/
	// a good approximation (1/(x+1)^8)
	float x1 = x + 1.0f;
	float x2 = x1*x1;
	float x4 = x2*x2;
	return 1.0f/(x4*x4);
}



void water::set_refraction_color(float light_brightness)
{
	// compute wether a visible change has happened
	if (fabs(light_brightness - last_light_brightness) * 128 < 1.0f)
		return;
	last_light_brightness = light_brightness;

	// fixme: color depends also on weather. bad weather -> light is less bright
	// so this will be computed implicitly. Environment can also change water color
	// (light blue in tropic waters), water depth also important etc.

	color wavetop = color(color(10, 10, 10), color(18, 93, 77), light_brightness);
	color wavebottom = color(color(10, 10, 20), color(18, 73, 107), light_brightness);
	for (unsigned s = 0; s < REFRAC_COLOR_RES; ++s) {
		float fs = float(s)/(REFRAC_COLOR_RES-1);
		color c(wavebottom, wavetop, fs);
		for (unsigned f = 0; f < FRESNEL_FCT_RES; ++f) {
			fresnelcolortexd[(s*FRESNEL_FCT_RES+f)*4+0] = c.r;
			fresnelcolortexd[(s*FRESNEL_FCT_RES+f)*4+1] = c.g;
			fresnelcolortexd[(s*FRESNEL_FCT_RES+f)*4+2] = c.b;
			// update color only, leave fresnel term (alpha) intact
		}
	}
	fresnelcolortex.reset(new texture(fresnelcolortexd, FRESNEL_FCT_RES, REFRAC_COLOR_RES, GL_RGBA,
					  texture::LINEAR/*_MIPMAP_LINEAR*/, texture::CLAMP_TO_EDGE));
}
