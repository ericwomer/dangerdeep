// perlin 2d noise generator
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "perlinnoise.h"
#include <math.h>


perlinnoise_generator::noise_func::noise_func(unsigned s, unsigned f, unsigned a, float px, float py)
	: size(s), frequency(f), amplitude(a), phasex(px), phasey(py)
{
	unsigned sz = 1<<size;
	vector<float> tmp(sz*sz);
	// generate noise function
	for (unsigned xy = 0; xy < tmp.size(); ++xy) {
		tmp[xy] = float(rand()) / float(RAND_MAX);
	}
	// smooth and equalize it
	float vmax = -1e10, vmin = 1e10;
	vector<float> tmp2(tmp.size());
	for (unsigned y = 0; y < sz; ++y) {
		unsigned y1 = (y + sz - 1) & (sz - 1);
		unsigned y2 = (y + 1) & (sz - 1);
		for (unsigned x = 0; x < sz; ++x) {
			unsigned x1 = (x + sz - 1) & (sz - 1);
			unsigned x2 = (x + 1) & (sz - 1);
			float sum =
				(tmp[y1*sz+x1] + tmp[y1*sz+x2] +
				 tmp[y2*sz+x1] + tmp[y2*sz+x2]) * (1.0f/16) +
				(tmp[y1*sz+x] + tmp[y*sz+x1] +
				 tmp[y2*sz+x] + tmp[y*sz+x2]) * (1.0f/8) +
				tmp[y*sz+x] * (1.0f/4);
			tmp2[y*sz+x] = sum;
			if (sum > vmax) vmax = sum;
			if (sum < vmin) vmin = sum;
		}
	}
	// equalize
	data.resize(tmp2.size());
	float vd = 255.0f/(vmax - vmin);
	for (unsigned xy = 0; xy < tmp2.size(); ++xy) {
		data[xy] = Uint8((tmp2[xy] - vmin) * vd);
	}
}



inline float perlinnoise_generator::noise_func::interpolate(const vector<float>& interpolation_func, float x, float y) const
{
	// map x,y to coordinates inside function (0 <= x,y < 1)
	float bx = phasex + x;
	float by = phasey + y;
	bx -= floorf(bx);
	by -= floorf(by);
	// remap to value/subvalue coordinates
	unsigned sz = 1<<size;
	unsigned sz1 = sz - 1;
	bx *= sz * frequency;
	by *= sz * frequency;
	unsigned x1 = unsigned(bx) & sz1;
	unsigned y1 = unsigned(by) & sz1;
	unsigned x2 = (x1 + 1) & sz1;
	unsigned y2 = (y1 + 1) & sz1;
	float xf = bx - floorf(bx);
	float yf = by - floorf(by);
	float a2 = interpolation_func[unsigned(interpolation_func.size()*xf)];
	float b2 = interpolation_func[unsigned(interpolation_func.size()*yf)];
	float a1 = 1.0f - a2;
	float b1 = 1.0f - b2;
	return	(data[y1*sz+x1] * a1 * b1 +
		 data[y1*sz+x2] * a2 * b1 +
		 data[y2*sz+x1] * a1 * b2 +
		 data[y2*sz+x2] * a2 * b2) * amplitude;
}



perlinnoise_generator::perlinnoise_generator()
{
	const unsigned res = 256;
	interpolation_func.resize(res);
	for (unsigned i = 0; i < res; ++i) {
/*
		interpolation_func[i] = (i < res/2) ? 0 : 1;
*/
/*
		interpolation_func[i] = float(i)/res;
*/
		float f = M_PI * float(i)/res;
		interpolation_func[i] = (1.0f - cosf(f)) * 0.5f;
	}
}



void perlinnoise_generator::add_noise_func(const noise_func& nf)
{
	noise_functions.push_back(nf);
}



void perlinnoise_generator::set_phase(unsigned func, float px, float py)
{
	if (func < noise_functions.size()) {
		noise_functions[func].phasex = px;
		noise_functions[func].phasey = py;
	}
}



perlinnoise perlinnoise_generator::generate_map(unsigned s) const
{
	perlinnoise result;
	unsigned size = 1<<s;
	result.size = size;
	result.noisemap.resize(size * size);
	vector<float> tmp(size*size);
	float dxy = 1.0f/size;
	float vmin = 1e10, vmax = -1e10;
	unsigned ptr = 0;
	float fy = 0;
	for (unsigned y = 0; y < size; ++y) {
		float fx = 0;
		for (unsigned x = 0; x < size; ++x) {
			float sum = 0;
			for (unsigned i = 0; i < noise_functions.size(); ++i) {
				sum += noise_functions[i].interpolate(interpolation_func, fx, fy);
			}
			if (sum < vmin) vmin = sum;
			if (sum > vmax) vmax = sum;
			tmp[ptr++] = sum;
			fx += dxy;
		}
		fy += dxy;
	}

	// normalize result
	float vd = 255.9f/(vmax - vmin);
	for (unsigned j = 0; j < size*size; ++j) {
		result.noisemap[j] = Uint8((tmp[j] - vmin) * vd);
	}

	return result;
}
