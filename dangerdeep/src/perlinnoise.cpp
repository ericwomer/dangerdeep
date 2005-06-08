// perlin 2d noise generator
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "perlinnoise.h"
#include "system.h"
#include <math.h>

#include <sstream>
#include <fstream>

perlinnoise_generator::noise_func::noise_func(unsigned s, unsigned f, float px, float py)
	: size(s), frequency(f), phasex(px), phasey(py)
{
	data.resize(size*size);
	unsigned base = rand();
	for (unsigned i = 0; i < size * size; ++i) {
		data[i] = Sint8(base >> 24);
		base = base * (base * 15731 + 789221) + 1376312589;
	}
#if 0
	ostringstream osgname;
        osgname << "noisefct" << s << ".pgm";
        ofstream osg(osgname.str().c_str());
        osg << "P5\n"<<s<<" "<<s<<"\n255\n";
        osg.write((const char*)(&data[0]), s*s);
#endif
}



inline Sint8 perlinnoise_generator::noise_func::interpolate(const vector<float>& interpolation_func, float x, float y) const
{
	// map x,y to coordinates inside function (0 <= x,y < 1)
	float bx = phasex + x;
	float by = phasey + y;
	bx -= floorf(bx);
	by -= floorf(by);
	// remap to value/subvalue coordinates
	bx *= size * frequency;
	by *= size * frequency;
	unsigned sz1 = size - 1;
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
	return Sint8(data[y1*size+x1] * a1 * b1 +
		     data[y1*size+x2] * a2 * b1 +
		     data[y2*size+x1] * a1 * b2 +
		     data[y2*size+x2] * a2 * b2);
}



bool is_power2(unsigned x)
{
	return (x & (x-1)) == 0;
}

perlinnoise_generator::perlinnoise_generator(unsigned size, unsigned sizeminfreq, unsigned sizemaxfreq)
	: resultsize(size)
{
	sys().myassert(is_power2(size), "size is not power of two");
	sys().myassert(is_power2(sizeminfreq), "sizeminfreq is not power of two");
	sys().myassert(is_power2(sizemaxfreq), "sizemaxfreq is not power of two");
	sys().myassert(sizeminfreq >= 1 && sizeminfreq <= size && sizeminfreq <= sizemaxfreq, "sizeminfreq out of range");
	sys().myassert(sizemaxfreq >= 1 && sizemaxfreq <= size, "sizemaxfreq out of range");
	unsigned nrfunc = 0;
	for (unsigned j = sizemaxfreq/sizeminfreq; j > 0; j >>= 1)
		++nrfunc;
	noise_functions.reserve(nrfunc);
	for (unsigned i = 0; i < nrfunc; ++i)
		noise_functions.push_back(noise_func(sizeminfreq<<i, 1 /*<<i*/));

	// create interpolation function
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



void perlinnoise_generator::set_phase(unsigned func, float px, float py)
{
	if (func < noise_functions.size()) {
		noise_functions[func].phasex = px;
		noise_functions[func].phasey = py;
	}
}


static inline Sint32 clamp_zero(Sint32 x) { return x & ~(x >> 31); }
static inline Sint32 clamp_value(Sint32 x, Sint32 val) { return val - clamp_zero(val - x); }
perlinnoise perlinnoise_generator::generate_map() const
{
	perlinnoise result;
	result.size = resultsize;
	result.noisemap.resize(resultsize * resultsize);
	float dxy = 1.0f/resultsize;
	unsigned ptr = 0;
	float fy = 0;
	for (unsigned y = 0; y < resultsize; ++y) {
		float fx = 0;
		for (unsigned x = 0; x < resultsize; ++x) {
			int sum = 0;
			for (unsigned i = 0; i < noise_functions.size(); ++i) {
				sum += noise_functions[i].interpolate(interpolation_func, fx, fy) >> i;
			}
			// sum is at most around +- 207, so we multiply with 19/32, to get in in +-127 range
			// to be sure we clamp it also.
			sum = clamp_value(clamp_zero(((sum * 19) >> 5) + 128), 255);
			result.noisemap[ptr++] = Uint8(sum);
			fx += dxy;
		}
		fy += dxy;
	}

	return result;
}
