// a coastmap
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <GL/gl.h>

#include "global_data.h"
#include "coastmap.h"
#include <fstream>


void coastmap::save(const string& filename) const
{
	ofstream out((get_map_dir() + filename).c_str(), ios::binary|ios::out);
	write_u16(out, pixels_per_seg);
	write_u16(out, segsx);
	write_u16(out, segsy);
	write_double(out, pixelw_real);
	write_double(out, offsety);
	write_double(out, offsetx);
	for (vector<coastsegment>::const_iterator it = coastsegments.begin(); it != coastsegments.end(); ++it) {
		it->save(out);
	}
}

coastmap::coastmap(const string& filename)
{
	ifstream in((get_map_dir() + filename).c_str(), ios::in | ios::binary);
	pixels_per_seg = read_u16(in);
	segsx = read_u16(in);
	segsy = read_u16(in);
	pixelw_real = read_double(in);
	offsetx = read_double(in);
	offsety = read_double(in);
	unsigned s = segsx * segsy;
	coastsegments.reserve(s);
	for ( ; s > 0; --s) {
		coastsegments.push_back(coastsegment(in));
	}
}

void coastmap::draw_as_map(void) const
{
	double rsegw = pixelw_real * pixels_per_seg;
	double ry = 0;//-offsety;
	for (unsigned y = 0; y < segsy; ++y) {
		double rx = offsetx;
		for (unsigned x = 0; x < segsx; ++x) {
			const coastsegment& cs = coastsegments[y*segsx+x];
			glColor4f(0.5,0.5,0.5,1);
			glBegin(GL_LINE_LOOP);
			glVertex2d(rx, ry);
			glVertex2d(rx, ry+rsegw);
			glVertex2d(rx+rsegw, ry+rsegw);
			glVertex2d(rx+rsegw, ry);
			glEnd();
			glColor4f(1,1,0,0.5);
			
			if (cs.type == 1) {
/*
				glBegin(GL_QUADS);
				glVertex2d(rx, -ry);
				glVertex2d(rx, -ry-rsegw);
				glVertex2d(rx+rsegw, -ry-rsegw);
				glVertex2d(rx+rsegw, -ry);
				glEnd();
*/				
			} else if (cs.type > 1) {
				for (vector<coastline>::const_iterator it = cs.coastlines.begin(); it != cs.coastlines.end(); ++it) {
					if (it->cyclic)
						glBegin(GL_LINE_LOOP);
					else
						glBegin(GL_LINE_STRIP);
					for (vector<vector2f>::const_iterator it2 = it->points.begin(); it2 != it->points.end(); ++it2) {
//cout<<"render "<<it2->x<<","<<it2->y<<"\n";
						glVertex2f(rx + it2->x, ry + it2->y);
					}
					glEnd();
				}
			}
			rx += rsegw;
		}
		ry += rsegw;
	}
}
