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
public:
	struct vertex {
		vector3 pos, normal;
		float u, v;
	};
	struct face {
		unsigned v[3];
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
	vector3 min, max;
	unsigned texmapping;

	model(const model& other);
	model& operator=(const model& other);
public:
	model() : tex(0), texmapping(0) {};
	~model() { delete tex; };
	model(const string& filename);
	void read(const string& filename);
	// use an empty texture name for textureless model
	void read_from_OFF(const string& filename, const string& texture_name, unsigned mapping = 0);
	void write(const string& filename) const;
	// if one color is given it is used for the whole model.
	// if two colors are given a gradient is computed along the z axis with col1 at bottom.
	void display(bool with_texture = true, color* col1 = 0, color* col2 = 0) const;
	void scale(double x, double y, double z);
	double get_length(void) const { return (max - min).y; };
	double get_width(void) const { return (max - min).x; };
};

#endif
