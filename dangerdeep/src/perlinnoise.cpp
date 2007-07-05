/*
Danger from the Deep - Open source submarine simulation
Copyright (C) 2003-2006  Thorsten Jordan, Luis Barrancos and others.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

// perlin 2d noise generator
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "perlinnoise.h"
#include <math.h>
#include <sstream>
#include <fstream>
#include <stdexcept>
using std::vector;

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
	fixed32 v1 = a1 * unsigned(data[offsetline1+x1]) + a2 * unsigned(data[offsetline1+x2]);
	fixed32 v2 = a1 * unsigned(data[offsetline2+x1]) + a2 * unsigned(data[offsetline2+x2]);
	unsigned res = (linefac1 * v1 + linefac2 * v2).intpart();
	return Uint8(res);
}



Uint8 perlinnoise::noise_func::interpolate_sqr(fixed32 x, fixed32 y) const
{
	fixed32 bx = (phasex + x).frac();
	fixed32 by = (phasey + y).frac();
	// remap to value/subvalue coordinates
	bx = bx * (size * frequency);
	by = by * (size * frequency);
	unsigned sz1 = size - 1;
	unsigned x1 = bx.intpart() & sz1;
	unsigned y1 = by.intpart() & sz1;
	unsigned x2 = (x1 + 1) & sz1;
	unsigned y2 = (y1 + 1) & sz1;
	Uint8 a = data[y1 * size + x1];
	Uint8 b = data[y1 * size + x2];
	Uint8 c = data[y2 * size + x1];
	Uint8 d = data[y2 * size + x2];
	// if next value is greater than this value, use ascending function f(x)=x^2
	// if next value is less than this value, use descending function f(x)=1-(1-x)^2
	// = 1-(1-2x+x^2) = 2x-x^2
//fixme: Ken Perlin suggests cosine interpolation, that works no matter if next value is greater or not.
//lookup would be much easier, and as benefit, we can use shaders to speed up rendering,
//composing perlin noise directly as texture in video memory
//the clouds are already interpolated that way! and not with this code!
	fixed32 bx2 = bx.frac() * bx.frac();
	fixed32 by2 = by.frac() * by.frac();
	fixed32 f = (b < a) ? bx.frac()*2 - bx2 : bx2;
	fixed32 r1 = f * b + (fixed32::one() - f) * a;
	f = (d < c) ? bx.frac() + bx.frac() - bx2 : bx2;
	fixed32 r2 = f * d + (fixed32::one() - f) * c;
	f = (r2 < r1) ? by.frac() + by.frac() - by2 : by2;
	unsigned res = (f * r2 + (fixed32::one() - f) * r1).intpart();
	return Uint8(res);
}



bool is_power2(unsigned x)
{
	return (x & (x-1)) == 0;
}

perlinnoise::perlinnoise(unsigned size, unsigned sizeminfreq, unsigned sizemaxfreq)
	: resultsize(size)
{
	if (!is_power2(size)) throw std::invalid_argument("size is not power of two");
	if (!is_power2(sizeminfreq)) throw std::invalid_argument("sizeminfreq is not power of two");
	if (!is_power2(sizemaxfreq)) throw std::invalid_argument("sizemaxfreq is not power of two");
	if (!(sizeminfreq >= 1 && sizeminfreq <= size && sizeminfreq <= sizemaxfreq)) throw std::invalid_argument("sizeminfreq out of range");
	if (!(sizemaxfreq >= 2 && sizemaxfreq <= size)) throw std::invalid_argument("sizemaxfreq out of range");
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

#if 1
		float f = M_PI * float(i)/res;
		interpolation_func[i] = fixed32((1.0f - cosf(f)) * 0.5f);
#else
		// special wave-shape-like function. use 1-f(x) if interpolating low->high value
		float x = float(i)/res;
		interpolation_func[i] = fixed32((1-x)*(1-x));
#endif
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



vector<Uint8> perlinnoise::generate_sqr() const
{
	vector<Uint8> result(resultsize * resultsize);
	fixed32 dxy = fixed32::one()/resultsize;
	unsigned ptr = 0;
	fixed32 fy;
	for (unsigned y = 0; y < resultsize; ++y) {
		fixed32 fx;
		for (unsigned x = 0; x < resultsize; ++x) {
			int sum = 0;
			for (unsigned i = 0; i < noise_functions.size(); ++i) {
				sum += (int(noise_functions[i].interpolate_sqr(fx, fy))-128) >> i;
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



perlinnoise::perlinnoise(unsigned levelsize, unsigned sizeminfreq, unsigned levels, bool /*dummy*/)
{
	if (!is_power2(levelsize)) throw std::invalid_argument("levelsize is not power of two");
	if (!is_power2(sizeminfreq)) throw std::invalid_argument("sizeminfreq is not power of two");
	if (levels < 1) throw std::invalid_argument("levels must be >= 1");

	resultsize = levelsize * sizeminfreq * (1 << levels - 1);

	noise_functions.reserve(levels);
	for (unsigned i = 0; i < levels; ++i) {
		noise_functions.push_back(noise_func(levelsize, 1));
	}

	// create interpolation function
	const unsigned res = 256;
	interpolation_func.resize(res);
	for (unsigned i = 0; i < res; ++i) {
		float f = M_PI * float(i)/res;
		interpolation_func[i] = fixed32((1.0f - cosf(f)) * 0.5f);
	}
}



Uint8 perlinnoise::value(unsigned x, unsigned y, unsigned depth) const
{
	fixed32 dxy = fixed32::one()/resultsize;
 	x = x & (resultsize - 1);
 	y = y & (resultsize - 1);
	int sum = 0;
	unsigned k = std::min(depth, noise_functions.size());
	for (unsigned i = 0; i < k; ++i) {
		// we have to remove the part of x/y that will be
		// integral and bigger than size later
		int xx = (x << i) & (resultsize - 1);
		int yy = (y << i) & (resultsize - 1);
		fixed32 fx = dxy * xx, fy = dxy * yy;
		noise_functions[i].set_line_for_interpolation(interpolation_func, fy);
		sum += (int(noise_functions[i].interpolate(interpolation_func, fx))-128) >> i;
	}
	// rescale sum here
	return Uint8(clamp_value(clamp_zero(((sum * 19) >> 5) + 128), 255));
}



float perlinnoise::valuef(unsigned x, unsigned y, unsigned depth) const
{
	fixed32 dxy = fixed32::one()/resultsize;
 	x = x & (resultsize - 1);
 	y = y & (resultsize - 1);
	float sum = 0, f = 1.0f;
	unsigned k = std::min(depth, noise_functions.size());
	for (unsigned i = 0; i < k; ++i) {
		// we have to remove the part of x/y that will be
		// integral and bigger than size later
		int xx = (x << i) & (resultsize - 1);
		int yy = (y << i) & (resultsize - 1);
		fixed32 fx = dxy * xx, fy = dxy * yy;
		noise_functions[i].set_line_for_interpolation(interpolation_func, fy);
		sum += (int(noise_functions[i].interpolate(interpolation_func, fx))-128) * f;
		f *= 0.5f;
	}
	return f;
}



std::vector<Uint8> perlinnoise::values(unsigned x, unsigned y, unsigned w, unsigned h, unsigned depth) const
{
	std::vector<Uint8> result(w * h);
	fixed32 dxy = fixed32::one()/resultsize;
 	x = x & (resultsize - 1);
 	y = y & (resultsize - 1);
	unsigned k = std::min(depth, noise_functions.size());
	for (unsigned y2 = y; y2 < y + h; ++y2) {
		for (unsigned i = 0; i < k; ++i) {
			int yy = (y2 << i) & (resultsize - 1);
			fixed32 fy = dxy * yy;
			noise_functions[i].set_line_for_interpolation(interpolation_func, fy);
		}
		for (unsigned x2 = x; x2 < x + w; ++x2) {
			int sum = 0;
			for (unsigned i = 0; i < k; ++i) {
				// we have to remove the part of x/y that will be
				// integral and bigger than size later
				int xx = (x2 << i) & (resultsize - 1);
				fixed32 fx = dxy * xx;
				sum += (int(noise_functions[i].interpolate(interpolation_func, fx))-128) >> i;
			}
			// rescale sum here
			result[y2*w+x2] = Uint8(clamp_value(clamp_zero(((sum * 19) >> 5) + 128), 255));
		}
	}
	return result;
}



std::vector<float> perlinnoise::valuesf(unsigned x, unsigned y, unsigned w, unsigned h, unsigned depth) const
{
	std::vector<float> result(w * h);
	fixed32 dxy = fixed32::one()/resultsize;
 	x = x & (resultsize - 1);
 	y = y & (resultsize - 1);
	unsigned k = std::min(depth, noise_functions.size());
	for (unsigned y2 = y; y2 < y + h; ++y2) {
		for (unsigned i = 0; i < k; ++i) {
			int yy = (y2 << i) & (resultsize - 1);
			fixed32 fy = dxy * yy;
			noise_functions[i].set_line_for_interpolation(interpolation_func, fy);
		}
		for (unsigned x2 = x; x2 < x + w; ++x2) {
			float sum = 0, f = 1.0f;
			for (unsigned i = 0; i < k; ++i) {
				// we have to remove the part of x/y that will be
				// integral and bigger than size later
				int xx = (x2 << i) & (resultsize - 1);
				fixed32 fx = dxy * xx;
				sum += (int(noise_functions[i].interpolate(interpolation_func, fx))-128) * f;
				f *= 0.5f;
			}
			// rescale sum here
			result[y2*w+x2] = f;
		}
	}
	return result;
}
