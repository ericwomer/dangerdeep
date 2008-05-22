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
	return 0;
}
