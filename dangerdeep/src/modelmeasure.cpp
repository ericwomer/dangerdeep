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
#include <glu.h>
#include <SDL.h>
#include "mymain.cpp"
#include "primitives.h"

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



void measure_mass_distribution(const std::string& massmapfn, const vector3i& resolution,
			       vector<float>& mass_part, const vector<float>& is_inside)
{
	glClear(GL_COLOR_BUFFER_BIT);
	texture massmap(massmapfn, texture::LINEAR, texture::CLAMP);
	glEnable(GL_TEXTURE_2D);
	sys().prepare_2d_drawing();
	massmap.draw(0, 0, res_x, res_y);
	sys().unprepare_2d_drawing();
	glDisable(GL_TEXTURE_2D);
	unsigned screen_pixels = res_x*res_y;
	vector<Uint8> pic(screen_pixels*3);
	glReadPixels(0, 0, res_x, res_y, GL_RGB, GL_UNSIGNED_BYTE, &pic[0]);
	sys().swap_buffers();
	float allmass = 0;
	for (int z = 0; z < resolution.z; ++z) {
		for (int y = 0; y < resolution.y; ++y) {
			unsigned mass_sum = 0;
			unsigned y0 = res_y*z/resolution.z;
			unsigned y1 = res_y*(z+1)/resolution.z;
			unsigned x0 = res_x*y/resolution.y;
			unsigned x1 = res_x*(y+1)/resolution.y;
			for (unsigned yy = y0; yy < y1; ++yy) {
				for (unsigned xx = x0; xx < x1; ++xx) {
					mass_sum += pic[(yy*res_x+xx)*3];
				}
			}
			float masspart = float(mass_sum)/((x1-x0)*(y1-y0)*255);
			//cout << "y="<<y<<" z="<<z<<" mass_sum="<<mass_sum<<" masspart="<<masspart<<"\n";
			for (int x = 0; x < resolution.x; ++x) {
				float in_part = is_inside[(z * resolution.y + y) * resolution.x + x];
				mass_part[(z * resolution.y + y) * resolution.x + x] = masspart * in_part;
				allmass += masspart * in_part;
			}
		}
	}
	// normalize mass part over voxels
	for (int z = 0; z < resolution.z; ++z) {
		for (int y = 0; y < resolution.y; ++y) {
			for (int x = 0; x < resolution.x; ++x) {
				mass_part[(z * resolution.y + y) * resolution.x + x] /= allmass;
			}
		}
	}
}



class worker : public thread
{
	model& mdl;
	vector<float>& is_inside;
	vector3i resolution;
	unsigned slice;
	unsigned nr_slices;
	int& counter;
	mutex& counter_mtx;
	unsigned samples_per_voxel;
public:
	worker(model& m, vector<float>& ii, const vector3i& res, unsigned s, unsigned nrs,
	       int& ctr, mutex& cm, unsigned samplespervoxel)
		: thread("modelmsr"), mdl(m), is_inside(ii), resolution(res), slice(s), nr_slices(nrs),
		  counter(ctr), counter_mtx(cm), samples_per_voxel(samplespervoxel) {}
	void loop()
	{
		int zmin = resolution.z * slice / nr_slices;
		int zmax = (slice + 1 == nr_slices) ? resolution.z : (resolution.z * (slice+1) / nr_slices);

		const vector3f& bmax = mdl.get_base_mesh().max;
		const vector3f& bmin = mdl.get_base_mesh().min;
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
								bool is_in = mdl.get_base_mesh().is_inside(vector3f(xc2, yc2, zc2));
								inside += is_in ? 1 : 0;
								xc2 += csx2;
							}
							yc2 += csy2;
						}
						zc2 += csz2;
					}
					//cout << "is_inside " << inside << " / " << samples_per_voxel*samples_per_voxel*samples_per_voxel << "\n";
					is_inside[(izz * resolution.y + iyy) * resolution.x + ixx] =
						float(inside) / (samples_per_voxel*samples_per_voxel*samples_per_voxel);
					xc += csx;
				}
				yc += csy;
				mutex_locker ml(counter_mtx);
				--counter;
			}
			zc += csz;
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

	system::create_instance(new class system(1.0, 1000.0, res_x, res_y, fullscreen));
	sys().set_res_2d(1024, 768);

	// prepare output data file
	string::size_type st = modelfilename.rfind(".");
	if (st == string::npos)
		throw error("invalid module filename");
	string datafilename = modelfilename.substr(0, st) + ".phys";
	bool torpedomode = modelfilename.find("torpedo") != string::npos;
	xml_doc physdat(datafilename);
	xml_elem physroot = physdat.add_child("dftd-physical-data");

	if (torpedomode) {
		cout << "*******************************\n";
		cout << "* Using special torpedo mode! *\n";
		cout << "*******************************\n";
	}
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0,res_x, 0,res_y);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D); // trick to render it correctly
	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glClearColor(0,0,0,0);
	glDisable(GL_BLEND);

	mdl = new model(modelfilename, false);
	mdl->register_layout();
	mdl->set_layout();
	vector3f mmin = mdl->get_min();
	vector3f mmax = mdl->get_max();
	// modify min/max so that model is always fully visible, when it rotates.
	mmin = mmin.min(-mmax);
	mmax = -mmin;
	mw = mmax.y - mmin.y;
	mh = mmax.z - mmin.z;
	screenarea_meters = mw * mh;
	xml_elem physcs = physroot.add_child("cross-section");
	physcs.set_attr(ANGLES, "angles");
	ostringstream osscs;

	// set up primary modelview matrix
	glLoadIdentity();
	// do not show model centered at screen but only above waterline for ships
	if (torpedomode) {
		glScalef(double(res_x)/mw, double(res_y)/mh, 0.0);
		glTranslated(-mmin.y, -mmin.z, 0);
	} else {
		glScalef(double(res_x)/mw, double(res_y)*2/mh, 0.0);
		glTranslated(-mmin.y, 0, 0);
	}
	cout << "min=" << mmin << " max=" << mmax << " mw=" << mw << " mh=" << mh << "\n";

	// voxel resolution
	const vector3i resolution = torpedomode ? vector3i(2, 4, 2) : vector3i(5, 7, 7);
	vector<float> mass_part(resolution.x*resolution.y*resolution.z);

	for (unsigned i = 0; i < ANGLES; ++i) {
		double angle = 360.0 * i / ANGLES;
		measure_model(angle, osscs);
		sys().poll_event_queue();
	}
	physcs.add_child_text(osscs.str());

	// some measurements
	const vector3f& bmax = mdl->get_base_mesh().max;
	const vector3f& bmin = mdl->get_base_mesh().min;
	const vector3f bsize = bmax - bmin;
	const double vol = bsize.x * bsize.y * bsize.z;

	vector<float> is_inside(resolution.x*resolution.y*resolution.z);
	// start workers and let them compute data, then wait for them to finish
	unsigned tm0 = sys().millisec();
	{
		mutex ctrmtx;
		int ctr = resolution.z * resolution.y;
		unsigned samples_per_voxel = torpedomode ? 20 : 4;
		thread::auto_ptr<worker> w0(new worker(*mdl, is_inside, resolution, 0, 2, ctr, ctrmtx, samples_per_voxel));
		thread::auto_ptr<worker> w1(new worker(*mdl, is_inside, resolution, 1, 2, ctr, ctrmtx, samples_per_voxel));
		w0->start();
		w1->start();
		sys().prepare_2d_drawing();
		while (ctr > 0) {
			int w = res_x - res_x * ctr / (resolution.z * resolution.y);
			primitive_col<4> qd(GL_QUADS);
			qd.vertices[3] = vector3f(0, 0, 0);
			qd.vertices[2] = vector3f(0, res_y/10, 0);
			qd.vertices[1] = vector3f(w, res_y/10, 0);
			qd.vertices[0] = vector3f(w, 0, 0);
			qd.colors[3] = colorf(1, 0, 0);
			qd.colors[2] = colorf(1, 0, 0);
			qd.colors[1] = colorf(0, 0, 1);
			qd.colors[0] = colorf(0, 0, 1);
			qd.render();
			sys().swap_buffers();
			thread::sleep(100);
		}
		sys().unprepare_2d_drawing();
	}
	cout << "\n";
	unsigned tm1 = sys().millisec();
	cout << "time needed " << tm1-tm0 << "\n";

	unsigned nr_inside = 0;
	double inside_vol = 0;
	ostringstream insidedat;
	ostringstream massdis;
	for (int z = 0; z < resolution.z; ++z) {
		cout << "Layer " << z+1 << "/" << resolution.z << "\n";
		for (int y = 0; y < resolution.y; ++y) {
			for (int x = 0; x < resolution.x; ++x) {
				float in = is_inside[(z * resolution.y + y) * resolution.x + x];
				insidedat << in << " ";
				inside_vol += in;
				cout << (in >= 0.5f ? 'X' : (in > 0.0f ? 'o' : ' '));
				nr_inside += (in > 0.0f) ? 1 : 0;
			}
			cout << "\n";
		}
	}
	cout << "Cubes inside: " << nr_inside << " of " << is_inside.size() << "\n";
	cout << "Sum of volume " << inside_vol << " real " << (inside_vol * vol) / is_inside.size() << "\n";
	xml_elem ve = physroot.add_child("voxels");
	ve.set_attr(resolution.x, "x");
	ve.set_attr(resolution.y, "y");
	ve.set_attr(resolution.z, "z");
	ve.set_attr(nr_inside, "innr");
	ve.set_attr(inside_vol, "invol");
	ve.add_child_text(insidedat.str());

	try {
		ostringstream massdis;
		string massmapfilename = modelfilename.substr(0, st) + ".mass.png";
		measure_mass_distribution(massmapfilename, resolution, mass_part, is_inside);
		for (int z = 0; z < resolution.z; ++z) {
			for (int y = 0; y < resolution.y; ++y) {
				for (int x = 0; x < resolution.x; ++x) {
					massdis << mass_part[(z * resolution.y + y) * resolution.x + x] << " ";
				}
			}
		}
		ve.add_child("mass-distribution").add_child_text(massdis.str());
	}
	catch (std::exception& e) {
		cout << e.what() << "\n";
		mmin = mdl->get_base_mesh().min;
		mmax = mdl->get_base_mesh().max;
		mw = mmax.y - mmin.y;
		mh = mmax.z - mmin.z;
		glLoadIdentity();
		glScalef(double(res_x)/mw, double(res_y)/mh, 0.0);
		cout << "min=" << mmin << " max=" << mmax << " mw=" << mw << " mh=" << mh << "\n";
		// sub's y-axis must point right, so that it fits massmap
		glTranslated(-mmin.y, -mmin.z, 0);
		glRotated(-90.0, 0, 1, 0);
		glRotated(-90.0, 1, 0, 0);
		mdl->get_base_mesh().transformation.inverse().multiply_gl();
		glClear(GL_COLOR_BUFFER_BIT);
		mdl->get_base_mesh().display();
		sys().swap_buffers();
		sys().screenshot(modelfilename.substr(0, st) + ".mass_map_draft");
		thread::sleep(3000);
	}

	double vol_inside = (inside_vol * vol) / is_inside.size();
	//cout << "Inside volume " << vol_inside << " (" << vol_inside/2.8317 << " BRT) of " << vol << "\n";
	physroot.add_child("volume").set_attr(vol_inside);
	physroot.child("volume").set_attr(mdl->get_base_mesh().compute_volume(), "mesh");
	physroot.add_child("center-of-gravity").set_attr(mdl->get_base_mesh().compute_center_of_gravity());
	matrix3 ten = mdl->get_base_mesh().compute_inertia_tensor(mdl->get_base_mesh_transformation());
	ostringstream ossit; ten.to_stream(ossit);
	physroot.add_child("inertia-tensor").add_child_text(ossit.str());

	physdat.save();

	delete mdl;

	return 0;
}
