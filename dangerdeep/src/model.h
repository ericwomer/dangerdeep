// A 3d model (C) Thorsten Jordan

#ifndef MODEL_H
#define MODEL_H

#include "vector3.h"
#include "texture.h"
#include "color.h"
#include <vector>
using namespace std;

class model	// fixme: this is a single textured mesh.
		// rename it to mesh and create model class that can
		// contain a vector/list of meshes (for improved models)
{
public:
	// the elements of this struct must be packed (use gcc -fpack-struct?)
	// so that OpenGl interleaved arrays can be used.
	// this struct is 32 bytes in size (2+3+3)*2 so it it already "packed"
	struct vertex {
		vector2f uv;
		vector3f normal;
		vector3f pos;
		vertex() {}
		~vertex() {}
		vertex(const vector2f& t, const vector3f& n, const vector3f& p) : uv(t), normal(n), pos(p) {}
		vertex(const vertex& o) : uv(o.uv), normal(o.normal), pos(o.pos) {}
		vertex& operator= (const vertex& o) { uv = o.uv; normal = o.normal; pos = o.pos; return *this; }
	};
	struct face {
		unsigned v[3];
		face(unsigned a, unsigned b, unsigned c) { v[0] = a; v[1] = b; v[2] = c; }
	};
protected:
	vector<vertex> vertices;
	vector<face> faces;
	texture* tex;
	// write floats in [-2,...2) to a short int
	static float read_packed_float(FILE* f);
	static void write_packed_float(FILE* f, float t);
	static float read_quantified_float(FILE* f, double min, double max);
	static void write_quantified_float(FILE* f, float t, double min, double max);
	vector3f min, max;
	unsigned texmapping;	// which mapping to use (nearest, bilinear, mipmap, trilinear)
	bool clamp;		// clamp texture?

	model(const model& other);
	model& operator=(const model& other);
public:
	model() : tex(0), texmapping(0) {};
	~model() { delete tex; };
	model(const string& filename);
	void read(const string& filename);
	// use an empty texture name for textureless model
	void read_from_OFF(const string& filename, const string& texture_name,
		unsigned mapping = 0, unsigned tilesx = 1, unsigned tilesy = 1);
	void write(const string& filename) const;
	void display(bool with_texture = true) const;
	double get_length(void) const { return (max - min).y; };
	double get_width(void) const { return (max - min).x; };
};

#endif
