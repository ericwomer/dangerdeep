// A 3d model (C) Thorsten Jordan

#ifndef MODEL_H
#define MODEL_H

#include "vector3.h"
#include "texture.h"
#include "color.h"
#include <vector>
using namespace std;

class model
{
protected:
	vector<vector3> vcoords, vnormals;
	vector<vector2> vtexcoords;
	vector<unsigned> facevalues;	// 3 values per face
	texture* tex;
	// write floats in [-2,...2) to a short int
	static float read_packed_float(FILE* f);
	static void write_packed_float(FILE* f, float t);
	static float read_quantified_float(FILE* f, double min, double max);
	static void write_quantified_float(FILE* f, float t, double min, double max);
	vector3 min, max;
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
