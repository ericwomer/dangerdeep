// perlin 2d noise generator
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "perlinnoise.h"
#include <math.h>

perlinnoise_generator::perlinnoise_generator(unsigned d, unsigned sz) : noise_func(d), size(sz)
{
	for (unsigned i = 0; i < d; ++i) {
		noise_func[i] = make_noise_func();
	}
	make_interpolation_func(256);
}


void perlinnoise_generator::make_interpolation_func(unsigned res)
{
	interpolation_func.resize(res);
	for (unsigned i = 0; i < res; ++i) {
		float f = M_PI * float(i)/res;
		interpolation_func[i] = (1.0f - cosf(f)) * 0.5f;
	}
}

float perlinnoise_generator::interpolate(unsigned d, float x, float y) const
{
	const vector<uint8_t>& nf = noise_func[d];
	unsigned x1 = unsigned(x) & (size - 1);
	unsigned y1 = unsigned(y) & (size - 1);
	unsigned x2 = (x1 + 1) & (size - 1);
	unsigned y2 = (y1 + 1) & (size - 1);
	float xf = x - floorf(x);
	float yf = y - floorf(y);
	float a2 = interpolation_func[unsigned(interpolation_func.size()*xf)];
	float b2 = interpolation_func[unsigned(interpolation_func.size()*yf)];
	float a1 = 1.0f - a2;
	float b1 = 1.0f - b2;
	return	nf[y1*size+x1] * a1 * b1 +
		nf[y1*size+x2] * a2 * b1 +
		nf[y2*size+x1] * a1 * b2 +
		nf[y2*size+x2] * a2 * b2;
}

vector<uint8_t> perlinnoise_generator::make_noise_func(void)
{
	vector<float> tmp(size * size);
	// generate noise function
	for (unsigned xy = 0; xy < size*size; ++xy) {
		tmp[xy] = float(rand()) / float(RAND_MAX);
	}
	// smooth and equalize it
	float vmax = -1e10, vmin = 1e10;
	vector<float> tmp2(size * size);
	for (unsigned y = 0; y < size; ++y) {
		unsigned y1 = (y + size - 1) & (size - 1);
		unsigned y2 = (y + 1) & (size - 1);
		for (unsigned x = 0; x < size; ++x) {
			unsigned x1 = (x + size - 1) & (size - 1);
			unsigned x2 = (x + 1) & (size - 1);
			float sum =
				(tmp[y1*size+x1] + tmp[y1*size+x2] +
				 tmp[y2*size+x1] + tmp[y2*size+x2]) * (1.0f/16) +
				(tmp[y1*size+x] + tmp[y*size+x1] +
				 tmp[y2*size+x] + tmp[y*size+x2]) * (1.0f/8) +
				tmp[y*size+x] * (1.0f/4);
			tmp2[y*size+x] = sum;
			if (sum > vmax) vmax = sum;
			if (sum < vmin) vmin = sum;
		}
	}
	// equalize
	vector<uint8_t> result(size*size);
	float vd = 255.0f/(vmax - vmin);
	for (unsigned xy = 0; xy < size*size; ++xy) {
		result[xy] = uint8_t((tmp2[xy] - vmin) * vd);
	}
	return result;
}

vector<uint8_t> perlinnoise_generator::generate_level(unsigned d, unsigned s) const
{
	vector<uint8_t> result(s * s);
	// noise function number d is tiled (d+1) times over the result
	// this means w,h must be larger than size, size
	float dx = float((1 << d) * size) / s;
	float dy = float((1 << d) * size) / s;
	float yy = 0;
	for (unsigned y = 0; y < s; ++y) {
		float xx = 0;
		for (unsigned x = 0; x < s; ++x) {
			result[y*s+x] = uint8_t(interpolate(d, xx, yy));
			xx += dx;
		}
		yy += dy;
	}
	return result;
}

perlinnoise perlinnoise_generator::generate_map(unsigned s) const
{
	perlinnoise result;
	result.size = s;
	result.noisemap.resize(s * s);
	for (unsigned i = 0; i < noise_func.size(); ++i) {
		vector<uint8_t> lv = generate_level(i, s);
		for (unsigned j = 0; j < s*s; ++j) {
			result.noisemap[j] += (lv[j] >> (i+1));
		}
	}
	return result;
}
