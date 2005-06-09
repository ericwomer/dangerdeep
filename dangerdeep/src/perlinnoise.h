// perlin 2d noise generator
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef PERLINNOISE_H
#define PERLINNOISE_H

#ifndef M_PI
#define M_PI 3.1415927
#endif

#include "fixed.h"

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
		fixed32 phasex;
		fixed32 phasey;
		// create random noise function
		noise_func(unsigned s, unsigned f, float px = 0.0f, float py = 0.0f);

		// interpolate noise function value
		mutable unsigned offsetline1, offsetline2;
		mutable fixed32 linefac1;
		mutable fixed32 linefac2;
		void set_line_for_interpolation(const vector<fixed32>& interpolation_func, fixed32 y) const;
		Uint8 interpolate(const vector<fixed32>& interpolation_func, fixed32 x) const;
	};

protected:
	vector<noise_func> noise_functions;
	unsigned resultsize;

	vector<fixed32> interpolation_func;

public:
	// give size of result (power of two), size of noise function with minimal frequency and maximum frequency
	// sizeminfreq is usually very small, 2 or 4 at least, at most the same as size, at least 1
	// sizemaxfreq is usually very high, at most the same as size
	perlinnoise_generator(unsigned size, unsigned sizeminfreq, unsigned sizemaxfreq);

	// get number of functions/levels
	unsigned get_number_of_levels() const { return noise_functions.size(); }

	// set phase of a level
	void set_phase(unsigned level, float px, float py);

	// generate a composition of the noise functions
	perlinnoise generate_map() const;
};

#endif
