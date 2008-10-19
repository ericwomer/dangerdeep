/*
Danger from the Deep - Open source submarine simulation
Copyright (C) 2003-2006  Thorsten Jordan, Luis Barrancos and others.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

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
#include "game.h"
#include "datadirs.h"
#include "polygon.h"
#include "frustum.h"
#include "log.h"
#include <fstream>
#include <sstream>
#include <iomanip>

using std::ofstream;
using std::ostringstream;
using std::setw;
using std::setfill;
using std::vector;
using std::list;

#ifndef fmin
#define fmin(x,y) (x<y) ? x : y
#endif
#ifndef fmax
#define fmax(x,y) (x>y) ? x : y
#endif


// fixme: allow rendering of geoclipmap patches as grid/lines for easier debugging

/*
Notes about geoclipmaps and this implementation:
- we partition the torus shape of a level in 8 rectangles,
  each called a patch. Because of size variations, we have
  9 versions of each patch. We can precompute all patches.
  However these consist only of indices and can be
  computed quickly on the fly, so we wouldnt't need to
  store them in VBOs.

  There are two tricks with geoclipmaps that this approach is missing:
  - view culling could be done finer: crop the torus by the visible
    area of the xy plane of the current level and render only the
    rest. This gives rectangles of any size making up the level.
    This is a bit more complex to compute and can't be precomputed,
    but we could render more fps.
  - Blending the height from one level to the coarser next level near
    the border gives a smooth geometric transition without the need
    of skirts between the levels. At the moment we have complex
    index lists for the patches to handle the boundaries with
    complex use of triangle strips and many degenerated triangles.
    geoclipmaps use only regular tri-strips making lines of quads.
    zero-area (not degenerated!) triangles are rendered between
    the levels as skirts.

- geoclipmap computes the regions for each level first, and then
  renders. the approach is more mathematical, but has some
  advantages, like skipping finer regions when update would be
  too costly

summary:
- we could change this code to be more similar to the geoclipmap
  algorithm from the paper
- then we can reuse most code for static terrain and dynamic water
- drawback is that we need to give for each vertex the height
  of it in the coarser level. For water with x/y displacement,
  we would need to give the full vertex of next coarser level
  to blend it. This gives additional 3 vertex attributes per
  vertex, that need to get updated every frame (which means
  CPU-bound interpolation for 3/4 of the values).
  This is complex and also costly.
  A serious drawback and the major reason why the geoclipmap
  implementation here is different from the paper.
  We have 7 vertex attributes per vertex:
  3x coord, 3x normal, 1x amount of foam.
  With smooth level transition we would need to give all 7 attributes
  for both levels, if we would do it right. This is overkill,
  but at least giving the next coarser coordinate raised
  the per-vertex attribute count from 7 to 10.

conclusions:
- better keep this working code as it is.
- the visible level transitions are not that bad, since viewer moves
  only slowly
- implement the geoclipmap closer to the paper for static terrain
  and reuse only the knowledge.
*/

#define REFRAC_COLOR_RES 32
#define FRESNEL_FCT_RES 256

#define FOAMAMOUNTRES 256

#undef  MEASURE_WAVE_HEIGHTS

const unsigned foamtexsize = 256;

#ifdef MEASURE_WAVE_HEIGHTS
static float totalmin = 0, totalmax = 0;
#endif

// computes valid detail values
static unsigned cmpdtl(int x)
{
	if (x < 4 || x > 512) return 128;
	return nextgteqpow2(unsigned(x));
}


water::water(double tm) :
	mytime(tm),
	wave_phases(cfg::instance().geti("wave_phases")),
	wavetile_length(cfg::instance().getf("wavetile_length")),
	wavetile_length_rcp(1.0f/wavetile_length),
	wave_tidecycle_time(cfg::instance().getf("wave_tidecycle_time")),
	last_light_color(-1, -1, -1),
	wave_resolution(nextgteqpow2(cfg::instance().geti("wave_fft_res"))),
	wave_resolution_shift(ulog2(wave_resolution)),
	wavetile_data(wave_phases),
	curr_wtp(0),
	owg(wave_resolution,
	    vector2f(1,1), // wind direction
	    12 /*12*/ /*10*/ /*31*/,	// wind speed m/s. fixme make dynamic (weather!)
	    wave_resolution * (1e-8) /* roughly 2e-6 for 128 */,	// scale factor for heights. depends on wave resolution, maybe also on tidecycle time
	    wavetile_length,
	    wave_tidecycle_time),
	use_hqsfx(false),
	vattr_aof_index(0),
	rerender_new_wtp(true),
	geoclipmap_resolution(cmpdtl(cfg::instance().geti("water_detail"))), // should be power of two
	geoclipmap_levels(wave_resolution_shift-2),
	patches(1 + (geoclipmap_levels-1)*8*3*3 + 4 /*horizon*/)
{
	// generate geoclipmap index data.
	patches.reset(0, new geoclipmap_patch(geoclipmap_resolution,
					      0, 0, 0, 0, geoclipmap_resolution, geoclipmap_resolution));
	unsigned N2 = geoclipmap_resolution/2, N4 = geoclipmap_resolution/4, N34 = geoclipmap_resolution*3/4;
	for (unsigned j = 1; j < geoclipmap_levels; ++j) {
		// each level has 8 patches and 9 variations of each patch.
		// the variation is in height/width
		// width/height is standard width/height -1,0,1
		// patches are bottom left to top right:
		// 567
		// 3 4
		// 012
		for (int y = -1; y <= 1; ++y) {
			for (int x = -1; x <= 1; ++x) {
				unsigned pn = 1 + (j-1)*9*8 + ((y+1) * 3 + (x+1)) * 8;
				patches.reset(pn + 0, new geoclipmap_patch(geoclipmap_resolution, j, 0x80,       0,       0, N4+x  , N4+y  ));
				patches.reset(pn + 1, new geoclipmap_patch(geoclipmap_resolution, j, 0x01, N4 +x  ,       0, N2    , N4+y-1));
				patches.reset(pn + 2, new geoclipmap_patch(geoclipmap_resolution, j, 0x40, N34+x  ,       0, N4-x  , N4+y  ));
				patches.reset(pn + 3, new geoclipmap_patch(geoclipmap_resolution, j, 0x02,       0, N4 +y  , N4+x-1, N2    ));
				patches.reset(pn + 4, new geoclipmap_patch(geoclipmap_resolution, j, 0x08, N34+x+1, N4 +y  , N4-x-1, N2    ));
				patches.reset(pn + 5, new geoclipmap_patch(geoclipmap_resolution, j, 0x10,       0, N34+y  , N4+x  , N4-y  ));
				patches.reset(pn + 6, new geoclipmap_patch(geoclipmap_resolution, j, 0x04, N4 +x  , N34+y+1, N2    , N4-y-1));
				patches.reset(pn + 7, new geoclipmap_patch(geoclipmap_resolution, j, 0x20, N34+x  , N34+y  , N4-x  , N4-y  ));
			}
		}
	}
	// horizon, for North, East, South, West part
	unsigned pn = patches.size() - 4;
	patches.reset(pn + 0, new geoclipmap_patch(geoclipmap_resolution, geoclipmap_levels-1, 0x01));
	patches.reset(pn + 1, new geoclipmap_patch(geoclipmap_resolution, geoclipmap_levels-1, 0x02));
	patches.reset(pn + 2, new geoclipmap_patch(geoclipmap_resolution, geoclipmap_levels-1, 0x04));
	patches.reset(pn + 3, new geoclipmap_patch(geoclipmap_resolution, geoclipmap_levels-1, 0x08));

#if 0 // analysis
	unsigned idxtotal = 0;
	for (unsigned j = 0; j < patches.size(); ++j)
		idxtotal += patches[j]->get_nr_indices();
	printf("generated %u indices total in %u VBOs, using %u bytes of video ram.\n",
	       idxtotal, patches.size(), idxtotal * 4 /* uint32! */);
	// 261598 indices with N=64, using 1046392 (<1MB) of video ram with uint32 indices
#endif

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
	const unsigned reflection_scale = 2;
	rx = std::min(rx, vps);
	ry = std::min(ry, vps);
	rx /= reflection_scale;
	ry /= reflection_scale;
	if (!texture::size_non_power_two()) {
		// choose next bigger power of two
		rx = nextgteqpow2(rx);
		ry = nextgteqpow2(ry);
	}
	// fixme: make ^ that configureable! reflection doesn't need to have that high detail...
	// fixme: auto mipmap?
	reflectiontex.reset(new texture(rx, ry, GL_RGB, texture::LINEAR, texture::CLAMP));

	log_info("wave resolution " << wave_resolution << " (shift=" << wave_resolution_shift << ")");
	log_info("reflection image size " << rx << "x" << ry);
	log_info("water detail: " << geoclipmap_resolution);

	use_hqsfx = cfg::instance().getb("use_hqsfx");

	// initialize shaders
	glsl_water.reset(new glsl_shader_setup(get_shader_dir() + "water.vshader",
					       get_shader_dir() + "water.fshader"));
	glsl_under_water.reset(new glsl_shader_setup(get_shader_dir() + "under_water.vshader",
						     get_shader_dir() + "under_water.fshader"));
	glsl_water->use();
	vattr_aof_index = glsl_water->get_vertex_attrib_index("amount_of_foam");
	loc_w_noise_xform_0 = glsl_water->get_uniform_location("noise_xform_0");
	loc_w_noise_xform_1 = glsl_water->get_uniform_location("noise_xform_1");
	loc_w_reflection_mvp = glsl_water->get_uniform_location("reflection_mvp");
	loc_w_viewpos = glsl_water->get_uniform_location("viewpos");
	loc_w_upwelltop = glsl_water->get_uniform_location("upwelltop");
	loc_w_upwellbot = glsl_water->get_uniform_location("upwellbot");
	loc_w_upwelltopbot = glsl_water->get_uniform_location("upwelltopbot");
	loc_w_tex_normal = glsl_water->get_uniform_location("tex_normal");
	loc_w_tex_reflection = glsl_water->get_uniform_location("tex_reflection");
	loc_w_tex_foam = glsl_water->get_uniform_location("tex_foam");
	loc_w_tex_foamamount = glsl_water->get_uniform_location("tex_foamamount");
	loc_w_foam_transform = glsl_water->get_uniform_location("foam_transform");
	loc_w_reflection_transform = glsl_water->get_uniform_location("reflection_transform");
	glsl_under_water->use();
	loc_uw_noise_xform_0 = glsl_under_water->get_uniform_location("noise_xform_0");
	loc_uw_noise_xform_1 = glsl_under_water->get_uniform_location("noise_xform_1");
	loc_uw_viewpos = glsl_under_water->get_uniform_location("viewpos");
	loc_uw_upwelltop = glsl_under_water->get_uniform_location("upwelltop");
	loc_uw_upwellbot = glsl_under_water->get_uniform_location("upwellbot");
	loc_uw_upwelltopbot = glsl_under_water->get_uniform_location("upwelltopbot");
	loc_uw_tex_normal = glsl_under_water->get_uniform_location("tex_normal");

	foamtex.reset(new texture(get_texture_dir() + "foam.png", texture::LINEAR, texture::REPEAT));//fixme maybe mipmap it
	foamamounttex.reset(new texture(FOAMAMOUNTRES, FOAMAMOUNTRES, GL_RGB, texture::LINEAR, texture::CLAMP));

	foamamounttrail.reset(new texture(get_texture_dir() + "foamamounttrail.png", texture::LINEAR, texture::REPEAT));//fixme maybe mipmap it

	// check FBO usage
	if (framebufferobject::supported()) {
		log_info("using opengl frame buffer objects");
		reflectiontex_fbo.reset(new framebufferobject(*reflectiontex, true));
		foamamounttex_fbo.reset(new framebufferobject(*foamamounttex, false));
	}

	const unsigned perimetertexs = 256;
	const unsigned perimetertexborder = 32;
	vector<Uint8> perimetertex(perimetertexs * perimetertexs * 2);
	unsigned perimetertexptr = 0;
	for (unsigned y = 0; y < perimetertexs; ++y) {
		float fy = y - 127.5f;
		for (unsigned x = 0; x < perimetertexs; ++x) {
			float fx = x - 127.5f;
			unsigned d = unsigned(sqrt(fy*fy + fx*fx));
			Uint8 a = 0;
			if (d < perimetertexs/2) {
				if (d >= perimetertexs/2 - perimetertexborder) {
					a = Uint8(255*(perimetertexs/2 - d)/perimetertexborder);
				} else {
					a = 255;
				}
			}
			perimetertex[perimetertexptr + 0] = 255;
			perimetertex[perimetertexptr + 1] = a;
			perimetertexptr += 2;
		}
	}
	foamperimetertex.reset(new texture(perimetertex, perimetertexs, perimetertexs,
					   GL_LUMINANCE_ALPHA, texture::LINEAR, texture::CLAMP));

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

	// multithreaded construction of water data (faster).
	// spawn 1 more thread (or 3 on 4-core cpus, but two threads are already fast enough)
	thread::auto_ptr<worker> myworker;
	if (true /* construction multithreaded */) {
		myworker.reset(new worker(*this, 1, 2));
		myworker->start();
		construction_threaded(owg, 0, 2);
		myworker.reset();
	} else {
		construction_threaded(owg, 0, 1);
	}
	add_loading_screen("water height data computed");

	// set up curr_wtp and subdetail
	curr_wtp = 0;
#ifdef MEASURE_WAVE_HEIGHTS
	cout << "total minh " << totalmin << " maxh " << totalmax << "\n";
#endif
	compute_amount_of_foam();

	add_loading_screen("water created");
	set_time(mytime);
}



void water::construction_threaded(ocean_wave_generator<float>& myowg, unsigned phase_start, unsigned phase_add)
{
	for (unsigned i = phase_start; i < wave_phases; i += phase_add) {
		generate_wavetile(myowg, wave_tidecycle_time * i / wave_phases, wavetile_data[i]);
	}
}



void water::setup_textures(const matrix4& reflection_projmvmat, const vector2f& transl,
			   bool under_water) const
{
	if (under_water) {
		glsl_under_water->use();
	} else {
		glsl_water->use();
		glsl_water->set_gl_texture(*foamtex, loc_w_tex_foam, 2);
		glsl_water->set_gl_texture(*foamamounttex, loc_w_tex_foamamount, 3);
	}

	// texture units / coordinates:
	// tex0: noise map (color normals) / matching texcoords
	// tex1: reflection map / matching texcoords
	// tex2: foam
	// tex3: amount of foam

	// set up scale/translation of foam and noise maps
	// we do not use the texture matrix here, as this would be overkill
	// and would be clumsy
	/* how to compute that.
	   We would need to compute a closed path over a tile (modulo tile size).
	   This can be any path with curves etc., but lets take a linear movement.
	   Start and end point must be the same in the tile (modulo tilesize)
	   after a given time t.
	   To achieve this we need to know how many tiles we move horizontally and vertically
	   for the path. This are numbers A and B. We move with velocity V (meters/second),
	   thus we get as time t for the path:
	   t = tile_size * sqrt(a*a + b*b) / V
	   and for the direction vector of movement:
	   D = (V/sqrt(a*a + b*b)) * (a, b)
	   Example, if we would like to move 2 tiles right and 5 down, with 0.5m/s, we get
	   t = 256m * sqrt(2*2+5*5) / 0.5m/s = 2757.2s
	   and D = (0.186, 0.464)
	   So we have to take mytime module t and use that as multiplier with D to get
	   the current position (plus initial offset S).
	   In the shader the vertex position is multiplied with factor z here and then
	   xy are added. So z translates meters to texture coordinates.
	   For noise #0 scale is 8/256m, so we have 8 tiles of the texture per wave tile.
	   So one noise tile is 256m/8 = 1/z = 32m long. That is the tile size we have
	   to use for the computation above.
	   32m with V=2m/sec -> t = 86.162 and D = (0.743, 1.857)
	*/
	static const int a_0 = 2, b_0 = 5;
	static const int a_1 = -4, b_1 = 3;
#ifdef WIN32
	// windows lacks sqrt(int) ?!?! TODO: casting to float ok as a general rule? -- matt
	static const double s_0 = sqrt((float)(a_0*a_0 + b_0*b_0)), s_1 = sqrt((float)(a_1*a_1 + b_1*b_1));
#else
	static const double s_0 = sqrt(a_0*a_0 + b_0*b_0), s_1 = sqrt(a_1*a_1 + b_1*b_1);
#endif
	static const double V_0 = 2.0, V_1 = 1.0;
	// maybe: remove hardwired scale factors of 8 and 32, but looks best with that values.
	static const double t_0 = wavetile_length / 8.0 * s_0 / V_0, t_1 = wavetile_length / 32.0 * s_1 / V_1;
	// need to divide by noise tile size here too (tex coordinates are in [0...1], not meters)
	static const vector2f D_0 = vector2f(a_0, b_0) * (V_0/s_0/32.0); // noise tile is 256/8=32m long
	static const vector2f D_1 = vector2f(a_1, b_1) * (V_1/s_1/8.0);  // noise tile is 256/32=8m long
	vector2f noise_0_pos = vector2f(0, 0) + D_0 * myfmod(mytime, t_0);
	vector2f noise_1_pos = vector2f(0, 0) + D_1 * myfmod(mytime, t_1);
	//fixme: do we have to treat the viewer offset here, like with tex matrix
	//       setup below?!
	if (under_water) {
		glsl_under_water->set_uniform(loc_uw_noise_xform_0, noise_0_pos.xyz(wavetile_length_rcp * 8.0f));
		glsl_under_water->set_uniform(loc_uw_noise_xform_1, noise_1_pos.xyz(wavetile_length_rcp * 32.0f));
		glsl_under_water->set_gl_texture(*water_bumpmap, loc_uw_tex_normal, 0);
	} else {
		glsl_water->set_uniform(loc_w_noise_xform_0, noise_0_pos.xyz(wavetile_length_rcp * 8.0f));
		glsl_water->set_uniform(loc_w_noise_xform_1, noise_1_pos.xyz(wavetile_length_rcp * 32.0f));
		glsl_water->set_gl_texture(*water_bumpmap, loc_w_tex_normal, 0);
	}

	// set up texture matrix, so that texture coordinates can be computed from position.
	if (!under_water) {
		// fixme 32m wide noise is a good compromise, that would be a noise
		// sample every 12.5cm. But it works only if the noise map
		// has high frequencies in it... this noise map has to few details
		const float noisetilescale = 32.0f;//meters (128/16=8, 8tex/m).
		glsl_water->set_uniform(loc_w_foam_transform,
					matrix4::diagonal(1.0f/noisetilescale,1.0f/noisetilescale,1) *
					matrix4::trans(transl.x, transl.y, 0));
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

	if (!under_water) {
		glsl_water->set_gl_texture(*reflectiontex, loc_w_tex_reflection, 1);
		matrix4 refl_trans = matrix4::trans(0.5, 0.5, 0.0)
			* matrix4::diagonal(0.5, 0.5, 1.0)
			* reflection_projmvmat;
		glsl_water->set_uniform(loc_w_reflection_transform, refl_trans);
		if (use_hqsfx) {
			// here get result and give it to the fragment shader
			glsl_water->set_uniform(loc_w_reflection_mvp, refl_trans);
		}
	}
}



void water::cleanup_textures() const
{
}



void water::draw_foam_for_ship(const game& gm, const ship* shp, const vector3& viewpos) const
{
	/* fixme: for each prev pos store also heading (as direction vector)
	   then hdg.ortho = normal!
	   and we can compute the prevpos of the bow!
	*/
	vector2 spos = shp->get_pos().xy() - viewpos.xy();
	vector2 sdir = shp->get_heading().direction();
	vector2 pdir = sdir.orthogonal();
	float sl = shp->get_length();
	float sw = shp->get_width();

	// draw foam caused by hull.
	primitives::textured_quad((spos + sdir * (sl* 0.5) + pdir * (sw*-0.5)).xyz(-viewpos.z),
				  (spos + sdir * (sl* 0.5) + pdir * (sw* 0.5)).xyz(-viewpos.z),
				  (spos + sdir * (sl*-0.5) + pdir * (sw* 0.5)).xyz(-viewpos.z),
				  (spos + sdir * (sl*-0.5) + pdir * (sw*-0.5)).xyz(-viewpos.z),
				  *foamperimetertex).render();

	double tm = gm.get_time();

	// draw foam caused by trail.
	const list<ship::prev_pos>& prevposn = shp->get_previous_positions();
	// can render strip of quads only when more than one position is stored.
	if (prevposn.empty())
		return;

	primitives foamtrail(GL_QUAD_STRIP, 2 + prevposn.size()*2, *foamamounttrail);
	color col(255, 255, 255, 255);

	// first position is current position and thus special.
	vector2 foamstart = shp->get_pos().xy() + sdir * (sl*0.5);
	vector2 pl = foamstart - viewpos.xy();
	vector2 pr = foamstart - viewpos.xy();
	foamtrail.colors[0] = col;
	foamtrail.colors[1] = col;
	double yc = myfmod(tm * 0.1, 3600);
	foamtrail.texcoords[0] = vector2f(0, yc);
	foamtrail.texcoords[1] = vector2f(1, yc);
	foamtrail.vertices[0] = vector3f(pl.x, pl.y, -viewpos.z);
	foamtrail.vertices[1] = vector3f(pr.x, pr.y, -viewpos.z);

	// iterate over stored positions, compute normal for trail for each position and width
	list<ship::prev_pos>::const_iterator pit = prevposn.begin();
	//double dist = 0;
// 	cout << "new trail\n";
// 	int ctr=0;
	unsigned pitc = 2;
	while (pit != prevposn.end()) {
		vector2 p = pit->pos + (pit->dir * (sl*0.5)) - viewpos.xy();
		vector2 nrml = pit->dir.orthogonal();
		// amount of foam (density) depends on time (age). foam vanishs after 30seconds
		double age = tm - pit->time;
		double foamamount = fmax(0.0, 1.0 - age * (1.0/30));
		// width of foam trail depends on speed and time.
		// "young" foam is growing to max. width, max. width is determined by speed
		// width is speed in m/s * 2, gives ca. 34m wide foam on each side with 34kts.
		double maxwidth = pit->speed * 2.0;
		double foamwidth = (1.0 - 1.0/(age * 0.25 + 1.0)) * maxwidth;
// 		cout << "[" << ctr++ << "] age=" << age << " amt=" << foamamount << " maxw=" << maxwidth
// 		     << " fw=" << foamwidth << "\n";
		++pit;	// now pit points to next point
		if (pit == prevposn.end()) {
			// amount is always zero on last point, to blend smoothly
			foamamount = 0;
		}
		// move p to viewer space
		vector2 pl = p - nrml * foamwidth;
		vector2 pr = p + nrml * foamwidth;
		col.a = Uint8(foamamount*255);
		foamtrail.colors[pitc] = col;
		foamtrail.colors[pitc+1] = col;
		//y-coord depends on total length somehow, rather on distance between two points.
		//but it should be fix for any position on the foam, or the edge of the foam
		//will "jump", an ugly effect - we use a workaround here to use time as fake
		//distance and take mod 3600 to keep fix y coords.
		double yc = myfmod((tm - age) * 0.1, 3600);
		foamtrail.texcoords[pitc] = vector2f(0, yc);
		foamtrail.texcoords[pitc+1] = vector2f(1, yc);
		foamtrail.vertices[pitc] = vector3f(pl.x, pl.y, -viewpos.z);
		foamtrail.vertices[pitc+1] = vector3f(pr.x, pr.y, -viewpos.z);
		pitc += 2;
	}
	foamtrail.render();
}



//static unsigned nrfm=0;
void water::compute_amount_of_foam_texture(const game& gm, const vector3& viewpos,
					   const vector<ship*>& allships) const
{
//	glPushMatrix();

	unsigned afs = foamamounttex->get_gl_width();	// size of amount of foam size, depends on water grid detail, fixme
	// foam is drawn the the same matrices as water, but with a smaller viewport
	if (foamamounttex_fbo.get()) {
		foamamounttex_fbo->bind();
	} else {
		// set up viewport
		glViewport(0, 0, afs, afs);
	}
	// clear it , fixme: must be done earlier, there is already drawn something inside the viewport
	glClearColor(0, 0, 0, 0);
	//fixme: clears whole viewport, which kills display with torpedo camera view
	//when not using FBOs...
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);

	// draw trails / wave tops

	//wave tops: draw one quad matching the water surface, texture mapped with a "amount-of-foam-of-wave-tile" texture
	// generated from wave heights (could be constant in time, just moving around).
	// indicates foam at high waves.
//	glDisable(GL_CULL_FACE);
	// as first trails of all ships
	// fixme: texture mapping seems to be wrong.
	for (vector<ship*>::const_iterator it = allships.begin(); it != allships.end(); ++it) {
		draw_foam_for_ship(gm, *it, viewpos);
	}
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
        osgname << "foamamount" << setw(2) << setfill('0') << nrfm++ << ".ppm";
        ofstream osg(osgname.str().c_str());
        osg << "P6\n"<<afs<<" "<<afs<<"\n255\n";
        osg.write((const char*)(&data[0]), afs*afs*3);
#endif

	if (foamamounttex_fbo.get()) {
		foamamounttex_fbo->unbind();
	} else {
		// copy viewport data to foam-amount texture
		foamamounttex->set_gl_texture();
		glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, afs, afs, 0);
		glViewport(0, 0, sys().get_res_x(), sys().get_res_y());
	}

	// clean up
	glEnable(GL_DEPTH_TEST);

//	glPopMatrix();
}



static inline double round_(double x)
{
	// note: just using
	// return floor(x + 0.5);
	// seems to work too, makes more sense with rounding. why this strange formula?!
	if (x < 0) return -floor(-x + 0.5);
	else return floor(x + 0.5);
}

void water::display(const vector3& viewpos, double max_view_dist, bool under_water) const
{
	// get projection and modelview matrix
	matrix4 proj = matrix4::get_gl(GL_PROJECTION_MATRIX);
	matrix4 modl = matrix4::get_gl(GL_MODELVIEW_MATRIX);
	matrix4 reflection_projmvmat = proj * modl;
	vector2f transl(myfmod(viewpos.x, double(wavetile_length)),
			myfmod(viewpos.y, double(wavetile_length)));

	// give -viewpos.z to vertex shader for generation of foam projection coordinates
	// the plane z = -viewpos.z is the water plane.
	if (under_water) {
		glsl_under_water->use();
		glsl_under_water->set_uniform(loc_uw_viewpos, viewpos);
	} else {
		glsl_water->use();
		glsl_water->set_uniform(loc_w_viewpos, viewpos);
	}
	setup_textures(reflection_projmvmat, transl, under_water);

	bool recompute_vertices = rerender_new_wtp
		|| (rerender_viewpos.square_distance(viewpos) > 0.0001);
	rerender_new_wtp = false;
	rerender_viewpos = viewpos;
	
	//fixme: in setup_textures wird texmatrix von modelviewmatrix gesetzt, die veraendern wir aber
	//hier nochmal, das passt dann nicht mehr...   wirklich? pruefe das!!!

	// render levels from nearest to farest.
	// we number the base level 0, nearer levels with additional subdetail have
	// numbers below zero, coarser levels numbers greater than zero.
	// Resolution of the data halves with each additional level.
	// So maximum level is log2(wave_resolution)-1, so the coarsest level
	// has a heightmap resolution of 2*2.
	// Level 0 has a heightmap resolution of wave_resolution*wave_resolution.
	// Finer levels have additional detail data, which is computed on-the-fly.
	// This should be a fractal function, possibly recursive.
	// It could be perlin noise or the wave height data itself, applied
	// in a scaled version. This gives many finer levels.
	// For a wave_resolution of 128 with wavetile_length of 256m we have
	// levels 0...6 with height data resolutions of 128,64,32,16,8,4,2.
	// Finer levels -1...x are possible, depending on the scale factor of the
	// fractal noise. Note that level 0 is NOT rendered with 128x128 quads, but only
	// a inner subsection with n*n quads (n is a power of two, e.g. 32).
	// A reasonable detail for the finest level would be 0.25m, giving an 8m tile length
	// for the finest level data.

	// compute which level is nearest. The higher the viewer is, the less
	// detail we need to render and thus the coarser the first level is.

	// Which level number is the farest depends on viewing range. Normally we render all
	// levels up to the coarsest, but with fog or other limited visibility we can spare
	// coarser levels here.

	// define some common values.
	double L = wavetile_length / wave_resolution;
	//double L_rcp = wave_resolution / wavetile_length;
	const unsigned N = geoclipmap_resolution;

	// compute module of wave tile length to get values in usable range,
	// to give same precision of values, no matter how big the value of viewpos.x/y is,
	// this can be very high, as its globally in meters.
	vector2 viewpos_mod(myfmod(viewpos.x, double(wavetile_length)), myfmod(viewpos.y, double(wavetile_length)));

	// Note! on fast movement one can see some popping of detail, when boundary between
	// levels moves around the screen. This can be avoided by smoothly blending
	// over height between data of two levels for vertices near the border.
	// This increases either CPU time (when done at the vertex computation) or GPU time
	// (when done in the vertex shader). It makes much more sense to compute that
	// on the CPU, because we would need to transfer (x,y,z) data for each vertex
	// not only for the current but also the _next_ level to the GPU.
	// The effect can hardly be seen at normal gameplay, so we ignore it for now.

	// compute vertices for all levels, if needed
	// this optimization brings 3% more frame rates on a GF7600GT,
	// with only a few lines of code
	const unsigned nr_vert_attr = 7;
	if (recompute_vertices) {
		unsigned nr_verts_total = geoclipmap_levels * (N+1)*(N+1) + 8 /* horizon */;
		// which data do we need per vertex?
		// without shaders:
		// - position (3f)
		// - texcoord0 (2f)
		// - texcoord1 (3f)
		// --- sum: 8f
		// with shaders (now)
		// - position (3f)
		// - normal (3f)
		// - amount of foam (1f)
		// --- sum: 7f
		// with shaders (later)
		// - position (3f)
		// - texcoord (2f) (base position x,y - no displacement)
		// - amount of foam (1f)
		// --- sum: 6f
		// No! we still need the normal in the vertex shader, to compute the slope dependent
		// color and, more important, the reflection texture coordinate distorsion...
		// so we have 9 floats here, but still not much data...
		// Some papers state that 32bytes per vertex are ideal (ATI cards), so 8f ideal here...
		// normal can be fetched as 2f instead of 3f and z recomputed by renormalization
		// in v-shader...
		vertices.init_data(nr_verts_total * nr_vert_attr * 4, 0, GL_STREAM_DRAW);
		float* vertex_data = (float*) vertices.map(GL_WRITE_ONLY);
		//printf("nr_verts_total %u, memory %u\n", nr_verts_total, nr_verts_total*nr_vert_attr*4);
		//with N=64 we have 21125 vertices, eating 507000 bytes of memory (shader).
		unsigned vertex_data_ptr = 0;
		// whole block takes 0.85ms, so way fast enough (shader).
		// maybe: test if writing to mapped memory is faster than storing and copying data (DMA).
		// but this is way fast enough already!
		for (unsigned level = 0 /*minlevel*/; level < geoclipmap_levels; ++level) {
			// scalar depending on level
			double level_fac = double(1 << level);
			// length between samples in meters, depends on level.
			double L_l = L * level_fac;
			int xoff = int(round_(0.5*viewpos_mod.x/L_l - 0.25*N)*2);
			int yoff = int(round_(0.5*viewpos_mod.y/L_l - 0.25*N)*2);
			float coordxadd = L_l, coordyadd = L_l;
			vector2f offset(xoff * L_l - viewpos_mod.x, yoff * L_l - viewpos_mod.y);
			// make sure offset is in wave tile, so add large modulo value
			xoff += 16*wave_resolution;
			yoff += 16*wave_resolution;

			unsigned yy = yoff;
			float coordyoff = offset.y;
			const wavetile_phase::mipmap_level& mml = curr_wtp->mipmaps[level];
			unsigned mod = (wave_resolution >> level) - 1;
			unsigned mult = (wave_resolution >> level);
			for (unsigned y = 0; y < N+1; ++y) {
				unsigned lineidx = (yy & mod) * mult;
				unsigned xx = xoff;
				float coordxoff = offset.x;
				for (unsigned x = 0; x < N+1; ++x) {
					// fixme: adding values of two tiles here could be worth the
					// trouble, use same data twice with different scales,
					// thus increasing geometrical detail.
					// like a 512m tile length with smallest waves of 0.25m
					// (4m-512m detail on coarse level, 0.25m-32m on fine level)
					// etc., uses view cpu time, but can increase detail
					// dramatically... already tried that to add subdetail
					// for nearer levels (<0) on earlier water renderings.
					// would increase number of rendered levels by 3 or 4, but still
					// ok for memory consumption...
					unsigned colidx = (xx & mod);
					const vector3f& p0 = mml.wavedata[lineidx + colidx];
					const vector3f& n0 = mml.normals[lineidx + colidx];
					vertex_data[vertex_data_ptr+0] = p0.x + coordxoff;
					vertex_data[vertex_data_ptr+1] = p0.y + coordyoff;
					vertex_data[vertex_data_ptr+2] = p0.z - viewpos.z;
					//vertex_data[vertex_data_ptr+3] = coordxoff;
					//vertex_data[vertex_data_ptr+4] = coordyoff;
					//vertex_data[vertex_data_ptr+5] = foamamount;
					vertex_data[vertex_data_ptr+3] = n0.x;
					vertex_data[vertex_data_ptr+4] = n0.y;
					vertex_data[vertex_data_ptr+5] = n0.z;
					vertex_data[vertex_data_ptr+6] = mml.amount_of_foam[lineidx + colidx];
					vertex_data_ptr += nr_vert_attr;
					++xx;
					coordxoff += coordxadd;
				}
				++yy;
				coordyoff += coordyadd;
			}
		}
		// force vertex height on border to horizon faces to zero, normal to (0,0,1).
		vertex_data_ptr = (geoclipmap_levels - 1) * (N+1)*(N+1) * nr_vert_attr;
		for (unsigned i = 0; i < N+1; ++i) {
			unsigned k[4] = { vertex_data_ptr + nr_vert_attr*i,
					  vertex_data_ptr + nr_vert_attr*(i + N*(N+1)),
					  vertex_data_ptr + nr_vert_attr*(i*(N+1)),
					  vertex_data_ptr + nr_vert_attr*(i*(N+1) + N) };
			for (unsigned j = 0; j < 4; ++j) {
				vertex_data[k[j] + 2] = -viewpos.z;
				vertex_data[k[j] + 3] = 0.0f;
				vertex_data[k[j] + 4] = 0.0f;
				vertex_data[k[j] + 5] = 1.0f;
				vertex_data[k[j] + 6] = 0.0f; // no foam
			}
		}
		// add horizon faces
		int hzx[8] = { -1, 0, 1, -1, 1, -1, 0, 1 };
		int hzy[8] = { -1,-1,-1,  0, 0,  1, 1, 1 };
		vertex_data_ptr = geoclipmap_levels * (N+1)*(N+1) * nr_vert_attr;
		for (unsigned j = 0; j < 8; ++j) {
			vertex_data[vertex_data_ptr + 0] = hzx[j] * max_view_dist;
			vertex_data[vertex_data_ptr + 1] = hzy[j] * max_view_dist;
			vertex_data[vertex_data_ptr + 2] = -viewpos.z;
			vertex_data[vertex_data_ptr + 3] = 0.0f;
			vertex_data[vertex_data_ptr + 4] = 0.0f;
			vertex_data[vertex_data_ptr + 5] = 1.0f;
			vertex_data[vertex_data_ptr + 6] = 0.0f; // no foam
			vertex_data_ptr += nr_vert_attr;
		}
		// finish
		vertices.unmap();
		vertices.unbind();
	}

	// ------------------- rendering --------------------------

	// compute viewing frustum for culling
	frustum viewfrustum = frustum::from_opengl();

	// as first, map buffers correctly.
	vertices.bind();
	glVertexPointer(3, GL_FLOAT, nr_vert_attr*4, (float*)0 + 0);
	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, nr_vert_attr*4, (float*)0 + 3);
	glVertexAttribPointer(vattr_aof_index, 1, GL_FLOAT, GL_FALSE, nr_vert_attr*4, (float*)0 + 6);
	glEnableVertexAttribArray(vattr_aof_index);
	vertices.unbind();

	// innermost level is rendered always
	patches[0]->render();
	// render outer levels with view frustum culling
	for (unsigned level = 1; level < geoclipmap_levels; ++level) {
		// scalar depending on level
		double level_fac = double(1 << level);
		// length between samples in meters, depends on level.
		double L_l = L * level_fac;
		// x_base/y_base tells offset in sample data according to level and
		// viewer position (viewpos_mod)
		// this multiply with 0.5 then round then *2 lets the patches map to
		//"even" vertices and must be used to determine which patch number to render.
		int x_base[4] = { int(round_(0.5*viewpos_mod.x/L_l - 0.25*N)*2),
				  int(round_(    viewpos_mod.x/L_l - 0.25*N)  ),
				  int(round_(    viewpos_mod.x/L_l + 0.25*N)  ),
				  int(round_(0.5*viewpos_mod.x/L_l + 0.25*N)*2) };
		int y_base[4] = { int(round_(0.5*viewpos_mod.y/L_l - 0.25*N)*2),
				  int(round_(    viewpos_mod.y/L_l - 0.25*N)  ),
				  int(round_(    viewpos_mod.y/L_l + 0.25*N)  ),
				  int(round_(0.5*viewpos_mod.y/L_l + 0.25*N)*2) };
		// compute sizes and according to that patch indices
		unsigned x_size[3], y_size[3];
		for (unsigned j = 0; j < 3; ++j) {
			x_size[j] = unsigned(x_base[j+1] - x_base[j]);
			y_size[j] = unsigned(y_base[j+1] - y_base[j]);
		}
		unsigned x_patchidx = 1, y_patchidx = 1;
		if (x_size[0] == N/4-1)
			x_patchidx = 0;
		else if (x_size[0] == N/4+1)
			x_patchidx = 2;
		if (y_size[0] == N/4-1)
			y_patchidx = 0;
		else if (y_size[0] == N/4+1)
			y_patchidx = 2;
		//printf("x/y patch idx=%u %u\n", x_patchidx, y_patchidx);
		unsigned patchidx = y_patchidx*3 + x_patchidx;
		
		// render outer levels with inner hole, render 8 parts
		const unsigned kk[8] = { 0, 0, 0, 1, 1, 2, 2, 2 };
		const unsigned jj[8] = { 0, 1, 2, 0, 2, 0, 1, 2 };
		for (unsigned i = 0; i < 8; ++i) {
			const unsigned k = kk[i];
			const unsigned j = jj[i];
			const vector2 offset = vector2(x_base[j], y_base[k]) * L_l - viewpos_mod;
			//printf("offset ist %f,%f i=%u\n",offset.x,offset.y,i);
			//printf("x/y size %u %u\n",x_size[j],y_size[k]);
			// fixme: enlarge poly by max. x/y displacement of wave coordinates,
			// or we see patches disappear sometimes near the screen border...
			polygon patchpoly(vector3(offset.x, offset.y, -viewpos.z),
					  vector3(offset.x + x_size[j]*L_l, offset.y, -viewpos.z),
					  vector3(offset.x + x_size[j]*L_l, offset.y + y_size[k]*L_l, -viewpos.z),
					  vector3(offset.x, offset.y + y_size[k]*L_l, -viewpos.z));
			if (!viewfrustum.clip(patchpoly).empty()) {
				// render patch
				patches[1 + (level-1)*9*8 + patchidx*8 + i]->render();
			} else {
				//printf("culled away patch, level=%i xyoff=%f,%f\n",level,offset.x,offset.y);
			}
		}
	}
	// render horizon polys
	for (unsigned k = 0; k < 4; ++k) {
		// fixme: view frustum clipping, but gives only small performance gain
		patches[patches.size() - 4 + k]->render();
	}

	// unmap, cleanup
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableVertexAttribArray(vattr_aof_index);

	// clean up textures
	cleanup_textures();
}



float water::get_height(const vector2& pos) const
{
	float ffac = wave_resolution * wavetile_length_rcp;
	float x = float(myfmod(pos.x, double(wavetile_length))) * ffac;
	float y = float(myfmod(pos.y, double(wavetile_length))) * ffac;
	int ix = int(floor(x));
	int iy = int(floor(y));
	int ix2 = (ix+1) & (wave_resolution-1);
	int iy2 = (iy+1) & (wave_resolution-1);
	float fracx = x - ix;
	float fracy = y - iy;
	ix &= (wave_resolution-1); // avoid rare cases of wrap-around, when x is == wave_resolution because of float round errors
	iy &= (wave_resolution-1);
	float a = curr_wtp->get_height(ix +(iy <<wave_resolution_shift));
	float b = curr_wtp->get_height(ix2+(iy <<wave_resolution_shift));
	float c = curr_wtp->get_height(ix +(iy2<<wave_resolution_shift));
	float d = curr_wtp->get_height(ix2+(iy2<<wave_resolution_shift));
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
	float hdx = curr_wtp->get_height(x2+ (y <<wave_resolution_shift))
		-   curr_wtp->get_height(x1+ (y <<wave_resolution_shift));
	float hdy = curr_wtp->get_height(x + (y2<<wave_resolution_shift))
		-   curr_wtp->get_height(x + (y1<<wave_resolution_shift));
	return vector3f(-hdx, -hdy, 1).normal();
}


//fixme: the correctness of the result of this function and the one above is not fully tested.
//with a realistic buoyancy model we don't need that function any longer!
vector3f water::get_normal(const vector2& pos, double rollfac) const
{
	float ffac = wave_resolution * wavetile_length_rcp;
	float x = float(myfmod(pos.x, double(wavetile_length))) * ffac;
	float y = float(myfmod(pos.y, double(wavetile_length))) * ffac;
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



void water::generate_wavetile(ocean_wave_generator<float>& myowg, double tiletime, wavetile_phase& wtp)
{
	vector<float> heights;
	myowg.set_time(myfmod(tiletime, wave_tidecycle_time));
	myowg.compute_heights(heights);
	wtp.minh = 1e10;
	wtp.maxh = -1e10;
	for (vector<float>::const_iterator it = heights.begin(); it != heights.end(); ++it) {
		wtp.minh = fmin(wtp.minh, *it);
		wtp.maxh = fmax(wtp.maxh, *it);
	}
#ifdef MEASURE_WAVE_HEIGHTS
	totalmin = std::min(wtp.minh, totalmin);
	totalmax = std::max(wtp.maxh, totalmax);
#endif
#if 0
	char fn[32]; sprintf(fn, "waveh%f.pgm", tiletime);
	std::ofstream osg(fn);
	osg << "P5\n";
	osg <<wave_resolution<<" "<<wave_resolution<<"\n255\n";
	for (vector<float>::const_iterator it = heights.begin(); it != heights.end(); ++it) {
		Uint8 h = Uint8((*it - wtp.minh)*255.9/(wtp.maxh - wtp.minh));
		osg.write((const char*)&h, 1);
	}
#endif
	//float maxabsh = std::max(fabs(wtp.minh), fabs(wtp.maxh));

	//unsigned hs = heights.size();

//	cout << "absolute height +- of tile " << max(fabs(wtp.minh), fabs(wtp.maxh)) << "\n";

	// choppy factor: formula from "waterengine": 0.5*WX/N = 0.5*wavelength/waveres, here = 1.0
	// fixme 5.0 default? - it seems that choppy waves don't look right. bug? fixme, with negative values it seems right. check this!
	// -2.0f also looks nice, -5.0f is too much. -1.0f should be ok
	vector<vector2f> displacements;
	myowg.compute_displacements(-2.0f, displacements);

#if 0
	// compute where foam is generated...
	// max. length of displacement is no good criteria, also not min. length.
	// we need to know where waves break, that would be where displacement
	// has its local extrema (maxima). but this is two dimensional then...
	// gives tessendorf some hints here? jacobian matrices?
	// yes, tessendorf 2004 p.12f. when the jacobian becomes negative,
	// we have overlap.
	// we have the displacment values in x and y direction and neee to
	// compute derivatives in x/y dir, giving 4 values that need to be
	// computed with lambda to get the jacobian value.
	char fn2[32]; sprintf(fn2, "waveh%fdisp.pgm", tiletime);
	std::ofstream osg2(fn2);
	osg2 << "P5\n";
	osg2 <<wave_resolution<<" "<<wave_resolution<<"\n255\n";
	double lambda = 0.5;//should be 1.0, as lambda has already been multiplied (2.0) above
	for (unsigned y = 0; y < wave_resolution; ++y) {
		unsigned ym1 = (y + wave_resolution - 1) & (wave_resolution-1);
		unsigned yp1 = (y + 1) & (wave_resolution-1);
		for (unsigned x = 0; x < wave_resolution; ++x) {
			unsigned xm1 = (x + wave_resolution - 1) & (wave_resolution-1);
			unsigned xp1 = (x + 1) & (wave_resolution-1);
			//fixme: this is not the real derivative, as distance between
			// samples is not taken into account. we need to multiply the disp
			// values with 0.5, but we do this by faking lambda to 0.5
			double dispx_dx = displacements[y*wave_resolution+xp1].x - displacements[y*wave_resolution+xm1].x;
			double dispx_dy = displacements[yp1*wave_resolution+x].x - displacements[ym1*wave_resolution+x].x;
			double dispy_dx = displacements[y*wave_resolution+xp1].y - displacements[y*wave_resolution+xm1].y;
			double dispy_dy = displacements[yp1*wave_resolution+x].y - displacements[ym1*wave_resolution+x].y;
			double Jxx = 1.0 + lambda * dispx_dx;
			double Jyy = 1.0 + lambda * dispy_dy;
			double Jxy = lambda * dispy_dx;
			double Jyx = lambda * dispx_dy;
			double J = Jxx*Jyy - Jxy*Jyx;
			//printf("x,y=%u,%u, Jxx,yy=%f,%f Jxy,yx=%f,%f J=%f\n",
			//       x,y, Jxx,Jyy, Jxy,Jyx, J);
			double fa = (J - 0.3) / -1.0 * 255;
			if (fa < 0) fa = 0; else if (fa > 255) fa = 255;
			Uint8 x = Uint8(fa);
			osg2.write((const char*)&x, 1);
		}
	}
	/* fixme. what do we need to do:
	   compute where foam should be generated (or even how much), by the formulas
	   seen above per water sample. (If J < 0.3, generate foam in growing amount)
	   Foam decays over time. Full foam must have disappeared within one period
	   of animation, so we can use the cyclic animation.
	   Start with clean field of amount of foam values (all zero),
	   then compute how much foam to generate per sample for all animation phases
	   and simulatenously compute decay.
	   Do this for *2* animation cycles, because of wrap around.
	   After that take the data of the last cycle and store per vertex and time step
	   how much foam there is for the realtime display later.
	*/
#endif

	unsigned mipmap_levels = wave_resolution_shift;
	wtp.mipmaps.reserve(mipmap_levels);
	double L = wavetile_length / wave_resolution;
	wtp.mipmaps.push_back(wavetile_phase::mipmap_level(displacements, heights,
							   wave_resolution_shift, L));
	for (unsigned i = 1; i < mipmap_levels; ++i) {
		wtp.mipmaps.push_back(wavetile_phase::mipmap_level(wtp.mipmaps.back().wavedata,
								   wave_resolution_shift - i,
								   L * (1 << i)));
	}

/*
	float maxdispl = 0.0;
	for (vector<vector2f>::const_iterator it = displacements.begin(); it != displacements.end(); ++it) {
		maxdispl = max(maxdispl, fabs(it->x));
		maxdispl = max(maxdispl, fabs(it->y));
	}
	cout << "abs max displ " << maxdispl << "\n";
*/
}



void water::compute_amount_of_foam()
{
	// compute amount of foam per vertex sample
	vector<float> aof(wave_resolution*wave_resolution);

	float rndtab[37];
	for (unsigned k = 0; k < 37; ++k)
		rndtab[k] = rnd();

	// factor to build derivatives correctly
	const double deriv_fac = wavetile_length_rcp * wave_resolution;
	const double lambda = 1.0; // lambda has already been multiplied with x/y displacements...
	const double decay = 4.0/wave_phases;
	const double decay_rnd = 0.25/wave_phases;
	const double foam_spawn_fac = 0.25;//0.125;
	for (unsigned k = 0; k < wave_phases * 2; ++k) {
		const vector<vector3f>& wd = wavetile_data[k % wave_phases].mipmaps[0].wavedata;
		// compute for each sample how much foam is added (spawned)
		for (unsigned y = 0; y < wave_resolution; ++y) {
			unsigned ym1 = (y + wave_resolution - 1) & (wave_resolution-1);
			unsigned yp1 = (y + 1) & (wave_resolution-1);
			for (unsigned x = 0; x < wave_resolution; ++x) {
				unsigned xm1 = (x + wave_resolution - 1) & (wave_resolution-1);
				unsigned xp1 = (x + 1) & (wave_resolution-1);
				double dispx_dx = (wd[y*wave_resolution+xp1].x - wd[y*wave_resolution+xm1].x) * deriv_fac;
				double dispx_dy = (wd[yp1*wave_resolution+x].x - wd[ym1*wave_resolution+x].x) * deriv_fac;
				double dispy_dx = (wd[y*wave_resolution+xp1].y - wd[y*wave_resolution+xm1].y) * deriv_fac;
				double dispy_dy = (wd[yp1*wave_resolution+x].y - wd[ym1*wave_resolution+x].y) * deriv_fac;
				double Jxx = 1.0 + lambda * dispx_dx;
				double Jyy = 1.0 + lambda * dispy_dy;
				double Jxy = lambda * dispy_dx;
				double Jyx = lambda * dispx_dy;
				double J = Jxx*Jyy - Jxy*Jyx;
				//printf("x,y=%u,%u, Jxx,yy=%f,%f Jxy,yx=%f,%f J=%f\n",
				//       x,y, Jxx,Jyy, Jxy,Jyx, J);
				//double foam_add = (J < 0.3) ? ((J < -1.0) ? 1.0 : (J - 0.3)/-1.3) : 0.0;
				double foam_add = (J < 0.0) ? ((J < -1.0) ? 1.0 : -J) : 0.0;
				aof[y*wave_resolution+x] += foam_add * foam_spawn_fac;
				// spawn foam also on neighbouring fields
				aof[ym1*wave_resolution+x] += foam_add * foam_spawn_fac * 0.5;
				aof[yp1*wave_resolution+x] += foam_add * foam_spawn_fac * 0.5;
				aof[y*wave_resolution+xm1] += foam_add * foam_spawn_fac * 0.5;
				aof[y*wave_resolution+xp1] += foam_add * foam_spawn_fac * 0.5;
			}
		}

		// compute decay, depends on time with some randomness
		unsigned ptr = 0;
		for (unsigned y = 0; y < wave_resolution; ++y) {
			for (unsigned x = 0; x < wave_resolution; ++x) {
				aof[ptr] = std::max(std::min(aof[ptr], 1.0f) - (decay + decay_rnd * rndtab[(3*x + 5*y) % 37]), 0.0);
				++ptr;
			}
		}

		// store amount of foam data when in second iteration
		if (k >= wave_phases) {
			wavetile_phase::mipmap_level& mm0 = wavetile_data[k - wave_phases].mipmaps[0];
			mm0.amount_of_foam = aof;
			for (unsigned j = 1; j < wavetile_data[k - wave_phases].mipmaps.size(); ++j) {
				unsigned res = wave_resolution >> j;
				const wavetile_phase::mipmap_level& mm1 = wavetile_data[k - wave_phases].mipmaps[j-1];
				wavetile_phase::mipmap_level& mm2 = wavetile_data[k - wave_phases].mipmaps[j];
				mm2.amount_of_foam.reserve(res*res);
				unsigned ptr = 0;
				for (unsigned y = 0; y < res; ++y) {
					for (unsigned x = 0; x < res; ++x) {
						float sum = mm1.amount_of_foam[ptr] + mm1.amount_of_foam[ptr+1]
							+ mm1.amount_of_foam[ptr+2*res] + mm1.amount_of_foam[ptr+1+2*res];
						// fixme: maybe let foam vanish on upper mipmap levels
						mm2.amount_of_foam.push_back(sum * 0.25f);
						ptr += 2;
					}
					ptr += 2*res;
				}
			}
		}
				
#if 0
		// test: write amount of foam as grey value image
		ostringstream osfn;
		osfn << "aof" << setw(4) << setfill('0') << k << ".pgm";
		ofstream ofs(osfn.str().c_str());
		ofs << "P5\n";
		ofs <<wave_resolution<<" "<<wave_resolution<<"\n255\n";
		ptr = 0;
		for (unsigned y = 0; y < wave_resolution; ++y) {
			for (unsigned x = 0; x < wave_resolution; ++x) {
				Uint8 x = Uint8(std::min(std::max(aof[ptr++], 0.0f), 1.0f) * 255.9);
				ofs.write((const char*)&x, 1);
			}
		}
#endif
	}
}



void water::generate_subdetail_texture()
{
	// update texture with glTexSubImage2D, that is faster than to re-create the texture
	if (water_bumpmap.get()) {
		// fixme: mipmap levels > 0 are not updated...
		// can we use automatic creation of mipmaps here?
		water_bumpmap->sub_image(0, 0, wave_resolution, wave_resolution,
					 curr_wtp->mipmaps[0].normals_tex, GL_RGB);
	} else {
		//fixme: mipmap levels of normal map should be computed
		//by this class, not glu!
		//this could explain some artifacts in the distance, at least
		//when using relief mapping - heights are not downsampled...?!
		water_bumpmap.reset(new texture(curr_wtp->mipmaps[0].normals_tex,
						wave_resolution, wave_resolution,
						GL_RGB, texture::LINEAR_MIPMAP_LINEAR,
						texture::REPEAT));
	}
}



void water::set_time(double tm)
{
	// do all the tasks here that should happen regularly, like recomputing new water or noisemaps
	mytime = tm;

	unsigned pn = unsigned(wave_phases * myfrac(tm / wave_tidecycle_time)) % wave_phases;
	if (curr_wtp == &wavetile_data[pn]) {
		rerender_new_wtp = false;
	} else {
		curr_wtp = &wavetile_data[pn];
		rerender_new_wtp = true;
		generate_subdetail_texture();
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



void water::set_refraction_color(const colorf& light_color)
{
	// compute wether a visible change has happened
	if (fabs(light_color.brightness() - last_light_color.brightness()) * 128 < 1.0f)
		return;
	last_light_color = light_color;

	// fixme: color depends also on weather. bad weather -> light is less bright
	// so this will be computed implicitly. Environment can also change water color
	// (light blue in tropic waters), water depth also important etc.
	// this depends on sky color...
	// good weather
//	color wavetop = color(color(10, 10, 10), color(18, 93, 77), light_brightness);
//	color wavebottom = color(color(10, 10, 20), color(18, 73, 107), light_brightness);
	// rather bad weather
	//fixme: multiply with light color here, not only light brightness.
	//dim yellow light should result in dim yellow-green upwelling color, not dim green.
	colorf wavetop = light_color.lerp(colorf(color(0, 0, 0)), colorf(color(49, 83, 94)));
	colorf wavebottom = light_color.lerp(colorf(color(0, 0, 0)), colorf(color(29, 56, 91)));

	float wt[3], wb[3];
	wavetop.store_rgb(wt);
	wavebottom.store_rgb(wb);
	vector3f upwelltop(wt[0], wt[1], wt[2]);
	vector3f upwellbot(wb[0], wb[1], wb[2]);
	vector3f upwelltopbot = upwelltop - upwellbot;
	glsl_water->use();
	glsl_water->set_uniform(loc_w_upwelltop, upwelltop);
	glsl_water->set_uniform(loc_w_upwellbot, upwellbot);
	glsl_water->set_uniform(loc_w_upwelltopbot, upwelltopbot);
	glsl_under_water->use();
	glsl_under_water->set_uniform(loc_uw_upwelltop, upwelltop);
	glsl_under_water->set_uniform(loc_uw_upwellbot, upwellbot);
	glsl_under_water->set_uniform(loc_uw_upwelltopbot, upwelltopbot);

	for (unsigned s = 0; s < REFRAC_COLOR_RES; ++s) {
		float fs = float(s)/(REFRAC_COLOR_RES-1);
		colorf c(wavebottom, wavetop, fs);
		for (unsigned f = 0; f < FRESNEL_FCT_RES; ++f) {
			c.store_rgb(&fresnelcolortexd[(s*FRESNEL_FCT_RES+f)*4+0]);
			// update color only, leave fresnel term (alpha) intact
		}
	}
	fresnelcolortex.reset(new texture(fresnelcolortexd, FRESNEL_FCT_RES, REFRAC_COLOR_RES, GL_RGBA,
					  texture::LINEAR/*_MIPMAP_LINEAR*/, texture::CLAMP));
}



void water::refltex_render_bind() const
{
	if (reflectiontex_fbo.get()) {
		reflectiontex_fbo->bind();
	} else {
		glViewport(0, 0, reflectiontex->get_width(), reflectiontex->get_height());
	}

	// clear depth buffer (not needed for sky drawing, but this color can be seen as mirror
	// image when looking from below the water surface (scope) fixme: clear color with upwelling color!)
	glClearColor(0, 1.0f/16, 1.0f/8, 0);
	//fixme: clears whole viewport, which kills display with torpedo camera view
	//when not using FBOs...
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}



void water::refltex_render_unbind() const
{
	if (reflectiontex_fbo.get()) {
		reflectiontex_fbo->unbind();
	} else {
		reflectiontex->set_gl_texture();
		glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0,
				 reflectiontex->get_width(), reflectiontex->get_height(), 0);
		// ^ glCopyTexSubImage may be faster! but we use FBOs anyway on modern cards...
	}
}



water::wavetile_phase::mipmap_level::mipmap_level(const std::vector<vector3f>& wd,
						  unsigned res_shift, double sampledist_)
	: resolution(1 << res_shift),
	  resolution_shift(res_shift),
	  sampledist(sampledist_)
{
	wavedata.reserve(1 << (2*resolution_shift));
	unsigned ptr = 0;
	for (unsigned y = 0; y < resolution; ++y) {
		for (unsigned x = 0; x < resolution; ++x) {
			vector3f sum = wd[ptr] + wd[ptr+1] + wd[ptr+2*resolution] + wd[ptr+1+2*resolution];
			wavedata.push_back(sum * 0.25f);
			ptr += 2;
		}
		ptr += 2*resolution;
	}
	compute_normals();
	debug_dump();
}



water::wavetile_phase::mipmap_level::mipmap_level(const std::vector<vector2f>& displacements,
						  const std::vector<float>& heights,
						  unsigned res_shift,
						  double sampledist_)
	: resolution(1 << res_shift),
	  resolution_shift(res_shift),
	  sampledist(sampledist_),
	  wavedata(1 << (2*res_shift))
{
	for (unsigned i = 0; i < wavedata.size(); ++i) {
		wavedata[i].x = displacements[i].x;
		wavedata[i].y = displacements[i].y;
		wavedata[i].z = heights[i];
	}
	compute_normals();
	debug_dump();
}



void water::wavetile_phase::mipmap_level::compute_normals()
{
	// Compute normals matching the tesselation!
	// if we render TRIANGLE_STRIPs, we have a determined tesselation and thus can
	// compute correct normals.
	// compute normals for each face (triangles!) and then per vertex.
	// Triangle strip scheme:
	// v0 --- v2 --- v4
	// |    / |    / |
	// |   /  |   /  |
	// |  /   |  /   |
	// v1 --- v3 --- v5
	// v1,v3,v5 are from lower line (smaller y).
	// Each vertex has six adjacent triangles.
	vector<vector3f> facenormals;
	facenormals.reserve(resolution * resolution * 2);
	for (unsigned y = 0; y < resolution; ++y) {
		unsigned y1 = y * resolution;
		unsigned y2 = ((y + 1) & (resolution - 1)) * resolution;
		for (unsigned x = 0; x < resolution; ++x) {
			unsigned x2 = (x + 1) & (resolution - 1);
			const vector3f& p0 = wavedata[y1 + x];
			const vector3f& p1 = wavedata[y1 + x2];
			const vector3f& p2 = wavedata[y2 + x];
			const vector3f& p3 = wavedata[y2 + x2];
			vector3f p01 = p1 - p0;
			vector3f p02 = p2 - p0;
			vector3f p03 = p3 - p0;
			p01.x += sampledist;
			p03.x += sampledist;
			p02.y += sampledist;
			p03.y += sampledist;
			facenormals.push_back(p03.cross(p02).normal());
			facenormals.push_back(p01.cross(p03).normal());
		}
	}

	// Now compute vertex normals from face normals.
	// Every vertex has six adjacent faces, in ccw order:
	// face0 = (x  ,y  )
	// face1 = (x-1,y  )
	// face2 = (x-2,y-1)
	// face3 = (x-1,y-1)
	// face4 = (x  ,y-1)
	// face5 = (x+1,y  )
	normals.reserve(1 << (2*resolution_shift));
	for (unsigned y = 0; y < resolution; ++y) {
		unsigned y1 = y * resolution;
		unsigned y2 = ((y + resolution - 1) & (resolution - 1)) * resolution;
		for (unsigned x = 0; x < resolution; ++x) {
			unsigned x1 = (x + resolution - 1) & (resolution - 1);
			const vector3f& f0 = facenormals[2*(y1+x)];
			const vector3f& f1 = facenormals[2*(y1+x1)+1];
			const vector3f& f2 = facenormals[2*(y2+x1)];
			const vector3f& f3 = facenormals[2*(y2+x1)+1];
			const vector3f& f4 = facenormals[2*(y2+x)];
			const vector3f& f5 = facenormals[2*(y1+x)+1];
			normals.push_back((f0 + f1 + f2 + f3 + f4 + f5).normal());
		}
	}

	// compute texture data
	//fixme: for higher levels the computed data is not used, as mipmaps are generated
	//by glu. however this data should be much better!
	//but we can't feed it to texture class yet
	normals_tex.resize(resolution*resolution*3); // fixme: for now use RGB
	unsigned ptr = 0;
	for (unsigned y = 0; y < resolution; ++y) {
		for (unsigned x = 0; x < resolution; ++x) {
			const vector3f& v = normals[ptr];
			normals_tex[3*ptr+0] = Uint8(v.x*127+128);
			normals_tex[3*ptr+1] = Uint8(v.y*127+128);
			normals_tex[3*ptr+2] = Uint8(v.z*127+128);
			++ptr;
		}
	}
}



void water::wavetile_phase::mipmap_level::debug_dump()
{
#if 0
	std::cout << "dump with res=" << resolution << " shift=" << resolution_shift << "\n";
	unsigned tm = sys().millisec();
	{
		// store heights as test image
		std::ostringstream ossn;
		ossn << "heights_" << tm << ".ppm";
		std::ofstream ofsn(ossn.str().c_str());
		ofsn << "P6\n" << resolution << " " << resolution << "\n255\n";
		float fac = 127/8.0f;
		for (unsigned i = 0; i < resolution*resolution; ++i) {
			uint8_t tmp[3];
			tmp[0] = wavedata[i].x * fac + 128;
			tmp[1] = wavedata[i].y * fac + 128;
			tmp[2] = wavedata[i].z * fac + 128;
			ofsn.write((const char*)tmp, 3);
		}
	}
	{
		// store normals as test image
		std::ostringstream ossn;
		ossn << "normals_" << tm << ".ppm";
		std::ofstream ofsn(ossn.str().c_str());
		ofsn << "P6\n" << resolution << " " << resolution << "\n255\n";
		for (unsigned i = 0; i < resolution*resolution; ++i) {
			uint8_t tmp[3];
			tmp[0] = normals[i].x * 127 + 128;
			tmp[1] = normals[i].y * 127 + 128;
			tmp[2] = normals[i].z * 127 + 128;
			ofsn.write((const char*)tmp, 3);
		}
	}
#endif
}



water::geoclipmap_patch::geoclipmap_patch(unsigned N,
					  unsigned level, unsigned border,
					  unsigned xoff, unsigned yoff,
					  unsigned columns, unsigned rows)
	: vbo(true),
	  min_vertex_index(0xffffffff),
	  max_vertex_index(0),
	  nr_indices(0),
	  use_fan(false)
{
// 	printf("gen patch: lev=%u border=%x, xyoff=%u/%u xysize=%u/%u\n",
// 	       level, border, xoff, yoff, columns, rows);
	// border = 0 (none), 1 (top), 2 (right), 4 (bottom), 8 (left)
	if (border != 0 && level == 0)
		throw error("can't use border on innermost level");
	// number of indices: per row we have 2 per quad plus 2 for the start and 2 for the
	// change to the next row (but not for last row).
	nr_indices = rows * (columns * 2 + 2 + 2) - 2;
	if (border & 0x0f) {
		// per row/colum we have three triangles (four, but one degenerated),
		// that makes two indices for the start and four per row/column
		unsigned parts = (border & 0x05) ? columns : rows; // either 0x4 or 0x1, bottom or top
		nr_indices += 2 /* degen. for conjunction */ + 2 + 4 * parts;
		// A,B,A,E,C,D
		// A---C
		// |\ /|
		// B-E-D
	}

	// Generate space for indices. Init VBO directly and map its space.
#if 1
	// if N <= 64, 16bit are enough to index vertices.... but we use 32 anyway, just in case.
	vbo.init_data(nr_indices * 4, 0, GL_STATIC_DRAW);
	uint32_t* index_data = (uint32_t*) vbo.map(GL_WRITE_ONLY);
	uint32_t last_index = 0; // avoid reading from VBO
#else
	vbo.init_data(nr_indices * 2, 0, GL_STATIC_DRAW);
	uint16_t* index_data = (uint16_t*) vbo.map(GL_WRITE_ONLY);
	uint16_t last_index = 0; // avoid reading from VBO
#endif

	unsigned index_ptr = 0;

	bool left_to_right = true;
	unsigned vertex_offset = level * (N+1)*(N+1) + yoff * (N+1) + xoff;
	min_vertex_index = vertex_offset;

	// write columns*rows
	for (unsigned y = 0; y < rows; ++y) {
		if (left_to_right) {
			if (y > 0) {
				// degenerated triangles for conjunction
				index_data[index_ptr+0] = last_index;
				index_data[index_ptr+1] = vertex_offset + N+1;
				index_ptr += 2;
			}
			// strip start
			index_data[index_ptr+0] = vertex_offset + N+1;
			index_data[index_ptr+1] = vertex_offset;
			if (y == 0 && border == 0x20) {
				// upper right patch, starts on corner
				index_data[index_ptr+1] = min_vertex_index =
					(level-1) * (N+1)*(N+1) + N*(N+1) + N;
			} else if (y + 1 == rows && border == 0x40) {
				// lower right patch, last line connects to corner
				index_data[index_ptr+0] = min_vertex_index =
					(level-1) * (N+1)*(N+1) + N;
			}
			index_ptr += 2;
			for (unsigned x = 0; x < columns; ++x) {
				index_data[index_ptr+0] = vertex_offset + x+1 + N+1;
				index_data[index_ptr+1] = vertex_offset + x+1;
				index_ptr += 2;
			}
			last_index = vertex_offset + columns;
			max_vertex_index = vertex_offset + columns + N+1;
			if (y == 0 && border == 0x10) {
				// upper left patch, first line ends on corner
				index_data[index_ptr-1] = last_index = min_vertex_index =
					(level-1) * (N+1)*(N+1) + N*(N+1);
			} else if (y + 1 == rows && border == 0x80) {
				// lower left patch, last line ends on corner
				index_data[index_ptr-2] = min_vertex_index =
					(level-1) * (N+1)*(N+1);
				max_vertex_index = vertex_offset + columns-1 + N+1;
			}
		} else {
			if (y > 0) {
				// degenerated triangles for conjunction
				index_data[index_ptr+0] = last_index;
				index_data[index_ptr+1] = vertex_offset + columns;
				index_ptr += 2;
			}
			// strip start
			index_data[index_ptr+0] = vertex_offset + columns;
			index_data[index_ptr+1] = vertex_offset + columns + N+1;
			max_vertex_index = vertex_offset + N+1 + columns;
			if (y + 1 == rows && border == 0x80) {
				// lower left patch, last line connects to corner
				index_data[index_ptr+1] = min_vertex_index =
					(level-1) * (N+1)*(N+1);
				max_vertex_index = vertex_offset + N+1 + columns-1;
			}
			index_ptr += 2;
			for (unsigned x = 0; x < columns; ++x) {
				index_data[index_ptr+0] = vertex_offset + columns-1 - x;
				index_data[index_ptr+1] = vertex_offset + columns-1 - x + N+1;
				index_ptr += 2;
			}
			last_index = vertex_offset + N+1;
			if (y + 1 == rows && border == 0x40) {
				// lower right patch, last line ends on corner
				index_data[index_ptr-1] = last_index = min_vertex_index =
					(level-1) * (N+1)*(N+1) + N;
			}
		}
		vertex_offset += N+1;
		left_to_right = !left_to_right;
	}

	if (border & 0x01) {
		// border at top
		// rightmost vertex on bottom of inner level's vertices.
		vertex_offset = level * (N+1)*(N+1) + (yoff + rows) * (N+1) + xoff + columns; // "A"
		unsigned vertex_offset2 = (level-1) * (N+1)*(N+1) + N; // "B"
		min_vertex_index = vertex_offset2 - N;
		// degenerated triangles for conjunction
		index_data[index_ptr+0] = last_index;
		index_data[index_ptr+1] = vertex_offset;
		index_ptr += 2;
		// start vertices.
		index_data[index_ptr+0] = vertex_offset;
		index_data[index_ptr+1] = vertex_offset2;
		index_ptr += 2;
		for (unsigned x = 0; x < columns; ++x) {
			index_data[index_ptr+0] = vertex_offset; // A
			index_data[index_ptr+1] = vertex_offset2 - 1; // E
			index_data[index_ptr+2] = vertex_offset - 1; // C
			index_data[index_ptr+3] = vertex_offset2 - 2; // D
			index_ptr += 4;
			vertex_offset -= 1;
			vertex_offset2 -= 2;
		}
	} else if (border & 0x04) {
		// border at bottom
		// leftmost vertex on top of inner level's vertices.
		vertex_offset = level * (N+1)*(N+1) + yoff * (N+1) + xoff; // "A"
		unsigned vertex_offset2 = (level-1) * (N+1)*(N+1) + N*(N+1); // "B"
		min_vertex_index = vertex_offset2;
		// degenerated triangles for conjunction
		index_data[index_ptr+0] = last_index;
		index_data[index_ptr+1] = vertex_offset;
		index_ptr += 2;
		// start vertices.
		index_data[index_ptr+0] = vertex_offset;
		index_data[index_ptr+1] = vertex_offset2;
		index_ptr += 2;
		for (unsigned x = 0; x < columns; ++x) {
			index_data[index_ptr+0] = vertex_offset; // A
			index_data[index_ptr+1] = vertex_offset2 + 1; // E
			index_data[index_ptr+2] = vertex_offset + 1; // C
			index_data[index_ptr+3] = vertex_offset2 + 2; // D
			index_ptr += 4;
			vertex_offset += 1;
			vertex_offset2 += 2;
		}
	} else if (border & 0x02) {
		// border at right
		// leftmost vertex on bottom of inner level's vertices.
		vertex_offset = level * (N+1)*(N+1) + yoff * (N+1) + xoff + columns; // "A"
		unsigned vertex_offset2 = (level-1) * (N+1)*(N+1); // "B"
		min_vertex_index = vertex_offset2;
		// degenerated triangles for conjunction
		index_data[index_ptr+0] = last_index;
		index_data[index_ptr+1] = vertex_offset;
		index_ptr += 2;
		// start vertices.
		index_data[index_ptr+0] = vertex_offset;
		index_data[index_ptr+1] = vertex_offset2;
		index_ptr += 2;
		for (unsigned y = 0; y < rows; ++y) {
			index_data[index_ptr+0] = vertex_offset; // A
			index_data[index_ptr+1] = vertex_offset2 + N+1; // E
			index_data[index_ptr+2] = vertex_offset + N+1; // C
			index_data[index_ptr+3] = vertex_offset2 + 2*(N+1); // D
			index_ptr += 4;
			vertex_offset += N+1;
			vertex_offset2 += 2*(N+1);
		}
	} else if (border & 0x08) {
		// border at left
		// rightmost vertex on top of inner level's vertices.
		vertex_offset = level * (N+1)*(N+1) + (yoff + rows) * (N+1) + xoff; // "A"
		unsigned vertex_offset2 = (level-1) * (N+1)*(N+1) + N*(N+1) + N; // "B"
		min_vertex_index = vertex_offset2 - N*(N+1);
		// degenerated triangles for conjunction
		index_data[index_ptr+0] = last_index;
		index_data[index_ptr+1] = vertex_offset;
		index_ptr += 2;
		// start vertices.
		index_data[index_ptr+0] = vertex_offset;
		index_data[index_ptr+1] = vertex_offset2;
		index_ptr += 2;
		for (unsigned y = 0; y < rows; ++y) {
			index_data[index_ptr+0] = vertex_offset; // A
			index_data[index_ptr+1] = vertex_offset2 - (N+1); // E
			index_data[index_ptr+2] = vertex_offset - (N+1); // C
			index_data[index_ptr+3] = vertex_offset2 - 2*(N+1); // D
			index_ptr += 4;
			vertex_offset -= N+1;
			vertex_offset2 -= 2*(N+1);
		}
	}

	// do not forget...
	vbo.unmap();
	vbo.unbind();

#if 0 // paranoia checks for debug
	if (index_ptr != nr_indices) {
		printf("ERROR: idxptr %u nri %u\n",index_ptr,nr_indices);
		throw error("BUG");
	}
	unsigned jmin = 99999999, jmax = 0;
	for (unsigned j = 0; j < nr_indices; ++j) {
		jmin = std::min(jmin, index_data[j]);
		jmax = std::max(jmax, index_data[j]);
	}
	if (jmin != min_vertex_index) {
		printf("ERROR: border %02x jmin %u min_vertex_index %u xyoff=%u,%u rowcol=%u,%u\n", border, jmin, min_vertex_index, xoff,yoff,columns,rows);
		throw error("BUG");
	}
	if (jmax != max_vertex_index) {
		printf("ERROR: border %02x jmax %u max_vertex_index %u xyoff=%u,%u rowcol=%u,%u\n", border, jmax, max_vertex_index, xoff,yoff,columns,rows);
		throw error("BUG");
	}
#endif
}



water::geoclipmap_patch::geoclipmap_patch(unsigned N,
					  unsigned highest_level,
					  unsigned border)
	: vbo(true),
	  min_vertex_index(0xffffffff),
	  max_vertex_index(0),
	  nr_indices(0),
	  use_fan(true)
{
	nr_indices = N+1 + 3;
	// Generate space for indices. Init VBO directly and map its space.
#if 1
	// if N <= 64, 16bit are enough to index vertices.... but we use 32 anyway, just in case.
	vbo.init_data(nr_indices * 4, 0, GL_STATIC_DRAW);
	uint32_t* index_data = (uint32_t*) vbo.map(GL_WRITE_ONLY);
#else
	vbo.init_data(nr_indices * 2, 0, GL_STATIC_DRAW);
	uint16_t* index_data = (uint16_t*) vbo.map(GL_WRITE_ONLY);
#endif

	unsigned index_ptr = 0;
	unsigned vertex_offset = (highest_level+1) * (N+1)*(N+1);
	if (border & 0x01) {
		// border at top
		index_data[index_ptr++] = vertex_offset + 6;
		index_data[index_ptr++] = vertex_offset + 5;
		unsigned vertex_offset2 = highest_level * (N+1)*(N+1) + N*(N+1);
		for (unsigned x = 0; x < N+1; ++x) {
			index_data[index_ptr++] = vertex_offset2 + x;
		}
		index_data[index_ptr++] = vertex_offset + 7;
		min_vertex_index = vertex_offset2;
		max_vertex_index = vertex_offset + 7;
	} else if (border & 0x02) {
		// border at right
		index_data[index_ptr++] = vertex_offset + 4;
		index_data[index_ptr++] = vertex_offset + 7;
		unsigned vertex_offset2 = highest_level * (N+1)*(N+1) + N*(N+1) + N;
		for (unsigned x = 0; x < N+1; ++x) {
			index_data[index_ptr++] = vertex_offset2;
			vertex_offset2 -= N+1;
		}
		index_data[index_ptr++] = vertex_offset + 2;
		min_vertex_index = vertex_offset2 + N+1;
		max_vertex_index = vertex_offset + 7;
	} else if (border & 0x04) {
		// border at bottom
		index_data[index_ptr++] = vertex_offset + 1;
		index_data[index_ptr++] = vertex_offset + 2;
		unsigned vertex_offset2 = highest_level * (N+1)*(N+1) + N;
		for (unsigned x = 0; x < N+1; ++x) {
			index_data[index_ptr++] = vertex_offset2 - x;
		}
		index_data[index_ptr++] = vertex_offset + 0;
		min_vertex_index = vertex_offset2 - N;
		max_vertex_index = vertex_offset + 2;
	} else if (border & 0x08) {
		// border at left
		index_data[index_ptr++] = vertex_offset + 3;
		index_data[index_ptr++] = vertex_offset + 0;
		unsigned vertex_offset2 = highest_level * (N+1)*(N+1);
		min_vertex_index = vertex_offset2;
		for (unsigned x = 0; x < N+1; ++x) {
			index_data[index_ptr++] = vertex_offset2;
			vertex_offset2 += N+1;
		}
		index_data[index_ptr++] = vertex_offset + 5;
		max_vertex_index = vertex_offset + 5;
	}

	// do not forget...
	vbo.unmap();
	vbo.unbind();

#if 0 // paranoia checks for debug
	if (index_ptr != nr_indices) {
		printf("ERROR: idxptr %u nri %u border %u\n",index_ptr,nr_indices,border);
		throw error("BUG");
	}
	unsigned jmin = 99999999, jmax = 0;
	for (unsigned j = 0; j < nr_indices; ++j) {
		jmin = std::min(jmin, index_data[j]);
		jmax = std::max(jmax, index_data[j]);
	}
	if (jmin != min_vertex_index) {
		printf("ERROR: jmin %u min_vertex_index %u border %u\n", jmin, min_vertex_index,border);
		throw error("BUG");
	}
	if (jmax != max_vertex_index) {
		printf("ERROR: jmax %u max_vertex_index %u border %u\n", jmax, max_vertex_index,border);
		throw error("BUG");
	}
#endif
}




void water::geoclipmap_patch::render() const
{
	// vertex/texture pointers are already set up
	vbo.bind();
	glDrawRangeElements(use_fan ? GL_TRIANGLE_FAN : GL_TRIANGLE_STRIP,
			    min_vertex_index, max_vertex_index, nr_indices, GL_UNSIGNED_INT, 0);
	vbo.unbind();
}
