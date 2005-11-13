// (ocean) water simulation test
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "ocean_wave_generator.h"
#include <fstream>
using namespace std;
typedef unsigned char Uint8;

int main(int argc, char** argv)
{
	srand(1234);
	ocean_wave_generator<float> owg1(2048,	// wave resolution
					 vector2f(1,1),	// wind dir
					 12,	// wind speed m/s
					 1e-8,	// scale factor "A"
					 1024,	// wave tile length
					 100);	// tide cycle time in seconds
	srand(1234);
/*
	ocean_wave_generator<float> owg2(256,	// wave resolution
					 vector2f(1,1),	// wind dir
					 12,	// wind speed m/s
					 1e-8,	// scale factor "A"
					 1024,	// wave tile length
					 100);	// tide cycle time in seconds
*/
	ocean_wave_generator<float> owg2(owg1, 256);

	srand(1234);
	owg1.set_time(0);
	srand(1234);
	owg2.set_time(0);
	vector<float> heights1;
	vector<float> heights2;
	cout << "gen 1...\n";
	owg1.compute_heights(heights1);
	cout << "gen 2...\n";
	owg2.compute_heights(heights2);

	// compute an interpolated version of the lower one
	vector<float> heights3 = heights1;
	int fac = 2048/256;
	for (int y = 0; y < 2048; ++y) {
		int yy = y / fac;
		int y2 = (yy + 1) % 256;
		float yr = float(y - yy * fac) / fac;
		for (int x = 0; x < 2048; ++x) {
			int xx = x / fac;
			int x2 = (xx + 1) % 256;
			float xr = float(x - xx * fac) / fac;
			float h0 = heights2[yy*256+xx];
			float h1 = heights2[yy*256+x2];
			float h2 = heights2[y2*256+xx];
			float h3 = heights2[y2*256+x2];
			float h4 = (1-xr)*h0 + xr*h1;
			float h5 = (1-xr)*h2 + xr*h3;
			heights3[y*2048+x] = (1-yr)*h4+yr*h5;
		}
	}

	vector<float> heights4 = heights3;
	for (unsigned i = 0; i < heights3.size(); ++i) {
		heights4[i] = heights1[i] - heights3[i];
	}

	float minh = 1e10;
	float maxh = -1e10;
	for (vector<float>::const_iterator it = heights1.begin(); it != heights1.end(); ++it) {
		minh = fmin(minh, *it);
		maxh = fmax(maxh, *it);
	}
	cout << "1: minh " << minh << " maxh " << maxh << "\n";
	ofstream osg1("waveh1.pgm");
	osg1 << "P5\n";
	osg1 << 2048 <<" "<< 2048 <<"\n255\n";
	for (vector<float>::const_iterator it = heights1.begin(); it != heights1.end(); ++it) {
		Uint8 h = Uint8((*it - minh)*255/(maxh - minh));
		osg1.write((const char*)&h, 1);
	}

	minh = 1e10;
	maxh = -1e10;
	for (vector<float>::const_iterator it = heights2.begin(); it != heights2.end(); ++it) {
		minh = fmin(minh, *it);
		maxh = fmax(maxh, *it);
	}
	cout << "2: minh " << minh << " maxh " << maxh << "\n";
	ofstream osg2("waveh2.pgm");
	osg2 << "P5\n";
	osg2 << 256 <<" "<< 256 <<"\n255\n";
	for (vector<float>::const_iterator it = heights2.begin(); it != heights2.end(); ++it) {
		Uint8 h = Uint8((*it - minh)*255.9/(maxh - minh));
		osg2.write((const char*)&h, 1);
	}

	minh = 1e10;
	maxh = -1e10;
	for (vector<float>::const_iterator it = heights3.begin(); it != heights3.end(); ++it) {
		minh = fmin(minh, *it);
		maxh = fmax(maxh, *it);
	}
	cout << "3: minh " << minh << " maxh " << maxh << "\n";
	ofstream osg3("waveh3.pgm");
	osg3 << "P5\n";
	osg3 << 2048 <<" "<< 2048 <<"\n255\n";
	for (vector<float>::const_iterator it = heights3.begin(); it != heights3.end(); ++it) {
		Uint8 h = Uint8((*it - minh)*255.9/(maxh - minh));
		osg3.write((const char*)&h, 1);
	}

	minh = 1e10;
	maxh = -1e10;
	for (vector<float>::const_iterator it = heights4.begin(); it != heights4.end(); ++it) {
		minh = fmin(minh, *it);
		maxh = fmax(maxh, *it);
	}
	cout << "4: minh " << minh << " maxh " << maxh << "\n";
	ofstream osg4("waveh4.pgm");
	osg4 << "P5\n";
	osg4 << 2048 <<" "<< 2048 <<"\n255\n";
	for (vector<float>::const_iterator it = heights4.begin(); it != heights4.end(); ++it) {
		Uint8 h = Uint8((*it - minh)*255.9/(maxh - minh));
		osg4.write((const char*)&h, 1);
	}

	return 0;
}
