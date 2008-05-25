#include "bivector.h"
#include <fstream>
using namespace std;

void save(const bivector<uint8_t>& b, const char* n)
{
	ofstream os(n);
	os << "P5\n" << b.size().x << " " << b.size().y << "\n255\n";
	os.write((const char*)b.data_ptr(), b.size().x*b.size().y);
}

bivector<uint8_t> load(const char* n)
{
	ifstream is(n);
	string tmp;
	getline(is, tmp);//P5
	getline(is, tmp);//comment
	int w, h;
	is >> w >> h;
	getline(is, tmp);// \n
	getline(is, tmp);//255
	//printf("%i %i\n",w,h);
	bivector<uint8_t> result(vector2i(w, h));
	is.read((char*)result.data_ptr(), w*h);
	return result;
}

int main(int, char**)
{
	save(load("test.pgm"), "test2.pgm");
	save(load("test.pgm").upsampled(), "test3.pgm");
	save(load("test.pgm").upsampled().upsampled(), "test4.pgm");
	save(load("test.pgm").upsampled().upsampled().upsampled(), "test5.pgm");
	save(load("test.pgm").downsampled(), "test6.pgm");
	save(load("test.pgm").downsampled().downsampled(), "test7.pgm");
	save(load("test.pgm").downsampled().downsampled().downsampled(), "test8.pgm");
	save(load("test.pgm").upsampled(true), "test3w.pgm");
	save(load("test.pgm").upsampled(true).upsampled(true), "test4w.pgm");
	save(load("test.pgm").upsampled(true).upsampled(true).upsampled(true), "test5w.pgm");

	save(load("testb.pgm"), "test2b.pgm");
	save(load("testb.pgm").upsampled(), "test3b.pgm");
	save(load("testb.pgm").upsampled().upsampled(), "test4b.pgm");
	save(load("testb.pgm").upsampled().upsampled().upsampled(), "test5b.pgm");
	save(load("testb.pgm").downsampled(), "test6b.pgm");
	save(load("testb.pgm").downsampled().downsampled(), "test7b.pgm");
	save(load("testb.pgm").downsampled().downsampled().downsampled(), "test8b.pgm");
	save(load("testb.pgm").upsampled(true), "test3bw.pgm");
	save(load("testb.pgm").upsampled(true).upsampled(true), "test4bw.pgm");
	save(load("testb.pgm").upsampled(true).upsampled(true).upsampled(true), "test5bw.pgm");

	save(load("test.pgm").convert<float>().smooth_upsampled().convert<uint8_t>(0,255), "test3s.pgm");
	save(load("test.pgm").convert<float>().smooth_upsampled().smooth_upsampled().convert<uint8_t>(0,255), "test4s.pgm");
	save(load("test.pgm").convert<float>().smooth_upsampled().smooth_upsampled().smooth_upsampled().convert<uint8_t>(0,255), "test5s.pgm");

	save(load("test.pgm").convert<float>().smooth_upsampled(true).convert<uint8_t>(0,255), "test3sw.pgm");
	save(load("test.pgm").convert<float>().smooth_upsampled(true).smooth_upsampled(true).convert<uint8_t>(0,255), "test4sw.pgm");
	save(load("test.pgm").convert<float>().smooth_upsampled(true).smooth_upsampled(true).smooth_upsampled(true).convert<uint8_t>(0,255), "test5sw.pgm");

	bivector<float> y(vector2i(256,256));
	y.add_gauss_noise(1.0);
	y *= 128;
	y += 128;
	save(y.convert<uint8_t>(0, 255), "testgauss.pgm");
	bivector<float> noise(vector2i(50, 50));
	noise.add_gauss_noise(1.0);
	bivector<float> x(vector2i(16,16));
	x.add_tiled(noise, 1.0);
	x = x.smooth_upsampled(true);
	x.add_tiled(noise, 0.5);
	x = x.smooth_upsampled(true);
	x.add_tiled(noise, 0.25);
	x = x.smooth_upsampled(true);
	x.add_tiled(noise, 0.125);
	x = x.smooth_upsampled(true);
	x.add_tiled(noise, 0.0625);
	x = x.smooth_upsampled(true);
	x.add_tiled(noise, 1.0/32);
	x = x.smooth_upsampled(true);
	x.add_tiled(noise, 1.0/64);
	x = x.smooth_upsampled(true);
	x.add_tiled(noise, 1.0/128);
	// saving
	x *= 128;
	x += 128;
	save(x.convert<uint8_t>(0, 255), "testgauss2.pgm");
	return 0;
}
