// perlin 2d noise generator
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef PERLINNOISE_H
#define PERLINNOISE_H

#ifndef M_PI
#define M_PI 3.1415927
#endif

typedef unsigned char uint8_t;

#include <stdlib.h>
#include <vector>
using std::vector;



struct perlinnoise
{
	vector<uint8_t> noisemap;
	unsigned size;	// width/height, map is quadratic
};	



class perlinnoise_generator
{
	vector<vector<uint8_t> > noise_func;
	unsigned size;

	vector<float> interpolation_func;

	void make_interpolation_func(unsigned res);
	inline float interpolate(unsigned d, float x, float y) const;
	vector<uint8_t> make_noise_func(void);
public:
	// all widths and heights must be powers of two!
	// give number of functions to generate and width and height (=size) of them
	perlinnoise_generator(unsigned depth = 4, unsigned sz = 32);
	// generate a composition of the noise functions, give total size
	perlinnoise generate_map(unsigned s) const;
	// generate the image of one noise function, give its number and target w/h (size)
	// target w,h must be larger than width/height
	vector<uint8_t> generate_level(unsigned d, unsigned s) const;
};

#endif
