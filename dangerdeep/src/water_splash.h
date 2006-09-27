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

// water splash (C)+(W) 2006 Thorsten Jordan

#ifndef WATER_SPLASH_H
#define WATER_SPLASH_H

#include "sea_object.h"
#include "bspline.h"
#include <vector>

class water_splash : public sea_object
{
 private:
	water_splash();
	water_splash& operator=(const water_splash& other);
	water_splash(const water_splash& other);

 protected:
	double resttime;
	double risetime;
	double riseheight;
	double falltime;
	double lifetime;
	std::auto_ptr<bspline> bradius_top;
	std::auto_ptr<bspline> bradius_bottom;
	std::auto_ptr<bspline> balpha;

	static void render_cylinder(double radius_bottom, double radius_top, double height,
				    double alpha, double u_scal = 2.0, unsigned nr_segs = 16);

	double compute_height(double t) const;

 public:
	water_splash(game& gm, const vector3& pos);
	void simulate(double delta_time);
	void display() const;
	void display_mirror_clip() const;
	vector3 get_acceleration() const { return vector3(); } // no acceleration
};

#endif
