// perlin 2d noise generator
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef PERLINNOISE_H
#define PERLINNOISE_H

#ifndef M_PI
#define M_PI 3.1415927
#endif

typedef unsigned char Uint8;

#include <stdlib.h>
#include <vector>
using std::vector;



struct perlinnoise
{
	vector<Uint8> noisemap;
	unsigned size;	// width/height, map is quadratic
};	



class perlinnoise_generator
{
public:
	// quadratic noise
	struct noise_func
	{
		vector<Uint8> data;
		unsigned size;		// in powers of two
		unsigned frequency;	// 1-x
		unsigned amplitude;	// 0-255
		float phasex, phasey;
		// create random noise function
		noise_func(unsigned s, unsigned f, unsigned a, float px = 0.0f, float py = 0.0f);
		// interpolate noise function value
		inline float interpolate(const vector<float>& interpolation_func, float x, float y) const;
	};

protected:
	vector<noise_func> noise_functions;

	vector<float> interpolation_func;

public:
	perlinnoise_generator();

	// register noise function
	void add_noise_func(const noise_func& nf);

	// set phase of function
	void set_phase(unsigned func, float px, float py);

	// generate a composition of the noise functions, give total size (power of two), e.g. 8
	perlinnoise generate_map(unsigned s) const;
};

#endif
