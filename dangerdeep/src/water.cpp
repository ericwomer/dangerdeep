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
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);//CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);//CLAMP_TO_EDGE);

	coords.reserve((tile_res+1)*(tile_res+1));
	colors.reserve((tile_res+1)*(tile_res+1));
	uv0.reserve((tile_res+1)*(tile_res+1));
//	normals.reserve((tile_res+1)*(tile_res+1));

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

	ocean_wave_generator<float> owg(tile_res, vector2f(1,1), 2 /*10*/ /*31*/, 5e-6, WAVE_LENGTH, TIDECYCLE_TIME);
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
	GLdouble m[16];
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
glTranslated(0.5,0.5,0);
glScaled(0.5,0.5,1.0);	// check if this matches the texture
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

	// now set pointers, enable arrays
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(color), &colors[0].r);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(vector3f), &coords[0].x);
	glClientActiveTexture(GL_TEXTURE0);
//	glEnableClientState(GL_NORMAL_ARRAY);
//	glNormalPointer(GL_FLOAT, sizeof(vector3f), &normals[0].x);
//	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(3, GL_FLOAT, sizeof(vector3f), &uv0[0].x);
	glDisableClientState(GL_NORMAL_ARRAY);
	glClientActiveTexture(GL_TEXTURE1);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	// ******************* draw tiles
	int s = 6;//0;//2
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

	glActiveTexture(GL_TEXTURE0);
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);

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

	const float VIRTUAL_PLANE_HEIGHT = 0.0f;//5.0f;	// fixme experiment, amount of distorsion

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
			vector3f N = wavetilenormals[phase][ptr];
			vector3f E = -coord.normal();

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

			float F = E*N;		// compute Fresnel term F(x) = 1/(x+1)^8
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
						
			coords[vecptr] = coord;
//			normals[vecptr] = N;
			uv0[vecptr] = texc;
			colors[vecptr] = primary;
			fx += add;
		}
		fy += add;
	}
	
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
