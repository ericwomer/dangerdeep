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
#include "ocean_wave_generator.h"
#include "matrix4.h"
#include "cfg.h"
#include "system.h"
#include <fstream>

// compute projected grid efficiency, it should be 50-95%
//#define COMPUTE_EFFICIENCY
#define WAVE_SUB_DETAIL		// sub fft detail

// for testing
//#define DRAW_WATER_AS_GRID

// some more interesting values: phase 256, facesperwave 64+,
// wavelength 256+,
#define WAVE_PHASES 256		// no. of phases for wave animation
#define WAVE_RESOLUTION 64	// FFT resolution
#define WAVE_LENGTH 128.0	// in meters, total length of one wave tile
#define TIDECYCLE_TIME 10.0	// seconds
#define FOAM_VANISH_FACTOR 0.1	// 1/second until foam goes from 1 to 0.
#define FOAM_SPAWN_FACTOR 0.2	// 1/second until full foam reached. maybe should be equal to vanish factor

#define REFRAC_COLOR_RES 32
#define FRESNEL_FCT_RES 256

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

water::water(unsigned xres_, unsigned yres_, double tm) :
	mytime(tm), xres(xres_), yres(yres_), reflectiontex(0), foamtex(0), fresnelcolortex(0),
	last_light_brightness(-10000), water_bumpmap(0),
	vertex_program_supported(false),
	fragment_program_supported(false),
	compiled_vertex_arrays_supported(false),
	use_vertex_programs(false),
	use_fragment_programs(false),
	water_vertex_program(0),
	water_fragment_program(0)
{
	wavetiledisplacements.resize(WAVE_PHASES);
	wavetileheights.resize(WAVE_PHASES);
	wavetilenormals.resize(WAVE_PHASES);

	glGenTextures(1, &reflectiontex);
	glBindTexture(GL_TEXTURE_2D, reflectiontex);
	unsigned rx = sys().get_res_x();
	unsigned ry = sys().get_res_y();
	unsigned vps = texture::get_max_size();
	if (ry < vps)
		for (unsigned i = 1; i < ry; i *= 2) vps = i;

	vertex_program_supported = sys().extension_supported("GL_ARB_vertex_program");
	fragment_program_supported = sys().extension_supported("GL_ARB_fragment_program");
	compiled_vertex_arrays_supported = sys().extension_supported("GL_EXT_compiled_vertex_array");

	use_vertex_programs = cfg::instance().getb("use_vertex_shaders");
	use_fragment_programs = cfg::instance().getb("use_pixel_shaders");

	// initialize shaders if wanted
	if (fragment_program_supported && use_fragment_programs) {
		water_fragment_program = texture::create_shader(GL_FRAGMENT_PROGRAM_ARB,
								get_shader_dir() + "water_fp.shader");
	}

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
	reflectiontexsize = vps;
	// fixme: auto mipmap?
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	coords.resize((xres+1)*(yres+1));
	uv0.resize((xres+1)*(yres+1));
	uv1.resize((xres+1)*(yres+1));
	normals.resize((xres+1)*(yres+1));

	foamtex = new texture(get_texture_dir() + "foam.png", GL_LINEAR);//fixme maybe mipmap it

	fresnelcolortexd.resize(FRESNEL_FCT_RES*REFRAC_COLOR_RES);
	for (unsigned f = 0; f < FRESNEL_FCT_RES; ++f) {
		float ff = float(f)/(FRESNEL_FCT_RES-1);
		//maybe reduce reflections by using 192 or 224 instead of 255 here
		//looks better! sea water shows less reflections in reality
		//because it is so rough.
		Uint8 a = Uint8(255 /*192*/ * exact_fresnel(ff)+0.5f);
		for (unsigned s = 0; s < REFRAC_COLOR_RES; ++s) {
			fresnelcolortexd[s*FRESNEL_FCT_RES+f].a = a;
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

	ocean_wave_generator<float> owg(WAVE_RESOLUTION, vector2f(1,1), 20 /*10*/ /*31*/, 2e-6 /* 5e-6 */, WAVE_LENGTH, TIDECYCLE_TIME);
	minh = 1e10;
	maxh = -1e10;
	for (unsigned i = 0; i < WAVE_PHASES; ++i) {
		owg.set_time(i*TIDECYCLE_TIME/WAVE_PHASES);
		wavetileheights[i] = owg.compute_heights();
		for (vector<float>::const_iterator it = wavetileheights[i].begin(); it != wavetileheights[i].end(); ++it) {
			if (*it > maxh) maxh = *it;
			if (*it < minh) minh = *it;
		}
		// choppy factor: formula from "waterengine": 0.5*WX/N = 0.5*wavelength/waveres, here = 1.0
		// fixme 5.0 default? - it seems that choppy waves don't look right. bug? fixme, with negative values it seems right. check this!
		// -2.0f also looks nice, -5.0f is too much. -1.0f should be ok
		wavetiledisplacements[i] = owg.compute_displacements(-2.0f);

#if 1	// use finite normals
		// we need the normals to compute ship rolling
		wavetilenormals[i] = owg.compute_finite_normals(wavetileheights[i]);
		
#else	// use fft normals
#if 0		// compare both
		vector<vector3f> n2 = owg.compute_normals();
#else
		wavetilenormals[i] = owg.compute_normals();
#endif
#endif

#if 0		// draw finite and fft normals to compare them, just a test
/*
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);
*/		
		glColor3f(1,0,0);
		glBegin(GL_LINES);
		fy = 0;
		for (unsigned y = 0; y < tile_res; ++y) {
			float fx = 0;
			for (unsigned x = 0; x < tile_res; ++x) {
				glVertex3f(fx*WAVE_LENGTH, fy*WAVE_LENGTH, wavetileh[i][y*tile_res+x]);
				glVertex3f(fx*WAVE_LENGTH+4*wavetilen[i][y*tile_res+x].x, fy*WAVE_LENGTH+4*wavetilen[i][y*tile_res+x].y, wavetileh[i][y*tile_res+x]+4*wavetilen[i][y*tile_res+x].z);
				fx += add;
			}
			fy += add;
		}
		glEnd();
#endif
#if 0
		glColor3f(0,1,0);
		glBegin(GL_LINES);
		fy = 0;
		for (unsigned y = 0; y < tile_res; ++y) {
			float fx = 0;
			for (unsigned x = 0; x < tile_res; ++x) {
				glVertex3f(fx*WAVE_LENGTH, fy*WAVE_LENGTH, wavetileh[i][y*tile_res+x]);
				glVertex3f(fx*WAVE_LENGTH+4*n2[y*tile_res+x].x, fy*WAVE_LENGTH+4*n2[y*tile_res+x].y, wavetileh[i][y*tile_res+x]+4*n2[y*tile_res+x].z);
				fx += add;
			}
			fy += add;
		}
		glEnd();
#endif

	}

	vector<Uint8> wbtmp(WAVE_RESOLUTION*WAVE_RESOLUTION*3);
	for (unsigned i = 0; i < WAVE_RESOLUTION*WAVE_RESOLUTION; ++i) {
		wbtmp[3*i+0] = (wavetilenormals[0][i].x+1.0f)/2.0f*255;
		wbtmp[3*i+1] = (wavetilenormals[0][i].y+1.0f)/2.0f*255;
		wbtmp[3*i+2] = (wavetilenormals[0][i].z+1.0f)/2.0f*255;
	}
	water_bumpmap = new texture(&wbtmp[0], WAVE_RESOLUTION, WAVE_RESOLUTION,
				    GL_RGB, GL_LINEAR_MIPMAP_LINEAR, GL_REPEAT, false);

	add_loading_screen("water height data computed");
}


water::~water()
{
	if (fragment_program_supported && use_fragment_programs) {
		texture::delete_shader(water_fragment_program);
	}

	glDeleteTextures(1, &reflectiontex);
	delete foamtex;
	delete fresnelcolortex;
	delete water_bumpmap;
}


void water::setup_textures(const matrix4& reflection_projmvmat) const
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glDisable(GL_LIGHTING);

	if (fragment_program_supported && use_fragment_programs) {
		// use fragment programs

		glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, water_fragment_program);
		glEnable(GL_FRAGMENT_PROGRAM_ARB);

		// texture units / coordinates:
		// tex0: noise map (color normals) / matching texcoords
		// tex1: reflection map / matching texcoords
		// tex2: --- / vector to viewer
		glActiveTexture(GL_TEXTURE0);
		water_bumpmap->set_gl_texture();

		// local parameters:
		// local 0 : upwelling color
		glProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 0,
					     0.0, 0.1, 0.3, 1.0);//fixme test
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

	glActiveTexture(GL_TEXTURE1);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, reflectiontex);
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	// rescale coordinates [-1,1] to [0,1]
	glTranslated(0.5,0.5,0);
	glScaled(0.5,0.5,1.0);
	reflection_projmvmat.multiply_gl();
	glMatrixMode(GL_MODELVIEW);

	if (!(fragment_program_supported && use_fragment_programs)) {
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
	if (fragment_program_supported && use_fragment_programs) {
		glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, 0);
		glDisable(GL_FRAGMENT_PROGRAM_ARB);
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
vector3f water::compute_coord(int phase, const vector3f& xyzpos, const vector2f& transl) const
{
	// generate values with mipmap function (needs viewer pos.)
	float xfrac = myfrac(float((xyzpos.x + transl.x) / WAVE_LENGTH)) * WAVE_RESOLUTION;
	float yfrac = myfrac(float((xyzpos.y + transl.y) / WAVE_LENGTH)) * WAVE_RESOLUTION;
	unsigned x0 = unsigned(floor(xfrac));
	unsigned y0 = unsigned(floor(yfrac));
	unsigned x1 = (x0 + 1) & (WAVE_RESOLUTION-1);
	unsigned y1 = (y0 + 1) & (WAVE_RESOLUTION-1);
	xfrac = myfrac(xfrac);
	yfrac = myfrac(yfrac);
	unsigned i0 = x0+y0*WAVE_RESOLUTION, i1 = x1+y0*WAVE_RESOLUTION, i2 = x0+y1*WAVE_RESOLUTION,
		i3 = x1+y1*WAVE_RESOLUTION;

	// z distance (for triliniar filtering) is constant along one projected grid line
	// so compute it per line.
	// if mipmap level is < 0, we may add some extra detail (noise)
	// this could be fft itself (height scaled) or some perlin noise
	// take x,y fraction as texture coordinates in perlin map.
	// linear filtering should be neccessary -> expensive!

	// fixme: make trilinear
	// fixme: unite displacements and height in one vector3f for simplicity's sake and performance gain
	// bilinear interpolation of displacement and height	
	vector3f ca(wavetiledisplacements[phase][i0].x, wavetiledisplacements[phase][i0].y, wavetileheights[phase][i0]);
	vector3f cb(wavetiledisplacements[phase][i1].x, wavetiledisplacements[phase][i1].y, wavetileheights[phase][i1]);
	vector3f cc(wavetiledisplacements[phase][i2].x, wavetiledisplacements[phase][i2].y, wavetileheights[phase][i2]);
	vector3f cd(wavetiledisplacements[phase][i3].x, wavetiledisplacements[phase][i3].y, wavetileheights[phase][i3]);
	float fac0 = (1.0f-xfrac)*(1.0f-yfrac);
	float fac1 = xfrac*(1.0f-yfrac);
	float fac2 = (1.0f-xfrac)*yfrac;
	float fac3 = xfrac*yfrac;
	vector3f coord = (ca*fac0 + cb*fac1 + cc*fac2 + cd*fac3) + xyzpos;

#ifdef WAVE_SUB_DETAIL
	// add some extra detail to near waves.
	// Every rectangle between four FFT values has
	// some extra detail. Pattern must be more chaotic to be realistic, a reapeating
	// pattern every 2m is not good. so use x0,y0 too.
/*
	// old code. matches new code with tilefac = 1
	int ixf = int(WAVE_RESOLUTION * xfrac), iyf = int(WAVE_RESOLUTION * yfrac);
	float addh = wavetileheights[phase][ixf+iyf*WAVE_RESOLUTION] * (1.0f/WAVE_RESOLUTION);
	coord.z += addh;
*/
	// 0 <= x0,y0 < WAVE_RESOLUTION
	// additional detail is taken from same fft values, but that's not very good (similar
	// pattern is noticeable), so we make a phase shift (maybe some perlin noise values
	// would be better)
	int tilefac = 8;//4;
	int rfac = WAVE_RESOLUTION /tilefac;
	int ixf = (x0 & (tilefac-1))*rfac + int(rfac * xfrac);
	int iyf = (y0 & (tilefac-1))*rfac + int(rfac * yfrac);
	float addh = wavetileheights[(phase+WAVE_PHASES/2)%WAVE_PHASES][ixf + iyf * WAVE_RESOLUTION] * (1.0f/rfac);
	coord.z += addh;
#endif
	return coord;
}


void water::display(const vector3& viewpos, angle dir, double max_view_dist, const matrix4& reflection_projmvmat) const
{
	int phase = int((myfmod(mytime, TIDECYCLE_TIME)/TIDECYCLE_TIME) * WAVE_PHASES);
	const float VIRTUAL_PLANE_HEIGHT = 25.0f;	// fixme experiment, amount of reflection distorsion, 30.0f seems ok, maybe a bit too much

	// maximum height of waves (half amplitude)
	double WAVE_HEIGHT = (maxh > fabs(minh)) ? maxh : fabs(minh);

	// fixme: always add elevation to projector.z?
	const double ELEVATION = 10.0; // fixme experiment

	// fixme: displacements must be used to enlarge projector area or else holes will be visible at the border
	// of the screen (left and right), especially on glasses mode

	// get projection and modelview matrix
	matrix4 proj = matrix4::get_gl(GL_PROJECTION_MATRIX);
	matrix4 modl = matrix4::get_gl(GL_MODELVIEW_MATRIX);
	matrix4 inv_modl = modl.inverse();

	// modify modelview matrix so that viewer is at (0,0,h) with h in |R with h = cameraheight
	vector3 correction = inv_modl.column(3);

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
	vector3 cameraforward = -inv_modl.column(2); // camera is facing along negative z-axis
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
	double af = fabs(cameraforward.z);
	projectorforward = (aimpoint * af + aimpoint2 * (1.0-af)) - projectorpos;

//cout << "projectorpos " << projectorpos << "\n";
//cout << "projector forward " << projectorforward << "\n";
	
	// compute rest of the projector matrix from pos and forward vector
	vector3 pjz = -projectorforward.normal();
	vector3 pjx = vector3(0,0,1).cross(pjz).normal(); // fixme: what if pjz==up vector (or very near it) then errors occour, they're visible!
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
	
	// compute min-max values and range matrix
	// fixme: the efficiency is mostly around 50%. We could increase it dramatically if
	// we don't take a bounding rectangle around the projected points, but some other
	// shape. Mostly the points form a trapez, and mostly the number of points is six.
	// Best solution: shape with exactly four points and minimum area surrounding all
	// points (convex hull with four points). Or compute the trapez.
	// Compute a shape so that we can transform a rectangle with a 4x4 matrix to the shape.
	// This gives nearly 100% efficiency, which should improve detail a lot (especially in
	// zoomscope/glasses view). We don't really need mrange but could generate any quadri-
	// lateral with x/y values, but it's simpler to use mrange.
	// If (!) we can generate a tranformation matrix that makes a trapez from a rectangle.
	// This seams impossible... ?!
	// improving the efficiency here should give the biggest gain.
	// drawing half of the triangles gives 33% more performanc, so 25% of performance
	// is currently wasted!
	matrix4 rangeprojector2world;
	if (proj_points.size() > 0) {
		double x_min = proj_points[0].x, x_max = proj_points[0].x, y_min = proj_points[0].y, y_max = proj_points[0].y;
//cout << "pts0.xy " << x_min << ", " << y_min << "\n";
		for (vector<vector3>::iterator it = ++proj_points.begin(); it != proj_points.end(); ++it) {
//cout << "ptsi.xy " << it->x << ", " << it->y << "\n";
			if (it->x < x_min) x_min = it->x;
			if (it->x > x_max) x_max = it->x;
			if (it->y < y_min) y_min = it->y;
			if (it->y > y_max) y_max = it->y;
		}
		matrix4 mrange(x_max-x_min, 0, 0, x_min, 0, y_max-y_min, 0, y_min, 0, 0, 1, 0, 0, 0, 0, 1);
//cout << "x_min " << x_min << " x_max " << x_max << " y_min " << y_min << " y_max " << y_max << "\n";
		rangeprojector2world = projector2world * mrange;
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
		vector3 v1 = rangeprojector2world * vector3(0,y,-1);
		vector3 v2 = rangeprojector2world * vector3(0,y,+1);
		vector3 v3 = rangeprojector2world * vector3(1,y,-1);
		vector3 v4 = rangeprojector2world * vector3(1,y,+1);
		// compute intersection with z = 0 plane here
		// we could compute intersection with earth's sphere here for a curved display
		// of water to the horizon, fixme
		double t1 = -v1.z/(v2.z-v1.z), t2 = -v3.z/(v4.z-v3.z);
		vector2 va = v1.xy() * (1-t1) + v2.xy() * t1;
		vector2 vb = v3.xy() * (1-t2) + v4.xy() * t2;
		for (unsigned xx = 0; xx <= xres; ++xx, ++ptr) {
			double x = double(xx)/xres;
			vector3f v(va.x * (1-x) + vb.x * x, va.y * (1-x) + vb.y * x, -viewpos.z);
			coords[ptr] = compute_coord(phase, v, transl);
#ifdef COMPUTE_EFFICIENCY
			vector3 tmp = world2camera * vector3(coords[ptr].x, coords[ptr].y, coords[ptr].z);
			if (fabs(tmp.x) <= 1.0 && fabs(tmp.y) <= 1.0 && fabs(tmp.z) <= 1.0)
				++vertices_inside;
			++vertices;
#endif
		}
	}

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
			vector3f n0 = (p1 - p0).cross(p2 - p0);
			vector3f n1 = (p2 - p0).cross(p3 - p0);
			normals[i0] += n0;
			normals[i1] += n0;
			normals[i2] += n0;
			normals[i0] += n1;
			normals[i2] += n1;
			normals[i3] += n1;
		}
	}

	// make normals normal ;-)
	//fixme: this could be done on the GPU with vertex shaders.
	for (unsigned i = 0; i < (xres+1)*(yres+1); ++i)
		normals[i].normalize();

	// compute remaining data (Fresnel etc.)
	//fixme: the whole loop could be done on the GPU with vertex shaders.
	//coords and normals change per frame, so no display lists can be used.
	//simple vertex arrays with locking should do the trick, maybe use
	//(locked) quadstrips, if they're faster than compiled vertex arrays, test it!

	vector<vector3f> uv2;
	if (fragment_program_supported && use_fragment_programs) {
		uv2.resize(uv0.size());
	}

	for (unsigned yy = 0, ptr = 0; yy <= yres; ++yy) {
		for (unsigned xx = 0; xx <= xres; ++xx, ++ptr) {
			// the coordinate is the same as the relative coordinate, because the viewer is at 0,0,0
			const vector3f& coord = coords[ptr];
			const vector3f& N = normals[ptr];

			if (fragment_program_supported && use_fragment_programs) {
				uv0[ptr] = vector2f(coord.x/4.0f, coord.y/4.0f); // fixme, use noise map texc's
				//fixme ^, offset is missing
				vector3f tx = vector3f(1, 0, 0);//fixme hack
				vector3f ty = N.cross(tx);
				vector3f tz = N;
				vector3f worldE = -coord;
				vector3f tangentE = vector3f(tx * worldE, ty * worldE, tz * worldE);
				uv2[ptr] = tangentE;
				// we have to convert E to the tangent space of this face or
				// do that per pixel at the fragment program
			} else {
				float rel_coord_length = coord.length();
				vector3f E = -coord * (1.0f/rel_coord_length); // viewer is in (0,0,0)
				float F = E*N;		// compute Fresnel term F(x) = ~ 1/(x+1)^8
				// value clamping is done by texture unit.
				// water color depends on height of wave and slope
				// slope (N.z) it mostly > 0.8
				float colorfac = (coord.z + viewpos.z + 3) / 9 + (N.z - 0.8f);
				uv0[ptr] = vector2f(F, colorfac);	// set fresnel and water color
			}

			// reflection texture coordinates (should be tweaked per pixel with fp, fixme)
			// they are broken with fp, reason unknown
			vector3f texc = coord + N * (VIRTUAL_PLANE_HEIGHT * N.z);
			texc.z -= VIRTUAL_PLANE_HEIGHT;
			uv1[ptr] = texc;
		}
	}

	// set up textures
	setup_textures(reflection_projmvmat);

	glColor4f(1,1,1,1);

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

	if (fragment_program_supported && use_fragment_programs) {
		glClientActiveTexture(GL_TEXTURE2);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(3, GL_FLOAT, sizeof(vector3f), &uv2[0].x);
	}

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
	if (fragment_program_supported && use_fragment_programs) {
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
	double t = myfrac(mytime/TIDECYCLE_TIME);
	int wavephase = int(WAVE_PHASES*t);
	float ffac = WAVE_RESOLUTION/WAVE_LENGTH;
	float x = float(myfmod(pos.x, WAVE_LENGTH)) * ffac;
	float y = float(myfmod(pos.y, WAVE_LENGTH)) * ffac;
	int ix = int(floor(x));
	int iy = int(floor(y));
	int ix2 = (ix+1)%WAVE_RESOLUTION;
	int iy2 = (iy+1)%WAVE_RESOLUTION;
	float fracx = x - ix;
	float fracy = y - iy;
	float a = wavetileheights[wavephase][ix+iy*WAVE_RESOLUTION];
	float b = wavetileheights[wavephase][ix2+iy*WAVE_RESOLUTION];
	float c = wavetileheights[wavephase][ix+iy2*WAVE_RESOLUTION];
	float d = wavetileheights[wavephase][ix2+iy2*WAVE_RESOLUTION];
	float e = a * (1.0f-fracx) + b * fracx;
	float f = c * (1.0f-fracx) + d * fracx;
	return (1.0f-fracy) * e + fracy * f;
}



vector3f water::get_normal(const vector2& pos, double f) const
{
	double t = myfrac(mytime/TIDECYCLE_TIME);
	int wavephase = int(WAVE_PHASES*t);
	float ffac = WAVE_RESOLUTION/WAVE_LENGTH;
	float x = float(myfmod(pos.x, WAVE_LENGTH)) * ffac;
	float y = float(myfmod(pos.y, WAVE_LENGTH)) * ffac;
	int ix = int(floor(x));
	int iy = int(floor(y));
	int ix2 = (ix+1)%WAVE_RESOLUTION;
	int iy2 = (iy+1)%WAVE_RESOLUTION;
	float fracx = x - ix;
	float fracy = y - iy;
	vector3f a = wavetilenormals[wavephase][ix+iy*WAVE_RESOLUTION];
	vector3f b = wavetilenormals[wavephase][ix2+iy*WAVE_RESOLUTION];
	vector3f c = wavetilenormals[wavephase][ix+iy2*WAVE_RESOLUTION];
	vector3f d = wavetilenormals[wavephase][ix2+iy2*WAVE_RESOLUTION];
	vector3f e = a * (1.0f-fracx) + b * fracx;
	vector3f g = c * (1.0f-fracx) + d * fracx;
	vector3f h = e * (1.0f-fracy) + g * fracy;
	h.z *= (1.0f/f);
	return h.normal();
}



void water::set_time(double tm)
{
	mytime = tm;
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
			color& tgt = fresnelcolortexd[s*FRESNEL_FCT_RES+f];
			// update color only, leave fresnel term (alpha) intact
			tgt.r = c.r;
			tgt.g = c.g;
			tgt.b = c.b;
		}
	}
	fresnelcolortex = new texture(&fresnelcolortexd[0], FRESNEL_FCT_RES, REFRAC_COLOR_RES, GL_RGBA,
				 GL_LINEAR/*_MIPMAP_LINEAR*/, GL_CLAMP_TO_EDGE, false);
}








// old code
#if 0

	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, water_bumpmaps[unsigned(framepart*WATER_BUMP_FRAMES)]->get_opengl_name());

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
	glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_DOT3_RGB); 
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PRIMARY_COLOR);
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE);
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);

	GLfloat scalefac0 = 1.0f/WAVE_LENGTH;
	GLfloat plane_s0[4] = { scalefac0, 0.0f, 0.0f, 0.0f };
	GLfloat plane_t0[4] = { 0.0f, scalefac0, 0.0f, 0.0f };
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGenfv(GL_S, GL_OBJECT_PLANE, plane_s0);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGenfv(GL_T, GL_OBJECT_PLANE, plane_t0);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);

	glActiveTexture(GL_TEXTURE1);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, foamtex->get_opengl_name());
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
	glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_INTERPOLATE);
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PREVIOUS);
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE);
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_PRIMARY_COLOR);//fixme: 2x source1 set?!
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);

	GLfloat scalefac1 = 32.0f/WAVE_LENGTH;//fixme: what is/was this?
	GLfloat plane_s1[4] = { scalefac1, 0.0f, 0.0f, 0.0f };
	GLfloat plane_t1[4] = { 0.0f, scalefac1, 0.0f, 0.0f };
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGenfv(GL_S, GL_OBJECT_PLANE, plane_s1);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGenfv(GL_T, GL_OBJECT_PLANE, plane_t1);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);

	glDisable(GL_LIGHTING);
	glBegin(GL_TRIANGLE_STRIP);
	if (onlyflatwater) {
		glVertex3f(c0,c3,0);
		glVertex3f(c0,c0,0);
		glVertex3f(c3,c3,0);
		glVertex3f(c3,c0,0);
	} else {
		glVertex3f(c0,c3,0);
		glVertex3f(c1,c2,wz);
		glVertex3f(c3,c3,0);
		glVertex3f(c2,c2,wz);
		glVertex3f(c3,c0,0);
		glVertex3f(c2,c1,wz);
		glVertex3f(c0,c0,0);
		glVertex3f(c1,c1,wz);
		glVertex3f(c0,c3,0);
		glVertex3f(c1,c2,wz);
	}
	glEnd();


	if (!onlyflatwater) {	
		// draw waves
		double timefac = myfmod(mytime, TIDECYCLE_TIME)/TIDECYCLE_TIME;

		// fixme: use LOD (fft with less resolution) for distance waves
		// until about 10km to the horizon

/*
		glActiveTexture(GL_TEXTURE1);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, wavefoamtex);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
		glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PREVIOUS);
		glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
		glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE);
		glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
*/
		//fixme: automatic texture coordinate generation ignores wave displacements (choppy waves)
		//so foam is mapped wrongly!
		GLfloat scalefac2 = 1.0f/WAVE_LENGTH; //1.0f/(WAVE_LENGTH*WAVES_PER_AXIS);
		GLfloat plane_s2[4] = { scalefac2, 0.0f, 0.0f, 0.0f };
		GLfloat plane_t2[4] = { 0.0f, scalefac2, 0.0f, 0.0f };
/*
		glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
		glTexGenfv(GL_S, GL_OBJECT_PLANE, plane_s2);
		glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
		glTexGenfv(GL_T, GL_OBJECT_PLANE, plane_t2);
		glEnable(GL_TEXTURE_GEN_S);
		glEnable(GL_TEXTURE_GEN_T);
*/
		unsigned dl = wavedisplaylists + int(WAVE_PHASES*timefac);


		// ************ draw water tiles ****************
		if (false /* draw_only_tiles_in_viewing_cone */) {
			// *********** draw water tiles in viewing cone

			// compute the rasterization of the triangle p0,p1,p2
			// with p0 = viewer's pos., p1,2 = p0 + viewrange * direction(brearing +,- fov/2)

			vector2 p[3];	// vertices for triangle
			p[0] = vector2(myfmod(viewpos.x, WAVE_LENGTH)/WAVE_LENGTH, myfmod(viewpos.y, WAVE_LENGTH)/WAVE_LENGTH);
			p[1] = p[0] + (dir+angle(45/*fov/2*/)).direction() * 8 /* view dist */;
			p[2] = p[0] + (dir-angle(45/*fov/2*/)).direction() * 8 /* view dist */;

		char tmp[21][21];
		memset(tmp, '.', 21*21);
		tmp[10][10]='o';

			// rasterize a ccw triangle, coordinate system is right handed (greater y is top)
			struct edge {
				double x, dx;
				int y, height;
				inline void step(void) { x += dx; --y; --height; }
				edge(const vector2& ptop, const vector2& pbot) {
					y = int(floor(ptop.y));
					height = y - int(floor(pbot.y));
					dx = (ptop.x - pbot.x)/(pbot.y - ptop.y);
					x = dx*(ptop.y - y) + ptop.x;
				}
			};

			int top = 0, middle = 1, bottom = 2;
			bool middle_is_left = true;
			if (p[1].y > p[top].y) top = 1;
			if (p[2].y > p[top].y) top = 2;
			middle = (top+1)%3;
			bottom = (top+2)%3;
			if (p[middle].y < p[bottom].y) {
				middle_is_left = false;
				bottom = (top+1)%3;
				middle = (top+2)%3;
			}
			edge top_bottom(p[top], p[bottom]);
			edge top_middle(p[top], p[middle]);
			edge middle_bottom(p[middle], p[bottom]);
			edge *eleft, *eright;
			if (middle_is_left) {
				eleft = &top_middle; eright = &top_bottom;
			} else {
				eleft = &top_bottom; eright = &top_middle;
			}
			// draw triangle, first upper half, then lower half
			int h = top_middle.height;
			int half_rasterized = 0;
cout << "raster test\np[0]: "<<p[0]<<"\np[1]: "<<p[1]<<"\np[2]: "<<p[2]<<"\ntop "<<top<<" middle "<<middle<<" bottom "<<bottom<<" m_is_l:"<<middle_is_left<<"\n";
			while (true) {
				for ( ; h > 0; --h) {
cout << "eleft: x=" << eleft->x << " y=" << eleft->y << " h=" <<eleft->height << " dx="<<eleft->dx<<"\n";
cout << "eright: x=" << eright->x << " y=" << eright->y << " h=" <<eright->height << " dx="<<eright->dx<<"\n";
					//raster a line
					for (int i = int(floor(eleft->x)); i < int(ceil(eright->x)); ++i) {
						//draw tile [i,eleft->y]
						cout<<"raster draw "<<i<<","<<eleft->y<<"\n";
						tmp[i+10][(eleft->y)+10] = 'x';
						int x = i + WAVES_PER_AXIS/2;
						int y = eleft->y + WAVES_PER_AXIS/2;
						glPushMatrix();
						glTranslatef((-WAVES_PER_AXIS/2+x)*WAVE_LENGTH, (-WAVES_PER_AXIS/2+y)*WAVE_LENGTH, 0);
						glCallList(dl);
						glPopMatrix();
					}
					eleft->step();
					eright->step();
				}
				++half_rasterized;
				if (half_rasterized == 2) break;
				// change edge pointers
				if (middle_is_left)
					eleft = &middle_bottom;
				else
					eright = &middle_bottom;
				h = middle_bottom.height;
			}
cout << "raster result\n";
for(int ry=0;ry<21;++ry){
for(int rx=0;rx<21;++rx){
cout<<tmp[rx][20-ry];
}
cout<<"\n";
}
		} else {	// draw all tiles in visible range

			for (int y = 0; y < WAVES_PER_AXIS; ++y) {
				plane_t2[3] = float(y)/WAVES_PER_AXIS;
				glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
				glTexGenfv(GL_T, GL_OBJECT_PLANE, plane_t2);
				for (int x = 0; x < WAVES_PER_AXIS; ++x) {
					plane_s2[3] = float(x)/WAVES_PER_AXIS;
					glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
					glTexGenfv(GL_S, GL_OBJECT_PLANE, plane_s2);
					glPushMatrix();
					glTranslatef((-WAVES_PER_AXIS/2+x)*WAVE_LENGTH, (-WAVES_PER_AXIS/2+y)*WAVE_LENGTH, 0);
					glCallList(dl);
					glPopMatrix();
				}
			}
		}
	}

	glActiveTexture(GL_TEXTURE1);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glDisable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
#endif

