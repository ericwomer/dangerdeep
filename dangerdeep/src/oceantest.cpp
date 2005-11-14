// (ocean) water simulation test
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "ocean_wave_generator.h"
#include <fstream>
using namespace std;
typedef unsigned char Uint8;

int main(int argc, char** argv)
{
	const unsigned resbig = 1024;
	const unsigned ressml = 128;
	srand(1234);
	/*
	  Wave tile length of 256 gives only few detail at small waves.
	  Rather use length of 512 or more. That way the tiles will be less visible.
	  Resolutions that are too high (4096 or so) don't bring any more detail.
	  For a 256m wide tile there is are only few high-frequency details.
	  (256m vs. 4096 res => 6.25cm per pixel).
	  Values below 25cm or so aren't noticeable. So use 25cm details, with 128 res
	  this gives a subdetail tile of 32m.
	  This means a 256m tile with 128 res, so 2m per pixel.
	  And a (bump map) hf-tile with 128 res, 32m length, so 0.25m per pixel.
	  The fft translates frequencies to heights, so we have 128x128 pixel
	  or 128x128 amplitude values for frequencies.
	  We want only the higher frequencies, this means anything below 2m.
	  With 32m to 256m the hf-tile is 1/8 size of the real tile.
	  So the frequencies for 1/1, 1/2, 1/4 (and maybe 1/8) must be cleared.
	  This means only 4 of 128 lines/columns, or clearing 1/4 of all lines/columns?
	  Which frequencies are stored? 1/1,1/2,1/3,...1/128 ???
	*/

	ocean_wave_generator<float> owg3(resbig,	// wave resolution
					 vector2f(1,1),	// wind dir
					 12,	// wind speed m/s
					 1e-8,	// scale factor "A"
					 256,	// wave tile length
					 10);	// tide cycle time in seconds
	srand(1234);
	ocean_wave_generator<float> owg1(owg3, resbig, 32);
/*
	ocean_wave_generator<float> owg2(ressml,	// wave resolution
					 vector2f(1,1),	// wind dir
					 12,	// wind speed m/s
					 1e-8,	// scale factor "A"
					 1024,	// wave tile length
					 100);	// tide cycle time in seconds
*/
	ocean_wave_generator<float> owg2(owg1, ressml);

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
	int fac = resbig/ressml;
	for (int y = 0; y < resbig; ++y) {
		int yy = y / fac;
		int y2 = (yy + 1) % ressml;
		float yr = float(y - yy * fac) / fac;
		for (int x = 0; x < resbig; ++x) {
			int xx = x / fac;
			int x2 = (xx + 1) % ressml;
			float xr = float(x - xx * fac) / fac;
			float h0 = heights2[yy*ressml+xx];
			float h1 = heights2[yy*ressml+x2];
			float h2 = heights2[y2*ressml+xx];
			float h3 = heights2[y2*ressml+x2];
			float h4 = (1-xr)*h0 + xr*h1;
			float h5 = (1-xr)*h2 + xr*h3;
			heights3[y*resbig+x] = (1-yr)*h4+yr*h5;
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
	osg1 << resbig <<" "<< resbig <<"\n255\n";
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
	osg2 << ressml <<" "<< ressml <<"\n255\n";
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
	osg3 << resbig <<" "<< resbig <<"\n255\n";
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
	osg4 << resbig <<" "<< resbig <<"\n255\n";
	for (vector<float>::const_iterator it = heights4.begin(); it != heights4.end(); ++it) {
		Uint8 h = Uint8((*it - minh)*255.9/(maxh - minh));
		osg4.write((const char*)&h, 1);
	}

	return 0;
}
