//  A bspline curve (C)+(W) Thorsten Jordan

#ifndef BSPLINE_H
#define BSPLINE_H

#include <cassert>
#include <vector>
using namespace std;

template <class T, class U>
class bsplinet
{
protected:
	int n;
	vector<T> cp;		// control points
	vector<U> tvec;		// t's for control points
	mutable vector<vector<T> > deBoor_pts;

	void make_deBoor_pts(void)
	{
		// prepare deBoor-Points Triangle
		deBoor_pts.resize(n+1);
		for (int j = 0; j <= n; ++j)
			deBoor_pts[j].resize(n+1-j);
	}
	
	bsplinet();

	int find_l(const U& t) const
	{
		// note: for non-uniform bsplines we have to compute l so that
		// tvec[l] <= t <= tvec[l+1]
		// because we have uniform bsplines here, we don't need to search!
		// note: the results of both algorithms differ when t == tvec[x] for any x.
		// it shouldn't cause trouble though, because the bspline value is the same then.
		int m = int(cp.size())-1;
		int l = n + int(floor(t * (m+1-n)));
		if (l > m) l = m;
		return l;
		
		// algorithm for non-uniform bsplines:
		// note: if t is equal to tvec[x] then l=x-1
		/*
		int l = n;
		for ( ; l <= m; ++l) {
			if (tvec[l] <= t && t <= tvec[l+1])
				break;
		}
		return l;
		*/
	}

public:
	bsplinet(int n_, const vector<T>& d) : n(n_), cp(d)
	{
		assert (n < int(d.size()) );
		
		make_deBoor_pts();

		// prepare t-vector
		// note: this algorithm works also for non-uniform bsplines
		// (let the user give a t vector)
		int m = int(d.size())-1;
		tvec.resize(m+n+2);
		int k = 0;
		for ( ; k <= n; ++k) tvec[k] = 0;
		for ( ; k <= m; ++k) tvec[k] = U(k-n)/U(m-n+1);
		for ( ; k <= m+n+1; ++k) tvec[k] = 1;
	}

	bsplinet(const bsplinet& o) : n(o.n), cp(o.cp), tvec(o.tvec) { make_deBoor_pts(); }
	bsplinet& operator= (const bsplinet& o) { n = o.n; cp = o.cp; tvec = o.tvec; return *this; }

	~bsplinet() {}

	const vector<T> control_points(void) const { return cp; }
	
	T value(const U& t) const
	{
		assert (0 <= t && t <= 1);

		int l = find_l(t);
	
		// fill in base deBoor points
		for (int j = 0; j <= n; ++j)
			deBoor_pts[0][j] = cp[l-n+j];
		
		// compute new values
		int doff = l-n;
		for (int r = 1; r <= n; ++r) {
			for (int i = l-n; i <= l-r; ++i) {
				U tv = (t - tvec[i+r])/(tvec[i+n+1] - tvec[i+r]);
				assert(isfinite(tv));
				deBoor_pts[r][i-doff] = deBoor_pts[r-1][i-doff] * (1 - tv)
					+ deBoor_pts[r-1][i+1-doff] * tv;
			}
		}

		return deBoor_pts[n][0];
	}

};

#endif
