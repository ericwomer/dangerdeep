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

#include "system.h"
#include <fstream>


// some more interesting values: phase 256, waveperaxis: ask your gfx card, facesperwave 64+,
// wavelength 256+,
#define WAVE_PHASES 256		// no. of phases for wave animation
#define WAVES_PER_AXIS 8	// no. of waves along x or y axis
#define FACES_PER_AXIS (WAVES_PER_AXIS*tile_res)
#define WAVE_LENGTH 128.0	// in meters, total length of one wave tile
#define TIDECYCLE_TIME 10.0	// seconds
#define FOAM_VANISH_FACTOR 0.1	// 1/second until foam goes from 1 to 0.
#define FOAM_SPAWN_FACTOR 0.2	// 1/second until full foam reached. maybe should be equal to vanish factor





water::water(unsigned bdetail, double tm) : mytime(tm), base_detail(bdetail), tile_res(1<<bdetail), reflectiontex(0), foamtex(0)
{
	wavetiledisplacements.resize(WAVE_PHASES);
	wavetileheights.resize(WAVE_PHASES);
	wavetilenormals.resize(WAVE_PHASES);

	glGenTextures(1, &reflectiontex);
	glBindTexture(GL_TEXTURE_2D, reflectiontex);
	vector<Uint8> fresnelspecul(256*256*3);
	for (int y = 0; y < 256; y++) {
		color wc = color(color(18, 73, 107), color(162, 193, 216 /*54, 98, 104*/), y/255.0f);
		for (int x = 0; x < 256; x++) {
			float scal = x/255.0f;
			color fc = color(wc, color(239, 237, 203 /*255, 255, 255*/), scal);

//#define SPHEREMAPCOORDS
#ifdef SPHEREMAPCOORDS
			float d = vector2f(x-127.5f, y-127.5f).length();
			d = d / 127.5f;
			if (d > 1.0f) d = 1.0f;
			d = cos(asin(d));
			d = d + 1.0f;
			d = d*d;
			d = d*d;
			d = d*d;
			d = 1.0f/d;
			fc = color(color(18, 73, 107), color(162, 193, 216), d);
#endif			
			fresnelspecul[(x+y*256)*3+0] = fc.r;
			fresnelspecul[(x+y*256)*3+1] = fc.g;
			fresnelspecul[(x+y*256)*3+2] = fc.b;
		}
	}

#ifdef SPHEREMAPCOORDS
	ofstream oss("fresnel.ppm");
	oss << "P6\n256 256\n255\n";
	oss.write((const char*)(&fresnelspecul[0]), 256*256*3);
#endif

	coords.reserve((tile_res+1)*(tile_res+1));
	colors.reserve((tile_res+1)*(tile_res+1));
	uv0.reserve((tile_res+1)*(tile_res+1));
#ifdef SPHEREMAPCOORDS
	normals.reserve((tile_res+1)*(tile_res+1));
#endif

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, &fresnelspecul[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	foamtex = new texture(get_texture_dir() + "foam.png", GL_LINEAR);//fixme maybe mipmap it
	
	// connectivity data is the same for all meshes and thus is reused
	waveindices.resize(base_detail);		// level 0 is minimum detail (4 quads per tile)
	for (unsigned i = 0; i < base_detail; ++i) {
		unsigned res = (2<<i);
		waveindices[i].reserve(res*res*4);
		for (unsigned y = 0; y < res; ++y) {
			unsigned y2 = y+1;
			for (unsigned x = 0; x < res; ++x) {
				unsigned x2 = x+1;
				waveindices[i].push_back(x +y *(res+1));
				waveindices[i].push_back(x2+y *(res+1));
				waveindices[i].push_back(x2+y2*(res+1));
				waveindices[i].push_back(x +y2*(res+1));
			}
		}
	}

	ocean_wave_generator<float> owg(tile_res, vector2f(1,1), 31 /*10*/ /*31*/, 5e-6, WAVE_LENGTH, TIDECYCLE_TIME);
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


void water::display(const vector3& viewpos, angle dir, double max_view_dist /*, bool onlyflatwater */) const
{
	bool onlyflatwater = false;

	// fixme: draw some quads between the farest wave tiles and the horizon to fill the gaps
	// wave tiles are drawn in triangle shape, like a rasterized triangle. draw some quads
	// from the border of the outmost tiles (matching their resolution and height) to a line
	// parallel to horizon/user view. from that line begin to bend the water faces according to
	// earth curvature. this avoids gaps and looks right.

	// the origin of the coordinate system is the bottom left corner of the tile
	// that the viewer position projected to xy plane is inside.
	glPushMatrix();

	vector3f transl(-myfmod(viewpos.x, WAVE_LENGTH), -myfmod(viewpos.y, WAVE_LENGTH), -viewpos.z);
//	glTranslated(-myfmod(viewpos.x, WAVE_LENGTH), -myfmod(viewpos.y, WAVE_LENGTH), -viewpos.z);

	// draw polygons to the horizon
	float wr = WAVES_PER_AXIS * WAVE_LENGTH / 2;
	float c0 = -max_view_dist;
	float c1 = -wr;
	float c2 = wr;
	float c3 = max_view_dist;
	float wz = 0;//-WAVE_HEIGHT; // this leads to hole between waves and horizon faces, fixme

	// set Color to light vector transformed to object space
	// (we need the inverse of the modelview matrix for that, at least the 3x3 upper left
	// part if we have directional light only)
	vector3f lightvec = vector3f(-1,0.5,1).normal();
	lightvec = lightvec * 0.5 + vector3f(0.5, 0.5, 0.5);
	glColor3f(lightvec.x, lightvec.y, lightvec.z);
	
	// set texture unit to combine primary color with texture color via dot3
	int phase = int((myfmod(mytime, TIDECYCLE_TIME)/TIDECYCLE_TIME) * WAVE_PHASES);
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	glDisable(GL_LIGHTING);

	// ******************* set up textures
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	
	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, reflectiontex);

//	float refraccol[4] = { 0.204f, 0.388f, 0.408f, 1.0f };
//	glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, refraccol);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
/*
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
	glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_INTERPOLATE);
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_CONSTANT);
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE);
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE2_RGB, GL_PRIMARY_COLOR);
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND2_RGB, GL_SRC_COLOR);
*/

/* not needed for fresnel/specular
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PRIMARY_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_CONSTANT);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);
*/

#ifdef SPHEREMAPCOORDS
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
#endif

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
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND2_RGB, GL_SRC_COLOR);

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

	// now set pointers, enable arrays
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(color), &colors[0].r);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(vector3f), &coords[0].x);
	glClientActiveTexture(GL_TEXTURE0);
#ifdef SPHEREMAPCOORDS
	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, sizeof(vector3f), &normals[0].x);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
#else
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, sizeof(vector2f), &uv0[0].x);
	glDisableClientState(GL_NORMAL_ARRAY);
#endif
	glClientActiveTexture(GL_TEXTURE1);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	// ******************* draw tiles
	int s = 10;//0;//2
	for (int y = -s; y <= s; ++y) {
		for (int x = -s; x <= s; ++x) {
			vector3f transl2 = transl + vector3f(x*WAVE_LENGTH, y*WAVE_LENGTH, 0);
			int lod0 = compute_lod_by_trans(transl2);
			int fillgap = 0;
			if (compute_lod_by_trans(transl2+vector3f(0,WAVE_LENGTH,0)) < lod0) fillgap |= 1;
			if (compute_lod_by_trans(transl2+vector3f(WAVE_LENGTH,0,0)) < lod0) fillgap |= 2;
			if (compute_lod_by_trans(transl2+vector3f(0,-WAVE_LENGTH,0)) < lod0) fillgap |= 4;
			if (compute_lod_by_trans(transl2+vector3f(-WAVE_LENGTH,0,0)) < lod0) fillgap |= 8;
			draw_tile(transl2, phase, lod0, fillgap);
		}
	}

	// ******************* restore old texture state, disable arrays
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glClientActiveTexture(GL_TEXTURE0);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glColor4f(1,1,1,1);

	glActiveTexture(GL_TEXTURE1);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glActiveTexture(GL_TEXTURE0);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glPopAttrib();


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

	glPopAttrib();

	glPopMatrix();
	glColor3f(1,1,1);
}



// the viewer should be in position 0,0,0 for the LOD to work right and also for lighting
int water::compute_lod_by_trans(const vector3f& transl) const
{
	double meandist = transl.xy().length();
	const float maxloddist = 256.0f;
	const float minloddist = 5000.0f;
	float a = (minloddist - maxloddist) / (int(base_detail) - 1);
	float distfac = a/(meandist + a - maxloddist);
	int lodlevel = int(base_detail*distfac) - 1;
	if (lodlevel < 0) lodlevel = 0;
	if (lodlevel >= int(base_detail)) lodlevel = int(base_detail)-1;
	return lodlevel;
}



void water::draw_tile(const vector3f& transl, int phase, int lodlevel, int fillgap) const
{
	// use LOD
	// draw triangle strips or use vertex arrays
	// draw a grid of quads, for each vertex do:
	// get height/coordinate from precomputed heights/transl
	// compute normalized vector to watcher = E
	// get normal vector from precomputed data = N
	// get vector to light source (directional, so vector is const = L)
	// Reflection vector (specular) is 2*(L*N)*N - L = R (R may be precomputed and stored!)
	// compute (E*R)^m with m = specular constant, e.g. 16
	// result = S (luminance value)
	// compute Fresnel(E*N) = F, with Fresnel(x) ~ 1/(x+1)^8
	// compute water color = refractive color * (1-F) + reflective color * F
	// refractive color is deep blue and constant.
	// reflective color is obtained from a reflection map (or constant: sky color)
	// So per pixel color is: water color + S
	// all per vertex, so primary color is enough. With reflections, the reflection map
	// goes to tex0, Fresnel as primary color, refractive as const color (texenv) and
	// S per vertex (per vertex needed, but differs from Fresnel).
	// tex1 may be needed for foam
	// in total: pixel color = S + refrac*(1-F) + tex0*F
	//                         *   -       *      #    *
	// * per vertex, - const, # per pixel
	// interpolate and add does not work! add for tex1 doesn't work, we need it to blend in
	// foam! we need three operations but have two.
	// However S is mono, so we could set up the blend function to SRC_ALPHA, GL_ONE
	// (alpha channel controls interpolation between color and white)
	// and store S as alpha channel in the primary color.
	// So we have:
	// Constant color: refractive color
	// Primary color: Fresnel term (RGB) / Specular value (ALPHA)
	// Texture 0: Reflection map
	// Texture 1: Foam
	// Foam can be mixed in with secondary color. Foam texture coordinates can get
	// computed by OpenGL.
	
	// Secondary color is added after texturing. So it's of no use, maybe it can be used
	// to add specular color. But it doesn't work either.
	// We could set specular color and Fresnel color at once by storing them in a texture
	// and use texture coordinates for mixing. This saves the primary color (maybe as foam
	// mixing value), but then we can't add a reflection texture. Do we need one?!

	// per pixel specular lighting looks very ugly. grid resolution is too low.
	// the fresnel term gives much more surface realism than a specular color.
	// per pixel fake specular lighting: disturb a per vertex lighting with dot3 bump
	// mapping, maybe linear fresnel (with dot3) is ok for small details?
	// high frequency waves could be done with perlin noise.
	
	// if we could approximate the fresnel term good enough then we could use tex0 for
	// dot3 computation and tex1 for final result computation -> per pixel fresnel.
	// maybe with lighting/fog we could add the color.
	// fresnel approximation is 1/(x+1)^8, ~~ 1-4*x which can be computed with one unit
	// but its too bad, we need a smoother fall-off for the value, 1-4*x gives a too hard
	// jump to zero reflection. A quadratic approximation would need unit1, and we need an
	// add/sub too, this is very difficult...
	// quadratic approx. is also not good enough.
	// per pixel fresnel on gf2 could work with register combiners, but is not ATI compatible
	// no way :-(

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
	
	// fixme: could we use spheremapping/cubemapping, automatic texture coordinate generation
	// to compute E or E*N?
	// yes, texture map is Fresnel(cos(asin(d))) with d = unit distance to texture center
	// i.e. d = 2*sqrt((x-0.5)^2+(y-0.5)^2) for x,y in [0,1]
	// but this leads to large parts of the texture to be blue, resolution is not high
	// enough to compute the Fresnel term with satisfying accuracy.
	// maybe a cube map could do better.
	// big problem: turning the view changes the texture coordinates, that means
	// turning the view changes either E (it shouldn't) or N (should only rotate N)
	// T=Modelviewmatrix, E'=T*E, N'=T*N (it should be, check with redbook, fixme)
	// E' is (0,0,1) then. n'.z = E'*N' = (T*E)*(T*N) ???
	// with E'=(0,0,1)=T*E is E=Tinv * E' = Tinv * (0,0,1) = Tinv,3rd col = T,2nd row
	// however: why does E or N change with turning, Fresnel doesn't?!
	// hmmm. E is (0,0,1) in eye space?! this is true only for vertices at the center of
	// the screen. Maybe sphere mapping works that way (E always 0,0,1 for all vertices)
	// in that case we can't use sphere mapping for fresnel...
	// NO! E is not always (0,0,1)!!!

	// Water drawing is slow. It seems that the per vertex computations are not the problem
	// nor is it the gap filling. Most probable reason is the amount of data that
	// has to be transferred each frame.
	// We have 4 colors, 3 coords, 2 texcoords per vertex (=4+12+8=24bytes)
	// with 65*65 verts per tile, 21*21 tiles, mean resolution, say, 20 we have
	// 21*21*21*21*24bytes/frame = ~4.7mb/frame, with 15frames ~ 70mb/second.

// fixme: opengl1.2 has a COLOR MATRIX so colors can get multiplied by a 4x4 matrix before drawing
// can we use this for anything?

	unsigned res = (2 << lodlevel);
	unsigned resi = res+1;
	unsigned revres = (1 << (base_detail-1 - lodlevel));

//#define DRAW_NORMALS
#ifdef DRAW_NORMALS
	glActiveTexture(GL_TEXTURE0);
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_LINES);
	glColor4f(1,1,1,1);
#endif

//glEnable(GL_LIGHTING);

	vector3f L = vector3f(1,0,1).normal();//fixme, get from caller!
//float tf = myfrac(mytime/10.0f)*2*M_PI;
//L = vector3f(cos(tf),sin(tf),1).normal();
	float add = float(WAVE_LENGTH)/res;
	float fy = 0;
	unsigned vecptr = 0;
	for (unsigned y = 0; y <= tile_res; y += revres) {
		float fx = 0;
		unsigned cy = y & (tile_res-1);
		for (unsigned x = 0; x <= tile_res; x += revres, ++vecptr) {
			unsigned cx = x & (tile_res-1);
			unsigned ptr = cy*tile_res+cx;
			vector3f coord = transl + vector3f(
				fx + wavetiledisplacements[phase][ptr].x,
				fy + wavetiledisplacements[phase][ptr].y,
				wavetileheights[phase][ptr]);
			coords[vecptr] = coord;
			vector3f N = wavetilenormals[phase][ptr];
#ifdef SPHEREMAPCOORDS
			normals[vecptr] = N;
#endif			
			vector3f E = -coord.normal();

			vector3f R = (2.0f*(L*N))*N - L;	// fixme precompute?

#ifdef DRAW_NORMALS
			glColor3f(1,0,0);
			vector3f u = coord + N * 1.0f;
			glVertex3fv(&coord.x);
			glVertex3fv(&u.x);
			glColor3f(1,1,0);
			u = coord + E * 10.0f + vector3f(0.1, 0.1, 0.0);
			glVertex3fv(&coord.x);
			glVertex3fv(&u.x);
			glColor3f(0,1,0);
			u = coord + R * 1.0f;
			glVertex3fv(&coord.x);
			glVertex3fv(&u.x);
#endif

			float S = E*R;		// specular component
			S = S * S;		// ^2
			S = S * S;		// ^4
			S = S * S;		// ^8
			S = S * S;		// ^16
			S = S * S;		// ^32
			// fixme: specular highlights appear to large, why?
		S = 0.0f;

			if (S > 1.0f) S = 1.0f;	// clamp value
			if (S < 0.0f) S = 0.0f;	// fixme: texture clamping does that already

			float F = E*N;		// compute Fresnel term F(x) = 1/(x+1)^8
			if (F < 0.0f) F = 0.0f;	// avoid angles > 90 deg.
			F = F + 1.0f;
			F = F * F;	// ^2
			F = F * F;	// ^4
			F = F * F;	// ^8
			F = 1.0f/F;
						
				// fixme: green/blue depends on slope only? is color of refraction? or fresnel?! read papers!!! fixme
			uv0[vecptr] = vector2f(S, F);
			Uint8 foampart = 255; //(((x*x+1)*(y+3))&7)==0?0:255;//test hack
//			Uint8 c = Uint8(F*255);	// fresnel term as part of color, if tex0 is reflection map
//			colors[vecptr] = color(c, c, c, Uint8(S*255));
			colors[vecptr] = color(foampart, foampart, foampart, 255);
			fx += add;
		}
		fy += add;
	}
	
// fixme: per vertex specular lighting could be done with OpenGL light sources?

#ifdef DRAW_NORMALS
	glEnd();
	glEnable(GL_TEXTURE_2D);
#endif

	// to avoid gaps, reposition the vertices, fillgap bits: 0,1,2,3 for top,right,bottom,left
	if (fillgap & 1) {
		for (unsigned i = res*resi+1; i < resi*resi-1; i += 2) {
			coords[i] = (coords[i-1] + coords[i+1]) * 0.5f;
			colors[i] = color(colors[i-1], colors[i+1], 0.5f);
			uv0[i] = (uv0[i-1] + uv0[i+1]) * 0.5f;
		}
	}
	if (fillgap & 2) {
		for (unsigned i = 2*resi-1; i < resi*resi-1; i += 2*resi) {
			coords[i] = (coords[i-resi] + coords[i+resi]) * 0.5f;
			colors[i] = color(colors[i-resi], colors[i+resi], 0.5f);
			uv0[i] = (uv0[i-resi] + uv0[i+resi]) * 0.5f;
		}
	}
	if (fillgap & 4) {
		for (unsigned i = 1; i < resi-1; i += 2) {
			coords[i] = (coords[i-1] + coords[i+1]) * 0.5f;
			colors[i] = color(colors[i-1], colors[i+1], 0.5f);
			uv0[i] = (uv0[i-1] + uv0[i+1]) * 0.5f;
		}
	}
	if (fillgap & 8) {
		for (unsigned i = resi; i < resi*res; i += 2*resi) {
			coords[i] = (coords[i-resi] + coords[i+resi]) * 0.5f;
			colors[i] = color(colors[i-resi], colors[i+resi], 0.5f);
			uv0[i] = (uv0[i-resi] + uv0[i+resi]) * 0.5f;
		}
	}

	// draw precomputed index list according to detail level
	glDrawElements(GL_QUADS, res*res*4, GL_UNSIGNED_INT, &(waveindices[lodlevel][0]));
}


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
	float ffac = tile_res/WAVE_LENGTH;
	float x = float(myfmod(pos.x, WAVE_LENGTH)) * ffac;
	float y = float(myfmod(pos.y, WAVE_LENGTH)) * ffac;
	int ix = int(floor(x));
	int iy = int(floor(y));
	int ix2 = (ix+1)%tile_res;
	int iy2 = (iy+1)%tile_res;
	float fracx = x - ix;
	float fracy = y - iy;
	float a = wavetileheights[wavephase][ix+iy*tile_res];
	float b = wavetileheights[wavephase][ix2+iy*tile_res];
	float c = wavetileheights[wavephase][ix+iy2*tile_res];
	float d = wavetileheights[wavephase][ix2+iy2*tile_res];
	float e = a * (1.0f-fracx) + b * fracx;
	float f = c * (1.0f-fracx) + d * fracx;
	return (1.0f-fracy) * e + fracy * f;
}

vector3f water::get_normal(const vector2& pos, double f) const
{
	double t = myfrac(mytime/TIDECYCLE_TIME);
	int wavephase = int(WAVE_PHASES*t);
	float ffac = tile_res/WAVE_LENGTH;
	float x = float(myfmod(pos.x, WAVE_LENGTH)) * ffac;
	float y = float(myfmod(pos.y, WAVE_LENGTH)) * ffac;
	int ix = int(floor(x));
	int iy = int(floor(y));
	int ix2 = (ix+1)%tile_res;
	int iy2 = (iy+1)%tile_res;
	float fracx = x - ix;
	float fracy = y - iy;
	vector3f a = wavetilenormals[wavephase][ix+iy*tile_res];
	vector3f b = wavetilenormals[wavephase][ix2+iy*tile_res];
	vector3f c = wavetilenormals[wavephase][ix+iy2*tile_res];
	vector3f d = wavetilenormals[wavephase][ix2+iy2*tile_res];
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
