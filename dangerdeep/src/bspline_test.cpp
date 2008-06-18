// some test code for the 2d bsplines!
#include "bspline.h"
#include <fstream>
#include <cstdlib>
using namespace std;

double rnd() { return double(rand())/RAND_MAX; }

#if 1
int main(int, char**)
{
	// two parts, parabel halfs with fast rise and slow fall
	// s = 0.5*a*t^2, a = g = 9.81m/s^2
	// t_total = t_a + t_b, t_a/t_b depends on s.
	// t_2 depends on s,
	// e.g. reach 20m in 0.5s -> 40m/s rise speed
	// 20m = 0.5*g*t_b^2 => t_b = sqrt(40m/g) = 2s ca.
	// t_a = 0.5s. -> t_total = 2.5s.
	// t>=t_a then s = s_0 - g*0.5*(t-t_a)^2 = 20m - 5*(t-0.5)^2

#if 0
	vector<double> p;
	p.push_back(0);		// 0s:	0m
	p.push_back(20);	// 0.5s:20m
	p.push_back(18.75);	// 1.0s:18.75
	p.push_back(15.0);	// 1.5s:15
	p.push_back(8.75);	// 2.0s:8.75
	p.push_back(0.0);	// 2.5s:0
	bspline bsp(3, p);
#else
	double risetime = 0.08, riseheight = 4.0;
	double falltime = sqrt(riseheight * 2.0 / 9.806);
	double lifetime = risetime + falltime;
	std::vector<double> h, t;
	// initially height 0.
	h.push_back(0);
	t.push_back(0);
	t.push_back(0);
	t.push_back(0);
	h.push_back(0);
	t.push_back(0);
	// half rise time, half rise height
	h.push_back(riseheight*0.5);
	t.push_back(risetime*0.5/lifetime);
	// full rise
	h.push_back(riseheight*0.5);
	t.push_back(risetime/lifetime);
	// compute some points for fall
	for (double tm = risetime; tm <= lifetime - risetime; tm += risetime) {
		h.push_back(riseheight - tm*tm * 9.806 * 0.5);
		t.push_back(tm + risetime);
	}
	h.push_back(0);
	t.push_back(1);
	t.push_back(1);
	t.push_back(1);
	non_uniform_bspline bsp(3, h, t);
#endif
	for (double t = 0; t <= 1.0; t += 0.02) {
		double v = bsp.value(t);
		printf("[%f] = %f\n", t, v);
	}
	return 0;
}

#else

const unsigned N = 4;
const unsigned D = 33;
const unsigned R = 4;
int main(int, char**)
{
	srand(3746867);
	std::vector<float> cps(D*D);
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
#endif
