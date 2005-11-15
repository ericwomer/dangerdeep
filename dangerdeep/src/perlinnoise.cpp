// perlin 2d noise generator
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "perlinnoise.h"
#include "system.h"
#include <math.h>

#include <sstream>
#include <fstream>

perlinnoise::noise_func::noise_func(unsigned s, unsigned f, float px, float py)
	: size(s), frequency(f), phasex(fixed32(px)), phasey(fixed32(py))
{
	data.resize(size*size);
	unsigned base = rand();
	for (unsigned i = 0; i < size * size; ++i) {
		data[i] = Uint8(base >> 24);
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



void perlinnoise::noise_func::set_line_for_interpolation(const vector<fixed32>& interpolation_func, fixed32 y) const
{
	fixed32 by = (phasey + y).frac();
	// remap to value/subvalue coordinates
	by = by * (size * frequency);
	unsigned sz1 = size - 1;
	offsetline1 = by.intpart() & sz1;
	offsetline2 = (offsetline1 + 1) & sz1;
	offsetline1 *= size;
	offsetline2 *= size;
	linefac2 = interpolation_func[(by.frac() * interpolation_func.size()).intpart()];
	linefac1 = fixed32::one() - linefac2;
}


Uint8 perlinnoise::noise_func::interpolate(const vector<fixed32>& interpolation_func, fixed32 x) const
{
	fixed32 bx = (phasex + x).frac();
	// remap to value/subvalue coordinates
	bx = bx * (size * frequency);
	unsigned sz1 = size - 1;
	unsigned x1 = bx.intpart() & sz1;
	unsigned x2 = (x1 + 1) & sz1;
	fixed32 a2 = interpolation_func[(bx.frac() * interpolation_func.size()).intpart()];
	fixed32 a1 = fixed32::one() - a2;
	unsigned v1 = (a1 * unsigned(data[offsetline1+x1]) + a2 * unsigned(data[offsetline1+x2])).intpart();
	unsigned v2 = (a1 * unsigned(data[offsetline2+x1]) + a2 * unsigned(data[offsetline2+x2])).intpart();
	unsigned res = (linefac1 * v1 + linefac2 * v2).intpart();
	return Uint8(res);
}



bool is_power2(unsigned x)
{
	return (x & (x-1)) == 0;
}

perlinnoise::perlinnoise(unsigned size, unsigned sizeminfreq, unsigned sizemaxfreq)
	: resultsize(size)
{
	sys().myassert(is_power2(size), "size is not power of two");
	sys().myassert(is_power2(sizeminfreq), "sizeminfreq is not power of two");
	sys().myassert(is_power2(sizemaxfreq), "sizemaxfreq is not power of two");
	sys().myassert(sizeminfreq >= 1 && sizeminfreq <= size && sizeminfreq <= sizemaxfreq, "sizeminfreq out of range");
	sys().myassert(sizemaxfreq >= 2 && sizemaxfreq <= size, "sizemaxfreq out of range");
	unsigned nrfunc = 0;
	for (unsigned j = sizemaxfreq/sizeminfreq; j > 0; j >>= 1)
		++nrfunc;
	// generate functions, most significant first.
	noise_functions.reserve(nrfunc);
	for (unsigned i = 0; i < nrfunc; ++i) {
		// growing size, constant frequency
		noise_functions.push_back(noise_func(size/(sizemaxfreq>>i), 1));
		// alternative, always same size, but growing frequency
//		noise_functions.push_back(noise_func(size/sizemaxfreq, 1<<i));
	}

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
		interpolation_func[i] = fixed32((1.0f - cosf(f)) * 0.5f);
	}
}



void perlinnoise::set_phase(unsigned func, float px, float py)
{
	if (func < noise_functions.size()) {
		noise_functions[func].phasex = fixed32(px);
		noise_functions[func].phasey = fixed32(py);
	}
}


static inline Sint32 clamp_zero(Sint32 x) { return x & ~(x >> 31); }
static inline Sint32 clamp_value(Sint32 x, Sint32 val) { return val - clamp_zero(val - x); }
vector<Uint8> perlinnoise::generate() const
{
	vector<Uint8> result(resultsize * resultsize);
	fixed32 dxy = fixed32::one()/resultsize;
	unsigned ptr = 0;
	fixed32 fy;
	for (unsigned y = 0; y < resultsize; ++y) {
		for (unsigned i = 0; i < noise_functions.size(); ++i) {
			noise_functions[i].set_line_for_interpolation(interpolation_func, fy);
		}
		fixed32 fx;
		for (unsigned x = 0; x < resultsize; ++x) {
			int sum = 0;
			for (unsigned i = 0; i < noise_functions.size(); ++i) {
				sum += (int(noise_functions[i].interpolate(interpolation_func, fx))-128) >> i;
			}
			// sum is at most around +- 207, so we multiply with 19/32, to get in in +-127 range
			// to be sure we clamp it also.
			sum = clamp_value(clamp_zero(((sum * 19) >> 5) + 128), 255);
			result[ptr++] = Uint8(sum);
			fx += dxy;
		}
		fy += dxy;
	}

	return result;
}
