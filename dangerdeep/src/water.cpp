// (ocean) water simulation and display (OpenGL)
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#pragma warning (disable : 4786)
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL.h>

#include "oglext/OglExt.h"

#include "water.h"
#include "texture.h"
#include "global_data.h"
#include "ocean_wave_generator.h"
#include "matrix.h"

#include "system.h"
#include <fstream>


// some more interesting values: phase 256, waveperaxis: ask your gfx card, facesperwave 64+,
// wavelength 256+,
#define WAVE_PHASES 256		// no. of phases for wave animation
#define WAVE_RESOLUTION 64	// FFT resolution
#define WAVE_LENGTH 128.0	// in meters, total length of one wave tile
#define TIDECYCLE_TIME 10.0	// seconds
#define FOAM_VANISH_FACTOR 0.1	// 1/second until foam goes from 1 to 0.
#define FOAM_SPAWN_FACTOR 0.2	// 1/second until full foam reached. maybe should be equal to vanish factor





water::water(unsigned xres_, unsigned yres_, double tm) : mytime(tm), xres(xres_), yres(yres_), reflectiontex(0), foamtex(0)
{
	wavetiledisplacements.resize(WAVE_PHASES);
	wavetileheights.resize(WAVE_PHASES);
	wavetilenormals.resize(WAVE_PHASES);

	glGenTextures(1, &reflectiontex);
	glBindTexture(GL_TEXTURE_2D, reflectiontex);
	unsigned rx = system::sys().get_res_x();
	unsigned ry = system::sys().get_res_y();
	unsigned vps = texture::get_max_size();
	if (ry < vps)
		for (unsigned i = 1; i < ry; i *= 2) vps = i;
	reflectiontexsize = vps;
	vector<Uint8> tmp0(reflectiontexsize*reflectiontexsize*3);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, reflectiontexsize, reflectiontexsize, 0, GL_RGB, GL_UNSIGNED_BYTE, &tmp0[0]);
	tmp0.clear();
	// fixme: auto mipmap?
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	coords.resize((xres+1)*(yres+1));
	colors.resize((xres+1)*(yres+1));
	uv0.resize((xres+1)*(yres+1));
	normals.resize((xres+1)*(yres+1));

	foamtex = new texture(get_texture_dir() + "foam.png", GL_LINEAR);//fixme maybe mipmap it
	
	// connectivity data is the same for all meshes and thus is reused
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

	ocean_wave_generator<float> owg(WAVE_RESOLUTION, vector2f(1,1), 8 /*10*/ /*31*/, 5e-6, WAVE_LENGTH, TIDECYCLE_TIME);
	for (unsigned i = 0; i < WAVE_PHASES; ++i) {
		owg.set_time(i*TIDECYCLE_TIME/WAVE_PHASES);
		wavetileheights[i] = owg.compute_heights();
		wavetiledisplacements[i] = owg.compute_displacements(-2.0f);	// fixme 5.0 default? - it seems that choppy waves don't look right. bug? fixme, with negative values it seems right. check this!

#if 1	// use finite normals
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
}


water::~water()
{
	glDeleteTextures(1, &reflectiontex);
	delete foamtex;
}


void water::setup_textures(void) const
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glDisable(GL_LIGHTING);
	
	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, reflectiontex);
	GLdouble m[16];
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	// rescale coordinates [-1,1] to [0,1]
	glTranslated(0.5,0.5,0);
	glScaled(0.5,0.5,1.0);
	glGetDoublev(GL_PROJECTION_MATRIX, m);
	glMultMatrixd(m);
	glGetDoublev(GL_MODELVIEW_MATRIX, m);
	glMultMatrixd(m);
	glMatrixMode(GL_MODELVIEW);

	float refraccol[4] = { 0.0706f, 0.2863f, 0.4196f, 1.0f };
	glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, refraccol);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
	glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_INTERPOLATE);
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_CONSTANT);
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE2_RGB, GL_PRIMARY_COLOR);
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND2_RGB, GL_SRC_COLOR);

/*
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PRIMARY_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_CONSTANT);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);
*/

	glActiveTexture(GL_TEXTURE1);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, foamtex->get_opengl_name());
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
	glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_INTERPOLATE);
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PREVIOUS);
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE);
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE2_RGB, GL_PRIMARY_COLOR);
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND2_RGB, GL_SRC_ALPHA);//COLOR);

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
}



void water::cleanup_textures(void) const
{
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



void water::display(const vector3& viewpos, angle dir, double max_view_dist) const
{
	matrix4 proj = matrix4::get_gl(GL_PROJECTION_MATRIX);
	matrix4 modl = matrix4::get_gl(GL_MODELVIEW_MATRIX);
	matrix4 prmd = proj * modl;

/*
	cout << "projection matrix\n";
	proj.print();
	cout << "modelview matrix\n";
	modl.print();
	cout << "projection*modelview matrix\n";
	prmd.print();
	cout << "inverse projection matrix\n";
	proj.inverse().print();
	cout << "inverse modelview matrix\n";
	modl.inverse().print();
	cout << "inverse (projection*modelview) matrix\n";
	prmd.inverse().print();
	cout << "accuracy test\n";
	(proj * proj.inverse()).print();
	(modl * modl.inverse()).print();
	(prmd * prmd.inverse()).print();
	matrix4 inv_proj = proj.inverse();
	matrix4 inv_prmd = prmd.inverse();
	cout << "inv +x " << (inv_prmd * vector3(1, 0, 0)) << "\n";
	cout << "inv -x " << (inv_prmd * vector3(-1, 0, 0)) << "\n";
	cout << "inv +y " << (inv_prmd * vector3(0, 1, 0)) << "\n";
	cout << "inv -y " << (inv_prmd * vector3(0, -1, 0)) << "\n";
	cout << "inv +z " << (inv_prmd * vector3(0, 0, 1)) << "\n";
	cout << "inv -z " << (inv_prmd * vector3(0, 0, -1)) << "\n";
	cout << "frustum coords in world space\n";
	cout << "near plane tl " << (inv_prmd * vector3(-1, 1, -1)) << "\n";
	cout << "near plane bl " << (inv_prmd * vector3(-1, -1, -1)) << "\n";
	cout << "near plane br " << (inv_prmd * vector3(1, -1, -1)) << "\n";
	cout << "near plane tr " << (inv_prmd * vector3(1, 1, -1)) << "\n";
	cout << "far plane tl " << (inv_prmd * vector3(-1, 1, 1)) << "\n";
	cout << "far plane bl " << (inv_prmd * vector3(-1, -1, 1)) << "\n";
	cout << "far plane br " << (inv_prmd * vector3(1, -1, 1)) << "\n";
	cout << "far plane tr " << (inv_prmd * vector3(1, 1, 1)) << "\n";
*/	

	int phase = int((myfmod(mytime, TIDECYCLE_TIME)/TIDECYCLE_TIME) * WAVE_PHASES);

	const float VIRTUAL_PLANE_HEIGHT = 30.0f;	// fixme experiment, amount of distorsion


	// compute projector matrix
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(0,0,50);
	glRotatef(45, 1,0,0);
	matrix4 mv = matrix4::get_gl(GL_MODELVIEW_MATRIX);
	glPopMatrix();
	matrix4 M_projector = (proj * /*mv*/ modl).inverse(); //fixme

	// compute coordinates
	for (unsigned yy = 0, ptr = 0; yy <= yres; ++yy) {
		double y = -1.0 + 2.0 * yy/yres;
		double v[16] = { -1,y,-1,1, -1,y,1,1, 1,y,-1,1, 1,y,1,1 };
		// vertices for start and end of two lines are computed and projected alltogether
		(M_projector * matrix4(v)).to_array(v);
		// compute intersection with z = 0 plane here
		// we could compute intersection with earth's sphere here for a curved display
		// of water to the horizon, fixme
		double t1 = v[2]/(v[6]-v[2]), t2 = v[10]/(v[14]-v[10]);
		vector2 va = vector2(v[0], v[1]) * (1-t1) + vector2(v[4], v[5]) * t1;
		vector2 vb = vector2(v[8], v[9]) * (1-t2) + vector2(v[12], v[13]) * t2;
		for (unsigned xx = 0; xx <= xres; ++xx, ++ptr) {
			double x = double(xx)/xres;
			vector2 v = va * (1-x) + vb * x;
			double h = get_height(v);//fixme replace by mipmap function get_coord
			coords[ptr] = vector3f(v.x, v.y, h);//get_coord(v);	// computes pos, height and displacements etc.	fixme
			
		}
	}

	// compute normals and remaining data (Fresnel etc.)
	for (unsigned yy = 0, ptr = 0; yy <= yres; ++yy) {
		for (unsigned xx = 0; xx <= xres; ++xx, ++ptr) {
			const vector3f& coord = coords[ptr];
			vector3f N = vector3f(0,0,1);	//fixme
			vector3f E = -coord.normal();	// viewer is in (0,0,0)
			float F = E*N;		// compute Fresnel term F(x) = ~ 1/(x+1)^8
			if (F < 0.0f) F = 0.0f;	// avoid angles > 90 deg.
			F = F + 1.0f;
			F = F * F;	// ^2
			F = F * F;	// ^4
			F = F * F;	// ^8
			F = 1.0f/F;
			Uint8 c = Uint8(F*255);
			Uint8 foampart = 255;
			color primary(c, c, c, foampart);

			vector3f texc = coord + N * (VIRTUAL_PLANE_HEIGHT * N.z);
			texc.z -= VIRTUAL_PLANE_HEIGHT;
						
			normals[ptr] = N;
			uv0[ptr] = texc;
			colors[ptr] = primary;
		}
	}

	// set up textures
	setup_textures();

	// draw elements (fixed index list) with vertex arrays using coords,uv0,normals,colors
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(color), &colors[0].r);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(vector3f), &coords[0].x);
	glClientActiveTexture(GL_TEXTURE0);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(3, GL_FLOAT, sizeof(vector3f), &uv0[0].x);
	glDisableClientState(GL_NORMAL_ARRAY);
	glClientActiveTexture(GL_TEXTURE1);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glDrawElements(GL_QUADS, gridindices.size(), GL_UNSIGNED_INT, &(gridindices[0]));

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glClientActiveTexture(GL_TEXTURE0);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

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
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_PRIMARY_COLOR);
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

