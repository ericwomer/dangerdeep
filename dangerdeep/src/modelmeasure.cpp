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

// a cross section measurement tool
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <gl.h>
#include <glu.h>
#include <SDL.h>

#include "system.h"
#include "vector3.h"
#include "datadirs.h"
#include "font.h"
#include "model.h"
#include "texture.h"
#include "thread.h"
#include "xml.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include "mymain.cpp"

using namespace std;

/*
  - measure cross section (graphical)
    depends on draught, this can vary,
    either use draught from CoG or what is given by the model,
    or compute full side cross section and later compute
    part by real draught, by computing volume below water
    divided by full volume. [DONE]
  - measure volume in z-slices (depth = x -> volume below water = y) [FIXME]
    why that? what for? superseded by voxels!?
  - determine center of gravity [DONE]
  - determine inertia tensor [DONE]
    fixme: first shift model so CoG is at 0,0,0 ?
  - measure area from above (for computing vertical drag when moving up/down) [FIXME]
  - maybe render silhuette from above for foam generation.
  - generate voxel data for buoyancy computation [DONE]

  maybe
  - modify ddxml and add the data
  - or write extra xml file with all that data. [DONE]

  remove old crosssection tool&code (SConscript)

  - problem: handle the transformation matrix! [FIXME]
    the objecttree can have a translation for the base mesh too...
  - problem: translating the model so that CoG is at 0,0,0 is difficult,
    because the model's dimensions are not resized then, but we need
    them for the voxel data.
    The inertia tensor differs if CoG is shifted before computing it.
*/

int res_x, res_y;
model* mdl;
double screenarea_meters;
double mw, mh;
unsigned ANGLES = 256;



void draw_model(double angle)
{
	glPushMatrix();
	glRotatef(angle, 0, 1, 0);
	glRotated(-90.0, 1, 0, 0);
	mdl->display();
	glPopMatrix();

// 	glColor3f(1,0,0);
// 	glBegin(GL_LINES);
// 	glVertex2d(0,0);
// 	glVertex2d(10,10);
// 	glEnd();
// 	glColor3f(1,1,1);
}

void measure_model(double angle, ostringstream& osscs)
{
	glClear(GL_COLOR_BUFFER_BIT);
	draw_model(angle);
	unsigned filledpixels = 0;
	unsigned screen_pixels = res_x*res_y;
	vector<Uint8> pic(screen_pixels*3);
	glReadPixels(0, 0, res_x, res_y, GL_RGB, GL_UNSIGNED_BYTE, &pic[0]);
	for (unsigned i = 0; i < screen_pixels*3; i += 3) {
		// faster counting, uses no conditional jumps.
		filledpixels += pic[i] ? 1 : 0;
	}
	double crosssection = screenarea_meters * filledpixels / screen_pixels;
	osscs << crosssection << " ";
	sys().swap_buffers();
}



class worker : public thread
{
	model& mdl;
	vector<uint8_t>& is_inside;
	vector3i resolution;
	unsigned slice;
	unsigned nr_slices;
	unsigned samples_per_voxel;
public:
	worker(model& m, vector<uint8_t>& ii, const vector3i& res, unsigned s, unsigned nrs,
	       unsigned samplespervoxel = 4)
		: mdl(m), is_inside(ii), resolution(res), slice(s), nr_slices(nrs),
		  samples_per_voxel(samplespervoxel) {}
	void loop()
	{
		int zmin = resolution.z * slice / nr_slices;
		int zmax = (slice + 1 == nr_slices) ? resolution.z : (resolution.z * (slice+1) / nr_slices);

		const vector3f& bmax = mdl.get_mesh(0).max;
		const vector3f& bmin = mdl.get_mesh(0).min;
		const vector3f bsize = bmax - bmin;
		const double csx = bsize.x / resolution.x;
		const double csy = bsize.y / resolution.y;
		const double csz = bsize.z / resolution.z;
		const double csx2 = csx / samples_per_voxel;
		const double csy2 = csy / samples_per_voxel;
		const double csz2 = csz / samples_per_voxel;
		double zc = bmin.z + zmin * csz;
		for (int izz = zmin; izz < zmax; ++izz) {
			double yc = bmin.y;
			for (int iyy = 0; iyy < resolution.y; ++iyy) {
				double xc = bmin.x;
				for (int ixx = 0; ixx < resolution.x; ++ixx) {
					unsigned inside = 0;
					double zc2 = zc + csz2 * 0.5;
					for (unsigned z = 0; z < samples_per_voxel; ++z) {
						double yc2 = yc + csy2 * 0.5;
						for (unsigned y = 0; y < samples_per_voxel; ++y) {
							double xc2 = xc + csx2 * 0.5;
							for (unsigned x = 0; x < samples_per_voxel; ++x) {
								bool is_in = mdl.is_inside(vector3f(xc2, yc2, zc2));
								inside += is_in ? 1 : 0;
								xc2 += csx2;
							}
							yc2 += csy2;
						}
						zc2 += csz2;
					}
					//cout << "is_inside " << inside << " / " << samples_per_voxel*samples_per_voxel*samples_per_voxel << "\n";
					unsigned inside_part = 255 * inside / (samples_per_voxel*samples_per_voxel*samples_per_voxel);
					is_inside[(izz * resolution.y + iyy) * resolution.x + ixx] = inside_part;
					xc += csx;
				}
				yc += csy;
			}
			zc += csz;
			cout << ".";
			cout.flush();
		}
		request_abort();
	}
};




int mymain(list<string>& args)
{
	res_x = 1024;
	bool fullscreen = true;
	string modelfilename;
	for (list<string>::iterator it = args.begin(); it != args.end(); ++it) {
		if (*it == "--help") {
			cout << "crosssection, usage:\n--help\t\tshow this\n"
			<< "--res n\t\tuse resolution n horizontal,\n\t\tn is 512,640,800,1024 (recommended) or 1280\n"
			<< "--nofullscreen\tdon't use fullscreen\n--angles n\tmeasure n different angles\n"
			<< "MODELFILENAME\n";
			return 0;
		} else if (*it == "--nofullscreen") {
			fullscreen = false;
		} else if (*it == "--res") {
			list<string>::iterator it2 = it; ++it2;
			if (it2 != args.end()) {
				int r = atoi(it2->c_str());
				if (r==512||r==640||r==800||r==1024||r==1280)
					res_x = r;
				++it;
			}
		} else if (*it == "--angles") {
			list<string>::iterator it2 = it; ++it2;
			if (it2 != args.end()) {
				int r = atoi(it2->c_str());
				ANGLES = r;
				++it;
			}
		} else {
			modelfilename = *it;
		}
	}

	res_y = res_x*3/4;

	class system mysys(1.0, 1000.0, res_x, res_y, fullscreen);
	mysys.set_res_2d(1024, 768);

	// prepare output data file
	string::size_type st = modelfilename.rfind(".");
	if (st == string::npos)
		throw error("invalid module filename");
	string datafilename = modelfilename.substr(0, st) + ".phys";
	xml_doc physdat(datafilename);
	xml_elem physroot = physdat.add_child("dftd-physical-data");
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0,res_x, 0,res_y);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glClearColor(0,0,0,0);
	glColor3f(1.0f, 1.0f, 1.0f);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

	mdl = new model(modelfilename, false);
	mdl->register_layout();
	mdl->set_layout();
	vector3f mmin = mdl->get_min();
	vector3f mmax = mdl->get_max();
	mw = mmax.y - mmin.y;
	mh = mmax.z - mmin.z;
	screenarea_meters = mw * mh;
	xml_elem physcs = physroot.add_child("cross-section");
	physcs.set_attr(ANGLES, "angles");
	ostringstream osscs;

	// set up primary modelview matrix
	glLoadIdentity();
	glScalef(double(res_x)/mw, double(res_y)/mh, 0.0);
	glTranslated(-mmin.y, -mmin.z, 0);

	for (unsigned i = 0; i < ANGLES; ++i) {
		double angle = 360.0 * i / ANGLES;
		measure_model(angle, osscs);
		mysys.poll_event_queue();
	}
	physcs.add_child_text(osscs.str());

	// some measurements
	const vector3f& bmax = mdl->get_mesh(0).max;
	const vector3f& bmin = mdl->get_mesh(0).min;
	const vector3f bsize = bmax - bmin;
	const double vol = bsize.x * bsize.y * bsize.z;

	const vector3i resolution(6, 6, 8);
	vector<uint8_t> is_inside(resolution.x*resolution.y*resolution.z);
	// start workers and let them compute data, then wait for them to finish
	{
		thread::auto_ptr<worker> w0(new worker(*mdl, is_inside, resolution, 0, 2));
		thread::auto_ptr<worker> w1(new worker(*mdl, is_inside, resolution, 1, 2));
		w0->start();
		w1->start();
	}
	cout << "\n";

	unsigned nr_inside = 0;
	ostringstream insidedat;
	for (int z = 0; z < resolution.z; ++z) {
		cout << "Layer " << z+1 << "/" << resolution.z << "\n";
		for (int y = 0; y < resolution.y; ++y) {
			for (int x = 0; x < resolution.x; ++x) {
				uint8_t in = is_inside[(z * resolution.y + y) * resolution.x + x];
				insidedat << hex << setfill('0') << setw(2) << unsigned(in);
				cout << (in >= 128 ? 'X' : (in >= 1 ? 'o' : ' '));
				nr_inside += (in >= 1) ? 1 : 0;
			}
			cout << "\n";
		}
	}
	cout << "Cubes inside: " << nr_inside << " of " << is_inside.size() << "\n";
	xml_elem ve = physroot.add_child("voxels");
	ve.set_attr(resolution.x, "x");
	ve.set_attr(resolution.y, "y");
	ve.set_attr(resolution.z, "z");
	ve.add_child_text(insidedat.str());

	double vol_inside = nr_inside * vol / is_inside.size();
	//cout << "Inside volume " << vol_inside << " (" << vol_inside/2.8317 << " BRT) of " << vol << "\n";
	physroot.add_child("volume").set_attr(vol_inside);
	physroot.add_child("center-of-gravity").set_attr(mdl->get_mesh(0).compute_center_of_gravity());
	matrix3 ten = mdl->get_mesh(0).compute_inertia_tensor();
	ostringstream ossit; ten.to_stream(ossit);
	physroot.add_child("inertia-tensor").add_child_text(ossit.str());
	// better change only the translation? and do not transform other meshes
/*
	mdl->get_mesh(0).transform(matrix4f::trans(-mdl->get_mesh(0).compute_center_of_gravity()));
	physroot.add_child("center-of-gravity2").set_attr(mdl->get_mesh(0).compute_center_of_gravity());
	ten = mdl->get_mesh(0).compute_inertia_tensor();
	ostringstream ossit2; ten.to_stream(ossit2);
	physroot.add_child("inertia-tensor2").add_child_text(ossit2.str());
*/

	physdat.save();

	delete mdl;

	return 0;
}
