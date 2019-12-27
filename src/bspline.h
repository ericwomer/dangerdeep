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

#include <stdexcept>
#include <vector>
#include <cmath>

#include "dmath.h"

///\brief Represents a non-uniform-B-spline interpolation object
template <class T>
class non_uniform_bsplinet
{
 protected:
	unsigned n, m;
	std::vector<T> cp;		// control points
	std::vector<double> tvec;		// t's for control points
	mutable std::vector<T> deBoor_pts;

	T& deBoor_at(unsigned row, unsigned column) const
	{
		return deBoor_pts[(n+1-row)*(n-row)/2 + column];
	}
	
	virtual unsigned find_l(double t) const
	{
		// algorithm for non-uniform bsplines:
		// note: if t is equal to tvec[x] then l=x-1
		// fixme: we could use a binary search here! but it would help only for large tvecs
		// store cp/tvec as map?
		unsigned l = n;
		for ( ; l <= m; ++l) {
			if (tvec[l] <= t && t <= tvec[l+1])
				break;
		}
		return l;
	}

	non_uniform_bsplinet();	// no empty construction

 public:
	non_uniform_bsplinet(unsigned n_, const std::vector<T>& d, const std::vector<double>& tvec_)
		: n(n_), m(d.size()-1), cp(d), tvec(tvec_)
	{
		if (n >= d.size()) throw std::runtime_error("bspline: n too large");
		if (d.size() < 2) throw std::runtime_error("bspline: d has too few elements");
		if (tvec_.size() != m+n+2) throw std::runtime_error("bspline: tvec has illegal size");
		deBoor_pts.resize((n+1)*(n+2)/2);
	}

	virtual ~non_uniform_bsplinet() {}

	const std::vector<T>& control_points() const { return cp; }
	
	T value(double t) const
	{
/* test: linear interpolation
		unsigned bp = unsigned(t*(cp.size()-1));
		unsigned np = bp+1;if(np==cp.size())np=bp;
		t = t*(cp.size()-1) - bp;
		return cp[bp] * (1.0-t) + cp[np] * t;
*/
		// better to limit t than to fail loudly.
		if (t < 0.0) t = 0.0;
		if (t > 1.0) t = 1.0;
		//if (t < 0.0 || t > 1.0) throw std::runtime_error("bspline: invalid t");

		unsigned l = find_l(t);
	
		// fill in base deBoor points
		for (unsigned j = 0; j <= n; ++j)
			deBoor_at(0, j) = cp[l-n+j];
		
		// compute new values
		for (unsigned r = 1; r <= n; ++r) {
			for (unsigned i = l-n; i <= l-r; ++i) {
				double tv = (t - tvec[i+r])/(tvec[i+n+1] - tvec[i+r]);
				if (!isfinite(tv)) throw std::runtime_error("bspline: invalid number generated");
				deBoor_at(r, i+n-l) = deBoor_at(r-1, i+n-l) * (1 - tv)
					+ deBoor_at(r-1, i+1+n-l) * tv;
			}
		}

		return deBoor_at(n, 0);
	}
};



///\brief Represents a B-Spline interpolation object.
template <class T>
class bsplinet : public non_uniform_bsplinet<T>
{
protected:
	unsigned find_l(double t) const
	{
		// note: for non-uniform bsplines we have to compute l so that
		// tvec[l] <= t <= tvec[l+1]
		// because we have uniform bsplines here, we don't need to search!
		// note: the results of both algorithms differ when t == tvec[x] for any x.
		// it shouldn't cause trouble though, because the bspline value is the same then.
		//note: the this->xyz dereferencing is needed to make the code compile.
		unsigned l = this->n + unsigned(floor(t * (this->m+1-this->n)));
		if (l > this->m) l = this->m;
		return l;
	}

public:
	bsplinet(unsigned n_, const std::vector<T>& d)
		: non_uniform_bsplinet<T>(n_, d, std::vector<double>(d.size()+n_+1 /* = m+n+2 */ ))
	{
		// prepare t-vector
		// note: this algorithm works also for non-uniform bsplines
		// (let the user give a t vector)
		unsigned k = 0;
		for ( ; k <= this->n; ++k) this->tvec[k] = 0;
		for ( ; k <= this->m; ++k) this->tvec[k] = double(k-this->n)/double(this->m-this->n+1);
		for ( ; k <= this->m+this->n+1; ++k) this->tvec[k] = 1;
	}
};



// square b-splines, give square vector of control points
template <class T>
class bspline2dt
{
protected:
	unsigned n, m;
	std::vector<T> cp;		// control points
	std::vector<double> tvec;		// t's for control points
	mutable std::vector<T> deBoor_pts;

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
	bspline2dt(unsigned n_, const std::vector<T>& d) : n(n_), cp(d)
	{
		unsigned ds = unsigned(sqrt(double(d.size())));
		if (ds*ds != d.size()) throw std::runtime_error("bspline2d: d not quadratic");
		if (n >= ds) throw std::runtime_error("bspline2d: n too large");
		if (ds < 2) throw std::runtime_error("bspline2d: d has too few elements");
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

	const std::vector<T>& control_points() const { return cp; }
	
	T value(double s, double t) const
	{
		if (s < 0.0 || s > 1.0) throw std::runtime_error("bspline2d: invalid s");
		if (t < 0.0 || t > 1.0) throw std::runtime_error("bspline2d: invalid t");

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
				if (!isfinite(tv)) throw std::runtime_error("bspline2d: invalid number generated");
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
				if (!isfinite(tv)) throw std::runtime_error("bspline2d: invalid number generated");
				deBoor_at(0, r, i+n-l2) = deBoor_at(0, r-1, i+n-l2) * (1 - tv)
					+ deBoor_at(0, r-1, i+1+n-l2) * tv;
			}
		}

		return deBoor_at(0, n, 0);
	}

};

typedef bsplinet<double> bspline;
typedef non_uniform_bsplinet<double> non_uniform_bspline;

#endif
