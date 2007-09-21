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

#include "texture.h"
#include "font.h"
#include "system.h"
#include "shader.h"
#include "datadirs.h"
#include "model.h"
#include "make_mesh.h"
#include "global_data.h"
#include "perlinnoise.h"
#include "sky.h"
#include "angle.h"
#include "bspline.h"
#include "log.h"

using namespace std;


const char* credits[] = {
"$80ffc0Project idea and initial code",
"$ffffffThorsten Jordan",
"", "", "", "", "",
"$80ffc0Program",
"$ffffffThorsten Jordan",
"Markus Petermann",
"Viktor Radnai",
"Andrew Rice",
"Alexandre Paes",
"Matt Lawrence",
"Michael Kieser",
"Renato Golin",
"Hiten Parmar",
"", "", "", "", "",
"$80ffc0Graphics",
"$ffffffLuis Barrancos",
"Markus Petermann",
"Christian Kolaß",
"Thorsten Jordan",
"", "", "", "", "",
"$80ffc0Models",
"$ffffffLuis Barrancos",
"Christian Kolaß",
"Thorsten Jordan",
"", "", "", "", "",
"$80ffc0Music and sound effects",
"$ffffffMartin Alberstadt",
"Marco Sarolo",
"", "", "", "", "",
"$80ffc0Hardcore Beta Testing",
"$ffffffAlexander W. Janssen",
"", "", "", "", "",
"$80ffc0Operating system adaption",
"$ffffffNico Sakschewski (Win32)",
"Andrew Rice (MacOSX)",
"Jose Alonso Cardenas Marquez (acm) (FreeBSD)",
"", "", "", "", "",
"$80ffc0Web site administrator",
"$ffffffMatt Lawrence",
"$ffffffAlexandre Paes",
"$ffffffViktor Radnai",
"", "", "", "", "",
"$80ffc0Packaging",
"$ffffffMarkus Petermann (SuSE rpm)",
"Viktor Radnai (Debian)",
"Giuseppe Borzi (Mandrake rpm)",
"Michael Kieser (WIN32-Installer)",
"", "", "", "", "",
"$80ffc0Bug reporting and thanks",
"$ffffffRick McDaniel",
"Markus Petermann",
"Viktor Radnai",
"Christian Kolaß",
"Nico Sakschewski",
"Martin Butterweck",
"Bernhard Kaindl",
"Robert Obryk",
"Giuseppe Lipari",
"John Hopkin",
"Michael Wilkinson",
"Lee Close",
"Christopher Dean (Naval Warfare Simulations, Sponsoring)",
"Arthur Anker",
"Stefan Vilijoen",
"Luis Barrancos",
"Tony Becker",
"Frank Kaune",
"Paul Marks",
"Aaron Kulkis",
"Giuseppe Borzi",
"Andrew Rice",
"Alexandre Paes",
"Alexander W. Janssen",
"vonhalenbach",
"...",
"...and all i may have forgotten here (write me!)",
"(no bockwursts were harmed in the making of this game).",
0
};



// ------------------------------------------- code ---------------------------------------

/*
geoclipmap rendering

we only need a function that tells us the height for any coordinate in
the (x,y) plane (with some parameter for the detail) and the render
resolution of each patch (called N by the authors, where N+1 is a power
of two, here our N is that of N+1 of the paper).

With that information we could render any kind of terrain, no matter if
the height is synthesized or taken from (compressed) stored data.

We have either one VBO for vertices of all levels or one VBO per
level. The vertices on the border between two levels aren't shared
except for the zero area triangles used to make the mesh watertight.
Having multiple VBOs could be better for updating, we don't need to map
the whole VBO, which interferes with rendering. Inner levels could be
updated while the card renders outer levels or similar
situations. However we would need some extra space to render the
connecting triangles, which could be done by reserving 4*N/2 space at
end of each VBO and storing there the heights of the next inner level
vertices on the edge, but this is clumsy as well.

It is easier to compute all vertex data for a level (thus N*N vertices)
and to not leave the center gap out. The gap in the center is N*N/4
vertices in size and is filled with the next finer level. The overhead
is small and we can reuse the computed height data (see below), plus we
can easily move the gap region around without reloading more vertex data
to the GPU.

Updating L-shaped regions of vertex data when the viewer moves is
problematic, that is for updating columns of vertices. This can't be
represented well by VBO commands. We would need one glBufferSubData call
per row, which is costly. The only alternative would be to map the
buffer, but this could interfere with drawing. And we can't map parts of
the data only. Mapping seems best alternative though. Some sites state
that mapping is so costly that it should be used for updated of more
than 32kb of data only.

The indices must be recomputed every frame. An obvious optimization is
to remember the clip area of the last frame, and to not recompute them
if that area didn't change. We could either draw the triangles directly
with data from host memory or copy the data to an index VBO and render
from there. The latter means extra overhead because of mapping and
copying, but decouples CPU and GPU and may be faster (test showed that
this can be indeed the case).

For each vertex we need to know the height of it in the next coarser
level. This can be trivially computed by requesting the height function
with a parameter for the detail one less than that of the current level.
The only problem is that every second vertex in x and y direction (thus
3/4 of all vertices) fall between vertices of coarser level. Its height
can be computed by linear combination of the neighbouring 2 or 4
vertices. Compute missing heights in rows with odd numbers first by
mixing vertex height of previous and next line 50%/50%, then compute
missing heights in all lines by mixing heights of previous/next vertex
50%/50%. The data can be looked up in the height data of the next
coarser level, if that is not only stored in the GPU but also the CPU
(system memory). Storing the data also in system memory just for that
lookup is wasteful on the other hand. We need the height data only for
updating vertex heights, which is done at a low rate per frame (most
vertex data is constant). So calling the terrain height function twice
per newly created vertex should be acceptable, and this eats less memory
and doesn't trash the cache.

We have to test if linear interpolation instead of cosine interpolation
for perlin noise generation would look good enough, because the height
values are transformed by the terrace function later anyway.
Linear interpolation is much faster.

*/








#if 0
void render_bobs(const model::mesh& sph, unsigned tm, unsigned dtm)
{
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(0, 0, -100);
	glRotatef(-60, 1, 0, 0);
	glRotatef(360.0/20 * tm/1000, 0, 0, 1);
	float a = sin(2*3.14159*0.25*tm/1000);
	float b = cos(2*3.14159*0.5*tm/1000);
	float c = 1.5*sin(2*3.14159*0.25*tm/1000+3);
	float morphfac = myfmod(0.1*tm/1000, 3.0);
	for (int y = 0; y <= 50; ++y) {
		float yf = (y-25)/5.0;
		for (int x = 0; x <= 50; ++x) {
			float xf = (x-25)/5.0;
			glPushMatrix();
			float h = a*(xf*yf*yf)+b*sin(2*3.14159*yf*(1.5+a)*0.25+a)*10.0+c*sin(2*3.14159*xf+a)*10.0;
			vector3f p0((x-25)*2, (y-25)*2, h);
			float r = 50.0*sin(3.14159*y/51);
			float beta = 2*3.14159*x/51;
			vector3f p1(r*cos(beta), r*sin(beta), (y-25)*2);
			float gamma = 2*3.14159*1*y/51;
			vector3f p2((x-25)*2*cos(gamma), (x-25)*2*sin(gamma), (y-25)*2);
			vector3f p;
			if (morphfac < 1.0f)
				p = p0*(1.0-morphfac) + p1*morphfac;
			else if (morphfac < 2.0f)
				p = p1*(2.0-morphfac) + p2*(morphfac-1.0);
			else
				p = p2*(3.0-morphfac) + p0*(morphfac-2.0);
			glTranslatef(p.x, p.y, p.z);
			sph.display();
			glPopMatrix();
		}
	}
	glPopMatrix();
}
#endif


class heightmap
{
	std::vector<float> data;
	unsigned xres, yres;
	//fixme: dimension 3 rather!
	vector2f scal, trans;
	vector2f min_coord, max_coord, area;

public:
	heightmap(const std::vector<float>& data2, unsigned rx, unsigned ry, const vector2f& s, const vector2f& t) :
		data(data2),//(rx*ry),
		xres(rx),
		yres(ry),
		scal(s),
		trans(t),
		min_coord(t),
		max_coord(vector2f(rx*s.x, ry*s.y) + t),
		area(max_coord - min_coord - vector2f(1e-3, 1e-3))
	{
	}

	/// get height with coordinate clamping and bilinear height interpolation
	float compute_height(const vector2f& coord) const;

	const std::vector<float>& heights() { return data; }
	unsigned get_xres() const { return xres; }
	unsigned get_yres() const { return yres; }
};



float heightmap::compute_height(const vector2f& coord) const
{
	// clamp
	vector2f c = coord.max(min_coord).min(max_coord) - min_coord;
	c.x = xres * c.x / area.x;
	c.y = yres * c.y / area.y;
	unsigned x = unsigned(floor(c.x));
	unsigned y = unsigned(floor(c.y));
	c.x -= x;
	c.y -= y;
	unsigned x2 = x + 1, y2 = y + 1;
	if (x2 + 1 >= xres) x2 = xres - 1;
	if (y2 + 1 >= yres) y2 = yres - 1;
	return (data[y * xres + x] * (1.0f - c.x) + data[y * xres + x2] * c.x) * (1.0f - c.y) +
		(data[y2* xres + x] * (1.0f - c.x) + data[y2* xres + x2] * c.x) * c.y;
}



class camera
{
	vector3 position;
	vector3 look_at;
public:
	camera(const vector3& p = vector3(), const vector3& la = vector3(0, 1, 0))
		: position(p), look_at(la) {}
	const vector3& get_pos() const { return position; }
	angle look_direction() const { return angle((look_at - position).xy()); }
	void set(const vector3& pos, const vector3& lookat) { position = pos; look_at = lookat; }
	matrix4 get_transformation() const;
	void set_gl_trans() const;
};



matrix4 camera::get_transformation() const
{
	// compute transformation matrix from camera
	// orientation
	// camera points to -z axis with OpenGL
	vector3 zdir = -(look_at - position).normal();
	vector3 ydir(0, 0, 1);
	vector3 xdir = ydir.cross(zdir);
	ydir = zdir.cross(xdir);
	vector3 p(xdir * position, ydir * position, zdir * position);
	return matrix4(xdir.x, xdir.y, xdir.z, -p.x,
		       ydir.x, ydir.y, ydir.z, -p.y,
		       zdir.x, zdir.y, zdir.z, -p.z,
		       0, 0, 0, 1);
}



void camera::set_gl_trans() const
{
	get_transformation().multiply_gl();
}



class canyon
{
	std::auto_ptr<model::mesh> mymesh;
	std::vector<float> heightdata;
	glsl_shader_setup myshader;
	std::auto_ptr<texture> sandrocktex;
	std::auto_ptr<texture> noisetex;
	std::auto_ptr<texture> grasstex;

	struct canyon_material : public model::material
	{
		canyon& cyn;
		canyon_material(canyon& c) : cyn(c) {}
		void set_gl_values() const;
	};
public:
	canyon(unsigned w = 256, unsigned h = 256);
	void display() const;
	/*const*/ std::vector<float>& get_heightdata() /*const*/ { return heightdata; }
};



canyon::canyon(unsigned w, unsigned h)
	: heightdata(w * h),
	  myshader(get_shader_dir() + "sandrock.vshader",
		   get_shader_dir() + "sandrock.fshader")
{
	vector<Uint8> pn = perlinnoise(w, 4, w/2).generate(); // generate_sqr(); // also looks good
	//save_pgm("canyon.pgm", w, w, &pn[0]);
	//heightdata = heightmap(w, h, vector2f(2.0f, 2.0f), vector2f(-128, -128));
	heightdata.resize(w * h);
	for (unsigned y = 0; y < h; ++y) {
		for (unsigned x = 0; x < w; ++x) {
			heightdata[y * w + x] = pn[y*w+x];
		}
	}

	// make terraces
	const unsigned height_segments = 6;
	const float total_height = 256.0;
	const float terrace_height = total_height / height_segments;
	for (unsigned y = 0; y < h; ++y) {
		for (unsigned x = 0; x < w; ++x) {
			float f = heightdata[y * w + x];
			unsigned t = unsigned(floor(f / terrace_height));
			float f_frac = f / terrace_height - t;
			float f2 = f_frac * 2.0 - 1.0; // be in -1...1 range
			// skip this for softer hills (x^3 = more steep walls)
			f2 = f2 * f2 * f2;
			f2 = asin(f2) / M_PI + 0.5; // result in 0...1 range
			heightdata[y * w + x] = (t + f2) * terrace_height;
		}
	}

	mymesh.reset(new model::mesh(w, h, heightdata, vector3f(2.0f, 2.0f, 0.5f),
				     vector3f(0.0f, 0.0f, 0.0f)));
	//fixme: only color here!
	sandrocktex.reset(new texture(get_texture_dir() + "sandrock.png", texture::LINEAR_MIPMAP_LINEAR, texture::REPEAT));
	vector<Uint8> noisevalues = perlinnoise(256, 2, 128).generate();
	noisetex.reset(new texture(noisevalues, 256, 256, GL_LUMINANCE, texture::LINEAR_MIPMAP_LINEAR, texture::REPEAT));
	grasstex.reset(new texture(get_texture_dir() + "grass.png", texture::LINEAR_MIPMAP_LINEAR, texture::REPEAT));
	mymesh->mymaterial = new canyon_material(*this);
#if 0
	mymesh->mymaterial = new model::material();
	mymesh->mymaterial->diffuse = color(32, 32, 255);
	mymesh->mymaterial->colormap.reset(new model::material::map());
	mymesh->mymaterial->colormap->set_texture(new texture(get_texture_dir() + "sandrock.jpg", texture::LINEAR_MIPMAP_LINEAR, texture::REPEAT));
	mymesh->mymaterial->specular = color(0, 0, 0);
#endif
	for (unsigned y = 0; y < h; ++y) {
		float fy = float(y)/(h-1);
		for (unsigned x = 0; x < w; ++x) {
			float fx = float(x)/(w-1);
			mymesh->texcoords[y*w+x] = vector2f((fx+sin(fy*8*2*M_PI)/32)*32 /*+fy*fx*64*/, heightdata[y*w+x]/256);
		}
	}
#if 0
	for (unsigned y = 0; y < h; ++y) {
		for (unsigned x = 0; x < w; ++x) {
			unsigned i = y * w + x;
			if (x == 0 || (x+1) == w) {
				mymesh->vertices[i].x *= 200;
			} else if (y == 0 || (y+1) == h) {
				mymesh->vertices[i].y *= 200;
			}
		}
	}
#endif
	mymesh->compile();
}



void canyon::canyon_material::set_gl_values() const
{
	cyn.myshader.use();
	cyn.myshader.set_gl_texture(*cyn.sandrocktex.get(), "texsandrock", 0);
	cyn.myshader.set_gl_texture(*cyn.noisetex.get(), "texnoise", 1);
	cyn.myshader.set_gl_texture(*cyn.grasstex.get(), "texgrass", 2);
}



void canyon::display() const
{
	mymesh->display();
	myshader.use_fixed();
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
}



class plant
{
public:
	static const unsigned nr_plant_types = 8;

	vector3f pos;
	vector2f size;
	unsigned type;

	plant(const vector3f& p, const vector2f& s, unsigned t)
		: pos(p), size(s), type(t)
	{
	}
};



struct plant_alpha_sortidx
{
	float sqd;
	unsigned idx;
	plant_alpha_sortidx() {}
	plant_alpha_sortidx(const std::vector<plant>& plants, unsigned i, const vector2f& viewpos)
		: sqd(plants[i].pos.xy().square_distance(viewpos)), idx(i) {}
	bool operator< (const plant_alpha_sortidx& other) const {
		return sqd > other.sqd;
	}
};


class plant_set
{
	// with the VBO we don't really need to store the plant vertex data...
	std::vector<plant> plants;
 	mutable vertexbufferobject plantvertexdata;
 	mutable vertexbufferobject plantindexdata;
	std::auto_ptr<texture> planttex;
	glsl_shader_setup myshader;
	unsigned vattr_treesize_idx;
	mutable std::vector<plant_alpha_sortidx> sortindices;
public:
	plant_set(vector<float>& heightdata, unsigned nr = 40000, unsigned w = 256, unsigned h = 256, const vector2f& scal = vector2f(2.0f, 2.0f));
	void display(const vector3& viewpos, float zang) const;
};


plant_set::plant_set(vector<float>& heightdata, unsigned nr, unsigned w, unsigned h, const vector2f& scal)
	: plantvertexdata(false), plantindexdata(true),
	  myshader(get_shader_dir() + "billboardtrees.vshader",
		   get_shader_dir() + "billboardtrees.fshader"),
	  vattr_treesize_idx(0)
{
	float areaw = w * scal.x, areah = h * scal.y;
	plants.reserve(nr);
	const float treeheight = 4.0;
	const float treewidth = 2.0;
	for (unsigned t = 0; t < nr; ) {
		float x = (rnd() - 0.5) * areaw;
		float y = (rnd() - 0.5) * areah;
		unsigned idxy = myclamp(unsigned((y + areah*0.5) / scal.y), 0U, h-1);
		unsigned idxx = myclamp(unsigned((x + areaw*0.5) / scal.x), 0U, w-1);
		float nz = 0.0;
		if (idxx > 0 && idxx < w-1 && idxy > 0 && idxy < h-1) {
			float hl = heightdata[idxy * w + idxx - 1];
			float hr = heightdata[idxy * w + idxx + 1];
			float hd = heightdata[idxy * w + idxx - w];
			float hu = heightdata[idxy * w + idxx + w];
			nz = vector3f(hl-hr, hd-hu, scal.x * scal.y).normal().z;
		}
		if (nz < 0.95) {
			continue;
		} else {
			++t;
		}
		float th = treeheight * rnd() * 0.25;
		float tw = treewidth * rnd() * 0.25;
		float h = heightdata[idxy * w + idxx] * 0.5;
		plants.push_back(plant(vector3f(x, y, h),
				       vector2f(treewidth + tw, treeheight + th),
				       rnd(plant::nr_plant_types)));
	}
	planttex.reset(new texture(get_texture_dir() + "plants.png", texture::LINEAR_MIPMAP_LINEAR, texture::CLAMP_TO_EDGE));

	// set up sorting indices
	sortindices.resize(plants.size());
	for (unsigned i = 0; i < plants.size(); ++i) {
		sortindices[i].idx = i;
	}

	myshader.use();
	vattr_treesize_idx = myshader.get_vertex_attrib_index("treesize");
	myshader.set_gl_texture(*planttex, "textrees", 0);
	// vertex data per plant are 4 * (3+2+1) floats (3x pos, 2x texc, 1x attr)
	plantvertexdata.init_data(4 * (3 + 2 + 1) * 4 * plants.size(), 0, GL_STATIC_DRAW);
	float* vertexdata = (float*) plantvertexdata.map(GL_WRITE_ONLY);
	for (unsigned i = 0; i < plants.size(); ++i) {
		// render each plant
		const plant& p = plants[i];
		// vertex 0
		vertexdata[6*(4*i + 0) + 0] = p.pos.x;
		vertexdata[6*(4*i + 0) + 1] = p.pos.y;
		vertexdata[6*(4*i + 0) + 2] = p.pos.z;
		vertexdata[6*(4*i + 0) + 3] = float(p.type)/plant::nr_plant_types;
		vertexdata[6*(4*i + 0) + 4] = 1.0f;
		vertexdata[6*(4*i + 0) + 5] = -p.size.x * 0.5;
		// vertex 1
		vertexdata[6*(4*i + 1) + 0] = p.pos.x;
		vertexdata[6*(4*i + 1) + 1] = p.pos.y;
		vertexdata[6*(4*i + 1) + 2] = p.pos.z;
		vertexdata[6*(4*i + 1) + 3] = float(p.type+1)/plant::nr_plant_types;
		vertexdata[6*(4*i + 1) + 4] = 1.0f;
		vertexdata[6*(4*i + 1) + 5] = p.size.x * 0.5;
		// vertex 2
		vertexdata[6*(4*i + 2) + 0] = p.pos.x;
		vertexdata[6*(4*i + 2) + 1] = p.pos.y;
		vertexdata[6*(4*i + 2) + 2] = p.pos.z + p.size.y;
		vertexdata[6*(4*i + 2) + 3] = float(p.type+1)/plant::nr_plant_types;
		vertexdata[6*(4*i + 2) + 4] = 0.0f;
		vertexdata[6*(4*i + 2) + 5] = p.size.x * 0.5;
		// vertex 3
		vertexdata[6*(4*i + 3) + 0] = p.pos.x;
		vertexdata[6*(4*i + 3) + 1] = p.pos.y;
		vertexdata[6*(4*i + 3) + 2] = p.pos.z + p.size.y;
		vertexdata[6*(4*i + 3) + 3] = float(p.type)/plant::nr_plant_types;
		vertexdata[6*(4*i + 3) + 4] = 0.0f;
		vertexdata[6*(4*i + 3) + 5] = -p.size.x * 0.5;
	}
	plantvertexdata.unmap();
	myshader.use_fixed();
}



void plant_set::display(const vector3& viewpos, float zang) const
{
	vector3f vp(viewpos);

	//unsigned tm0 = sys().millisec();
	for (unsigned i = 0; i < plants.size(); ++i) {
		sortindices[i].sqd = plants[sortindices[i].idx].pos.xy().square_distance(vp.xy());
	}
	//unsigned tm1 = sys().millisec();
	// this sucks up to 16ms, so this limits fps at 60.
	// this can't be shadowed by gpu time.
	std::sort(sortindices.begin(), sortindices.end());
	//unsigned tm2 = sys().millisec();
	//DBGOUT2(tm1-tm0,tm2-tm1);

#if 1 // indices in VBO (3fps faster)
	// index data per plant are 4 indices = 16 byte
	//fixme: why transfer this to a VBO? why not drawing these indices
	//directly from the array?!
	plantindexdata.init_data(4 * 4 * plants.size(), 0, GL_STREAM_DRAW);
	uint32_t* indexdata = (uint32_t*) plantindexdata.map(GL_WRITE_ONLY);
	for (unsigned i = 0; i < plants.size(); ++i) {
		// 4 vertices per plant
		unsigned bi = sortindices[i].idx * 4; // base index for plant i
		indexdata[4*i + 0] = bi;
		indexdata[4*i + 1] = bi+1;
		indexdata[4*i + 2] = bi+2;
		indexdata[4*i + 3] = bi+3;
	}
	plantindexdata.unmap();
#else
	vector<unsigned> pidat;
	pidat.reserve(plants.size()*4);
	for (unsigned i = 0; i < plants.size(); ++i) {
		unsigned bi = sortindices[i].idx * 4;
		pidat.push_back(bi+0);
		pidat.push_back(bi+1);
		pidat.push_back(bi+2);
		pidat.push_back(bi+3);
	}
#endif

	glActiveTexture(GL_TEXTURE0);
	planttex->set_gl_texture();
	glColor4f(1,1,1,1);
	glNormal3f(0, 0, 1);

	// fixme: cull invisible plants

	glDepthMask(GL_FALSE);
	myshader.use();
	myshader.set_uniform("viewpos", vp.xy());
	myshader.set_uniform("windmovement", myfrac(sys().millisec()/4000.0));

	plantvertexdata.bind();
	glEnableClientState(GL_VERTEX_ARRAY);
	glClientActiveTexture(GL_TEXTURE0);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glVertexPointer(3, GL_FLOAT, 6*4, (float*)0 + 0);
	glTexCoordPointer(2, GL_FLOAT, 6*4, (float*)0 + 3);
	glVertexAttribPointer(vattr_treesize_idx, 1, GL_FLOAT, GL_FALSE, 6*4, (float*)0 + 5);
	glEnableVertexAttribArray(vattr_treesize_idx);
	plantvertexdata.unbind();
#if 1 // indices in VBO (3fps faster)
	plantindexdata.bind();
	glDrawRangeElements(GL_QUADS, 0, plants.size()*4 - 1, plants.size() * 4, GL_UNSIGNED_INT, 0);
	plantindexdata.unbind();
#else
	glDrawRangeElements(GL_QUADS, 0, plants.size()*4 - 1, plants.size() * 4, GL_UNSIGNED_INT, &pidat[0]);
#endif
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableVertexAttribArray(vattr_treesize_idx);
	glDepthMask(GL_TRUE);
	myshader.use_fixed();
}



void add_tree(const vector3f& pos, float ang,
	      vector<vector3f>& vertices,
	      vector<vector2f>& texcoords,
	      vector<vector3f>& normals,
	      vector<Uint32>& indices)
{
	const float treeheight = 4.0;
	const float treewidth = 2.0;
	unsigned bi = vertices.size();
	// 10 vertices per tree
	// 48 indices per tree (16 triangles, 4 per direction, 2-sided quads)
	// form pine tree with cones? cyliner with 3 quads at bottom, 8 tris for cone
	// makes 14 tris. (verts: 8+1+1 + 2*3 at least = 16)
	// normally we should use billboarding anyway (check pbs)
	float th = treeheight * rnd() * 0.25;
	float tw = treewidth * rnd() * 0.25;
	vector3f postop = pos;
	postop.z += treeheight + th;
	vertices.push_back(postop);
	normals.push_back(vector3f(0, 0, 1));
	texcoords.push_back(vector2f(0.5, 0.0));
	for (unsigned i = 0; i <= 8; ++i) {
		angle a(ang - i * 360/8);
		vector3f pos2 = pos + (a.direction() * (treewidth + tw) * 0.5).xyz((postop.z - pos.z)*0.25);
		vertices.push_back(pos2);
		normals.push_back(a.direction().xyz(2.0f).normal());
		texcoords.push_back(vector2f(float(i)/8, 0.75));
	}
	for (unsigned i = 0; i < 8; ++i) {
		indices.push_back(bi);
		indices.push_back(bi+1+i);
		indices.push_back(bi+1+i+1);
	}
	bi = vertices.size();
	for (unsigned i = 0; i < 3; ++i) {
		angle a(ang - i * 360/3);
		vector3f pos2 = pos + (a.direction() * (treewidth + tw) * 0.1).xyz((postop.z - pos.z)*0.25);
		vertices.push_back(pos2);
		pos2.z = pos.z;
		vertices.push_back(pos2);
		normals.push_back(a.direction().xyz(2.0f).normal());
		normals.push_back(a.direction().xyz(2.0f).normal());
		texcoords.push_back(vector2f(float(i)/3, 0.75));
		texcoords.push_back(vector2f(float(i)/3, 1.0));
	}
	for (unsigned i = 0; i < 3; ++i) {
		indices.push_back(bi+2*i);
		indices.push_back(bi+2*i+1);
		indices.push_back(bi+2*((i+1)%3));
		indices.push_back(bi+2*((i+1)%3));
		indices.push_back(bi+2*i+1);
		indices.push_back(bi+2*((i+1)%3)+1);
	}

#if 0
	vector3f postop = pos;
	postop.z += treeheight + th;
	vertices.push_back(pos);
	vertices.push_back(postop);
	normals.push_back(vector3f(0, 0, 1));
	normals.push_back(vector3f(0, 0, 1));
	texcoords.push_back(vector2f(1.0, 1.0));
	texcoords.push_back(vector2f(1.0, 0.0));
	for (unsigned i = 0; i < 4; ++i) {
		angle a(ang + 90 * i);
		vector3f pos2 = pos + (a.direction() * (treewidth + tw) * 0.5).xy0();
		vector3f postop2 = pos2;
		postop2.z += treeheight + th;
		vertices.push_back(pos2);
		vertices.push_back(postop2);
		normals.push_back(vector3f(0, 0, 1));
		normals.push_back(vector3f(0, 0, 1));
		texcoords.push_back(vector2f(0.0, 1.0));
		texcoords.push_back(vector2f(0.0, 0.0));
		// side one
		indices.push_back(bi+2+i*2);
		indices.push_back(bi      );
		indices.push_back(bi+3+i*2);
		indices.push_back(bi+3+i*2);
		indices.push_back(bi      );
		indices.push_back(bi+1    );
		// side two
		indices.push_back(bi      );
		indices.push_back(bi+2+i*2);
		indices.push_back(bi+1    );
		indices.push_back(bi+1    );
		indices.push_back(bi+2+i*2);
		indices.push_back(bi+3+i*2);
	}
#endif
}



auto_ptr<model::mesh> generate_trees(vector<float>& heightdata, unsigned nr = 20000, unsigned w = 256, unsigned h = 256, const vector2f& scal = vector2f(2.0f, 2.0f))
{
	float areaw = w * scal.x, areah = h * scal.y;
	auto_ptr<model::mesh> m(new model::mesh("trees"));
	for (unsigned t = 0; t < nr; ) {
		float x = (rnd() - 0.5) * areaw;
		float y = (rnd() - 0.5) * areah;
		unsigned idxy = myclamp(unsigned((y + areah*0.5) / scal.y), 0U, h-1);
		unsigned idxx = myclamp(unsigned((x + areaw*0.5) / scal.x), 0U, w-1);
		float nz = 0.0;
		if (idxx > 0 && idxx < w-1 && idxy > 0 && idxy < h-1) {
			float hl = heightdata[idxy * w + idxx - 1];
			float hr = heightdata[idxy * w + idxx + 1];
			float hd = heightdata[idxy * w + idxx - w];
			float hu = heightdata[idxy * w + idxx + w];
			nz = vector3f(hl-hr, hd-hu, scal.x * scal.y).normal().z;
		}
		if (nz < 0.95) {
			continue;
		} else {
			++t;
		}
		float h = heightdata[idxy * w + idxx] * 0.5;
		add_tree(vector3f(x, y, h), rnd()*90, m->vertices, m->texcoords, m->normals, m->indices);
	}
	m->mymaterial = new model::material();
	m->mymaterial->colormap.reset(new model::material::map());
//	m->mymaterial->colormap->set_texture(new texture(get_texture_dir() + "tree1.png", texture::LINEAR_MIPMAP_LINEAR, texture::CLAMP_TO_EDGE));
	m->mymaterial->specular = color(0, 0, 0);
	m->compile();
	return m;
}



void generate_fadein_pixels(vector<Uint8>& pix, unsigned ctr, unsigned s)
{
	// draw spiral
	unsigned x = 0, y = 0;
	const int dx[4] = { 1, 0,-1, 0 };
	const int dy[4] = { 0, 1, 0,-1 };
	unsigned i = 0;
	for (unsigned m = s; m > 0; m -= 2) {
		for (unsigned k = 0; k < 4; ++k) {
			for (unsigned j = 0; j < m - 1; ++j) {
				pix[2*(y*s+x)+1] = (i < ctr) ? 0x00 : 0xff;
				x += dx[k];
				y += dy[k];
				++i;
			}
		}
		// we need to go down and right one cell
		x += 1;
		y += 1;
	}
}


template <class T, unsigned size>
class lookup_function
{
	typename std::vector<T> values;
	float dmin, dmax, drange_rcp;
public:
	lookup_function(float dmin_ = 0.0f, float dmax_ = 1.0f)
		: values(size + 2), dmin(dmin_), dmax(dmax_), drange_rcp(1.0f/(dmax_ - dmin_)) {}
	void set_value(unsigned idx, T v) {
		values.at(idx) = v;
		// duplicate last value (avoid the if (idx == size), its faster to
		// just do it.
		values[size+1] = values[size];
	}
	T value(float f) const {
		if (f < dmin) f = dmin;
		if (f > dmax) f = dmax;
		// note: if drange_rcp is a bit too large (float is unprecise)
		// the result could be a bit larger than 1.0f * size
		// which is no problem when its smaller than 1.0 + 1/(size+1)
		// which is normally the case. to avoid segfaults we just
		// make "values" one entry bigger and duplicate the last value.
		return values[unsigned(size * ((f - dmin) * drange_rcp))];
	}
	unsigned get_value_range() const { return size + 1; }
};

void show_credits()
{
	//glClearColor(0.1,0.25,0.4,0);
	glClearColor(0.175,0.25,0.125,0.0);

#if 0
	{
		// 2^5 * 256 detail is enough = 8192
		// with 1 value per meter -> repeat every 8192m, far enough
		perlinnoise pn(64, 4, 6, true); // max. 8192
		//const unsigned s = 256*256;
		const unsigned s2 = 256*16;
		//const unsigned s3 = 32;
		const unsigned height_segments = 10;
		const float total_height = 256.0;
		const float terrace_height = total_height / height_segments;
		std::vector<Uint8> heights = pn.values(0, 0, s2, s2, 6);
		lookup_function<float, 256U> asin_lookup;
		for (unsigned i = 0; i <= 256; ++i)
			asin_lookup.set_value(i, asin(float(i)/256) / M_PI + 0.5);
#if 1
		for (unsigned y = 0; y < s2; ++y) {
			for (unsigned x = 0; x < s2; ++x) {
				float f = heights[y*s2+x];
				unsigned t = unsigned(floor(f / terrace_height));
				float f_frac = f / terrace_height - t;
				float f2 = f_frac * 2.0 - 1.0; // be in -1...1 range
				// skip this for softer hills (x^3 = more steep walls)
				f2 = f2 * f2 * f2;
				f2 = asin_lookup.value(f2);
				heights[y*s2+x] = Uint8((t + f2) * terrace_height);
			}
		}
#endif
		save_pgm("pntest.pgm", s2, s2, &heights[0]);
	}
#endif

	//srand(0);

#if 0
	// create a sphere
	auto_ptr<model::mesh> sph;
	sph.reset(make_mesh::sphere(1.0f, 2.0f, 8, 8, 1, 1, true, "sun"));
	sph->mymaterial = new model::material();
	sph->mymaterial->diffuse = color(32, 32, 255);
	sph->mymaterial->specular = color(255, 255, 255);
	sph->compile();
#endif

	vector3 viewpos(0, 0, 64);

	vector<float> heightdata;
	canyon cyn(256, 256);
	heightmap chm(cyn.get_heightdata(), 256, 256, vector2f(2.0f, 2.0f), vector2f(-256, -256));
	auto_ptr<model::mesh> trees = generate_trees(cyn.get_heightdata());
	plant_set ps(cyn.get_heightdata());
	auto_ptr<sky> mysky(new sky(8*3600.0)); // 10 o'clock
	vector3 sunpos(0, 3000, 4000);
	mysky->rebuild_colors(sunpos, vector3(-500, -3000, 1000), viewpos);

	// compute bspline for camera path
	std::vector<vector3f> bsppts;
	vector2f bsp[13] = {
		vector2f( 0.00,  0.75),
		vector2f( 0.75,  0.75),
		vector2f( 0.75,  0.00),
		vector2f( 0.00,  0.00),
		vector2f(-0.75,  0.00),
		vector2f(-0.75, -0.75),
		vector2f( 0.00, -0.75),
		vector2f( 0.75, -0.75),
		vector2f( 0.75,  0.00),
		vector2f( 0.00,  0.00),
		vector2f(-0.75,  0.00),
		vector2f(-0.75,  0.75),
		vector2f( 0.00,  0.75)
	};
	for (unsigned j = 0; j < 13; ++j) {
		vector2f a = bsp[j] * 256;
		bsppts.push_back(a.xyz(chm.compute_height(a) * 0.5 + 20.0));
	}
	bsplinet<vector3f> cam_path(2, bsppts);

// we need a function to get height at x,y coord
// store xy res of height map too
// 	const float flight_wh(128, 128);
// 	vector<vector3f> flight_coords;
// 	flight_coords.push_back(vector3f(0, -flight_wh*0.75));
// fly an 8 like figure

	auto_ptr<texture> bkg;
	auto_ptr<glsl_shader_setup> glss;
	bool use_shaders = glsl_program::supported();
#if 0
	if (use_shaders) {
		const unsigned sz = 16;
		vector<Uint8> data(sz*sz);
		for (unsigned y = 0; y < sz; ++y)
			for (unsigned x = 0; x < sz; ++x)
				data[y*sz+x] = rand() & 0xff;
		bkg.reset(new texture(data, sz, sz, GL_LUMINANCE, texture::LINEAR, texture::REPEAT));
		glss.reset(new glsl_shader_setup(get_shader_dir() + "credits.vshader",
						 get_shader_dir() + "credits.fshader"));
	}
#endif

	int lineheight = font_arial->get_height();
	int lines_per_page = (768+lineheight-1)/lineheight;
	int textpos = -lines_per_page;
	int textlines = 0;
	for ( ; credits[textlines] != 0; ++textlines);
	int textendpos = textlines;
	float lineoffset = 0.0f;

	//float lposition[4] = {200, 0, 0, 1};

	std::auto_ptr<texture> fadein_tex;
	std::vector<Uint8> fadein_pixels(8*8*2);
	generate_fadein_pixels(fadein_pixels, 0, 8);
	fadein_tex.reset(new texture(fadein_pixels, 8, 8, GL_LUMINANCE_ALPHA,
				     texture::NEAREST, texture::REPEAT));
	unsigned fadein_ctr = 0;

	bool quit = false;
	float lines_per_sec = 2;
	float ctr = 0.0, ctradd = 1.0/32.0;
	unsigned tm = sys().millisec();
	unsigned tm0 = tm;
	unsigned frames = 1;
	unsigned lastframes = 1;
	double fpstime = sys().millisec() / 1000.0;
	double totaltime = sys().millisec() / 1000.0;
	double measuretime = 5;	// seconds
	while (!quit) {
		list<SDL_Event> events = sys().poll_event_queue();
		for (list<SDL_Event>::iterator it = events.begin(); it != events.end(); ++it) {
			if (it->type == SDL_KEYDOWN) {
				quit = true;
			} else if (it->type == SDL_MOUSEBUTTONUP) {
				quit = true;
			}
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#if 0
		// render sphere bobs
		render_bobs(*sph, sys().millisec() - tm0, sys().millisec() - tm);
#endif

		glColor4f(1,1,1,1);
		glPushMatrix();
		glLoadIdentity();
		float zang = 360.0/40 * (sys().millisec() - tm0)/1000;
		float zang2 = 360.0/200 * (sys().millisec() - tm0)/1000;
		vector3 viewpos2 = viewpos + (angle(-zang2).direction() * 192).xy0();
		float terrainh = chm.compute_height(vector2f(viewpos2.x, viewpos2.y));
		viewpos2.z = terrainh * 0.5 + 20.0; // fixme, heightmap must take care of z scale

		float path_fac = myfrac((1.0/120) * (sys().millisec() - tm0)/1000);
		vector3f campos = cam_path.value(path_fac);
		vector3f camlookat = cam_path.value(myfrac(path_fac + 0.01));
		//camera cm(viewpos2, viewpos2 + angle(zang).direction().xyz(-0.25));
		camera cm(campos, camlookat);
		zang = cm.look_direction().value();
		cm.set_gl_trans();

		// sky also sets light source position
		mysky->display(colorf(1.0f, 1.0f, 1.0f), viewpos2, 30000.0, false);
		//glDisable(GL_FOG);
		glFogi(GL_FOG_MODE, GL_EXP);
		float fog_color[4] = { 0.6, 0.6, 0.6, 1.0 };
		glFogfv(GL_FOG_COLOR, fog_color);
		glFogf(GL_FOG_DENSITY, 0.0008);

		// render canyon
		cyn.display();
		//trees->display();
		ps.display(campos, zang);//viewpos2 here, but it flickers

		glPopMatrix();

		sys().prepare_2d_drawing();
		if (fadein_tex.get()) {
			fadein_ctr = (sys().millisec() - tm0) * 64 / 3200;
			// generate fadein tex
			generate_fadein_pixels(fadein_pixels, fadein_ctr, 8);
			fadein_tex.reset(new texture(fadein_pixels, 8, 8, GL_LUMINANCE_ALPHA,
						     texture::NEAREST, texture::REPEAT));
			glPushMatrix();
			glScalef(4.0, 4.0, 4.0);
			fadein_tex->draw_tiles(0, 0, sys().get_res_x_2d()/4, sys().get_res_y_2d()/4);
			glPopMatrix();
			if (fadein_ctr >= 64)
				fadein_tex.reset();
		}

#if 0
		if (use_shaders) {
			glss->use();
			glColor4f(0.2,0.8,1,1);
			bkg->set_gl_texture();
			glBegin(GL_QUADS);
			glTexCoord2f(0+ctr,0);
			glMultiTexCoord2f(GL_TEXTURE1_ARB, ctr, ctr*0.5);
			glVertex2i(0,768);
			glTexCoord2f(1.33333+ctr,0);
			glMultiTexCoord2f(GL_TEXTURE1_ARB, ctr, ctr*0.5);
			glVertex2i(1024,768);
			glTexCoord2f(1.33333+ctr,1);
			glMultiTexCoord2f(GL_TEXTURE1_ARB, ctr, ctr*0.5);
			glVertex2i(1024,0);
			glTexCoord2f(0+ctr,1);
			glMultiTexCoord2f(GL_TEXTURE1_ARB, ctr, ctr*0.5);
			glVertex2i(0,0);
			glEnd();
			glBindTexture(GL_TEXTURE_2D, 0);
			glss->use_fixed();
		}
#endif

		for (int i = textpos; i <= textpos+lines_per_page; ++i) {
			if (i >= 0 && i < textlines) {
				int y = (i-textpos)*lineheight+int(-lineoffset*lineheight);
				font_arial->print_hc(512+int(64*sin(y*2*M_PI/640)), y, credits[i], color::white(), true);
			}
		}
		sys().unprepare_2d_drawing();

		unsigned tm2 = sys().millisec();
		lineoffset += lines_per_sec*(tm2-tm)/1000.0f;
		int tmp = int(lineoffset);
		lineoffset -= tmp;
		textpos += tmp;
		if (textpos >= textendpos) textpos = -lines_per_page;
		ctr += ctradd * (tm2-tm)/1000.0f;
		tm = tm2;

		// record fps
		++frames;
		totaltime = tm2 / 1000.0;
		if (totaltime - fpstime >= measuretime) {
			fpstime = totaltime;
			log_info("fps " << (frames - lastframes)/measuretime);
			lastframes = frames;
		}

		sys().swap_buffers();
	}
	
	glClearColor(0, 0, 1, 0);
}
