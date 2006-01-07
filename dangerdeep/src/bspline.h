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

//  A bspline curve (C)+(W) Thorsten Jordan

#ifndef BSPLINE_H
#define BSPLINE_H

#include <cassert>
#include <vector>
using namespace std;

#if (defined(__APPLE__) && defined(__GNUC__)) || defined(__MACOSX__)
#include <complex.h>
#ifndef isfinite
#define isfinite(x) finite(x)
#endif
#endif

template <class T>
class bsplinet
{
protected:
	unsigned n, m;
	vector<T> cp;		// control points
	vector<double> tvec;		// t's for control points
	mutable vector<T> deBoor_pts;

	T& deBoor_at(unsigned row, unsigned column) const
	{
		return deBoor_pts[(n+1-row)*(n-row)/2 + column];
	}
	
	bsplinet();

	unsigned find_l(double t) const
	{
		// note: for non-uniform bsplines we have to compute l so that
		// tvec[l] <= t <= tvec[l+1]
		// because we have uniform bsplines here, we don't need to search!
		// note: the results of both algorithms differ when t == tvec[x] for any x.
		// it shouldn't cause trouble though, because the bspline value is the same then.

#if 1
		unsigned l = n + unsigned(floor(t * (m+1-n)));
		if (l > m) l = m;
		return l;
		
		// algorithm for non-uniform bsplines:
		// note: if t is equal to tvec[x] then l=x-1
#else
		// fixme: we could use a binary search here!
		unsigned l = n;
		for ( ; l <= m; ++l) {
			if (tvec[l] <= t && t <= tvec[l+1])
				break;
		}
		return l;
#endif
	}

public:
	bsplinet(unsigned n_, const vector<T>& d) : n(n_), m(d.size()-1), cp(d)
	{
		assert (n < d.size() );
		assert (d.size() >= 2);
		
		deBoor_pts.resize((n+1)*(n+2)/2);

		// prepare t-vector
		// note: this algorithm works also for non-uniform bsplines
		// (let the user give a t vector)
		tvec.resize(m+n+2);
		unsigned k = 0;
		for ( ; k <= n; ++k) tvec[k] = 0;
		for ( ; k <= m; ++k) tvec[k] = double(k-n)/double(m-n+1);
		for ( ; k <= m+n+1; ++k) tvec[k] = 1;
	}

	const vector<T>& control_points(void) const { return cp; }
	
	T value(double t) const
	{
/* test: linear interpolation
		unsigned bp = unsigned(t*(cp.size()-1));
		unsigned np = bp+1;if(np==cp.size())np=bp;
		t = t*(cp.size()-1) - bp;
		return cp[bp] * (1.0-t) + cp[np] * t;
*/
		assert (0 <= t && t <= 1);

		unsigned l = find_l(t);
	
		// fill in base deBoor points
		for (unsigned j = 0; j <= n; ++j)
			deBoor_at(0, j) = cp[l-n+j];
		
		// compute new values
		for (unsigned r = 1; r <= n; ++r) {
			for (unsigned i = l-n; i <= l-r; ++i) {
				double tv = (t - tvec[i+r])/(tvec[i+n+1] - tvec[i+r]);
				assert(isfinite(tv));
				deBoor_at(r, i+n-l) = deBoor_at(r-1, i+n-l) * (1 - tv)
					+ deBoor_at(r-1, i+1+n-l) * tv;
			}
		}

		return deBoor_at(n, 0);
	}

};



// square b-splines, give square vector of control points
template <class T>
class bspline2dt
{
protected:
	unsigned n, m;
	vector<T> cp;		// control points
	vector<double> tvec;		// t's for control points
	mutable vector<T> deBoor_pts;

	T& deBoor_at(unsigned line, unsigned row, unsigned column) const
	{
		return deBoor_pts[((n+1)*(n+2)/2)*line + (n+1-row)*(n-row)/2 + column];
	}
	
	bspline2dt();

	unsigned find_l(double t) const
	{
		// note: for non-uniform bsplines we have to compute l so that
		// tvec[l] <= t <= tvec[l+1]
		// because we have uniform bsplines here, we don't need to search!
		// note: the results of both algorithms differ when t == tvec[x] for any x.
		// it shouldn't cause trouble though, because the bspline value is the same then.

#if 1
		unsigned l = n + unsigned(floor(t * (m+1-n)));
		if (l > m) l = m;
		return l;
		
		// algorithm for non-uniform bsplines:
		// note: if t is equal to tvec[x] then l=x-1
#else
		unsigned l = n;
		for ( ; l <= m; ++l) {
			if (tvec[l] <= t && t <= tvec[l+1])
				break;
		}
		return l;
#endif
	}

public:
	// give square vector of control points, in C++ order, line after line
	bspline2dt(unsigned n_, const vector<T>& d) : n(n_), cp(d)
	{
		unsigned ds = unsigned(sqrt(double(d.size())));
		assert(ds*ds == d.size());
		assert(n < ds);
		assert(ds >= 2);
		m = ds-1;

		deBoor_pts.resize((n+1) * (n+1)*(n+2)/2);

		// prepare t-vector
		// note: this algorithm works also for non-uniform bsplines
		// (let the user give a t vector)
		tvec.resize(m+n+2);
		unsigned k = 0;
		for ( ; k <= n; ++k) tvec[k] = 0;
		for ( ; k <= m; ++k) tvec[k] = double(k-n)/double(m-n+1);
		for ( ; k <= m+n+1; ++k) tvec[k] = 1;
	}

	const vector<T>& control_points(void) const { return cp; }
	
	T value(double s, double t) const
	{
		assert (0 <= s && s <= 1);
		assert (0 <= t && t <= 1);

		unsigned l = find_l(s);
		unsigned l2 = find_l(t);
	
		// fill in base deBoor points
		for (unsigned j2 = 0; j2 <= n; ++j2)
			for (unsigned j = 0; j <= n; ++j)
				deBoor_at(j2, 0, j) = cp[(l2-n+j2)*(m+1) + l-n+j];
		
		// compute new values
		for (unsigned r = 1; r <= n; ++r) {
			for (unsigned i = l-n; i <= l-r; ++i) {
				double tv = (s - tvec[i+r])/(tvec[i+n+1] - tvec[i+r]);
				assert(isfinite(tv));
				for (unsigned j = 0; j <= n; ++j) {
					deBoor_at(j, r, i+n-l) = deBoor_at(j, r-1, i+n-l) * (1 - tv)
						+ deBoor_at(j, r-1, i+1+n-l) * tv;
				}
			}
		}

		for (unsigned j2 = 0; j2 <= n; ++j2)
			deBoor_at(0, 0, j2) = deBoor_at(j2, n, 0);
		for (unsigned r = 1; r <= n; ++r) {
			for (unsigned i = l2-n; i <= l2-r; ++i) {
				double tv = (t - tvec[i+r])/(tvec[i+n+1] - tvec[i+r]);
				assert(isfinite(tv));
				deBoor_at(0, r, i+n-l2) = deBoor_at(0, r-1, i+n-l2) * (1 - tv)
					+ deBoor_at(0, r-1, i+1+n-l2) * tv;
			}
		}

		return deBoor_at(0, n, 0);
	}

};



// some test code for the 2d bsplines! save as extra file to test it.
#if 0
#include "bspline.h"
#include <fstream>
using namespace std;

double rnd(void) { return double(rand())/RAND_MAX; }

const unsigned N = 4;
const unsigned D = 33;
const unsigned R = 4;
int main(int, char**)
{
	srand(3746867);
	vector<float> cps(D*D);
	for (unsigned y = 0; y < D; ++y)
		for (unsigned x = 0; x < D; ++x)
			cps[D*y+x] = 8.0f*rnd()/D;
	bspline2dt<float> bsp(N, cps);
	ofstream out("bspline.off");
	out << "OFF\n" << D*R*D*R << " " << (R*D-1)*(R*D-1)*2 << " 0\n";
	for (unsigned y = 0; y < R*D; ++y) {
		float fy = float(y)/(R*D-1);
		for (unsigned x = 0; x < R*D; ++x) {
			float fx = float(x)/(R*D-1);
			out << fx << " " << fy << " " << bsp.value(fx, fy) << "\n";
		}
	}
	for (unsigned y = 0; y < R*D-1; ++y) {
		for (unsigned x = 0; x < R*D-1; ++x) {
			out << "3 " << y*(R*D)+x << " " << y*(R*D)+x+1 << " " << (y+1)*(R*D)+x << "\n";
			out << "3 " << y*(R*D)+x+1 << " " << (y+1)*(R*D)+x+1 << " " << (y+1)*(R*D)+x << "\n";
		}
	}
	return 0;
}
#endif // test code

#endif
