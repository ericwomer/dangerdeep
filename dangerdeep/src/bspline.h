//  A bspline curve point generation algorithm (C)+(W) 2003 Thorsten Jordan

#ifndef BSPLINE_H
#define BSPLINE_H

#include <cassert>
#include <vector>
using namespace std;

template <class T>
T bspline_val(double t, int n, const vector<T>& d)
{
	assert (0 <= t && t <= 1);
	assert (n < int(d.size()) );
	
	int m = int(d.size())-1;
	
	vector<vector<T> > deBoor_pts;

	// prepare deBoor-Points Triangle
	deBoor_pts.resize(n+1);
	for (int j = 0; j <= n; ++j)
		deBoor_pts[j].resize(n+1-j);

	// prepare t-vector
	vector<double> tvec;
	tvec.resize(m+n+2);	// fixme: THIS is the performance bottleneck. for large coastlines
	int k = 0;		// m is around 1000, recreated for every point! fixme reuse this vector, it is constant for one spline
	for ( ; k <= n; ++k) tvec[k] = 0;
	for ( ; k <= m; ++k) tvec[k] = double(k-n)/double(m-n+1);
	for ( ; k <= m+n+1; ++k) tvec[k] = 1;
	
	// find l, also fixme: general bsplines may have non uniform t distances for their control points
	int l = n;	// but we know how the tvec is created so we don't need to find t but can find its position in O(1) time!
	for ( ; l < m+1; ++l) {
		if (tvec[l] <= t && t <= tvec[l+1])
			break;
	}
	
	// fill in base deBoor points
	for (int j = 0; j <= n; ++j)
		deBoor_pts[0][j] = d[l-n+j];
		
	// compute new values
	int doff = l-n;
	for (int r = 1; r <= n; ++r) {
		for (int i = l-n; i <= l-r; ++i) {
			double tv = (t - tvec[i+r])/(tvec[i+n+1] - tvec[i+r]);
			deBoor_pts[r][i-doff] = deBoor_pts[r-1][i-doff] * (1 - tv)
				+ deBoor_pts[r-1][i+1-doff] * tv;
		}
	}

	return deBoor_pts[n][0];
}

#endif
