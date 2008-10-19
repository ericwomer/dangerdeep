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

#include "water_splash.h"
#include "global_data.h"
#include "texture.h"
#include "game.h"
#include "global_constants.h"

void water_splash::render_cylinder(double radius_bottom, double radius_top, double height,
				   double alpha, const texture& tex,
				   double u_scal, unsigned nr_segs)
{
	//fixme: use different alpha for bottom? like always full alpha?
	// compensate tide! so set z_lower to -1.5
	primitives::cylinder_z(radius_bottom, radius_top, -1.5, height,
			       alpha, tex, u_scal, nr_segs).render();
}



double water_splash::compute_height(double t) const
{
	if (t <= risetime) {
		double q = t/risetime - 1.0;
		return riseheight * (1.0 - q*q);
	} else {
		double q = t - risetime;
		return riseheight - GRAVITY * 0.5 * q*q;
	}
}



water_splash::water_splash(game& gm_, const vector3& pos, double risetime_, double riseheight_)
	: sea_object(gm_, "gun_shell.3ds" /* hack */),
	  risetime(risetime_), riseheight(riseheight_)
{
	double falltime = sqrt(riseheight * 2.0 / GRAVITY);
	lifetime = risetime + falltime;
	resttime = lifetime;
	position = pos;
	std::vector<double> p(6);
	double fac = riseheight / 25.0;
	p[0] = fac * 5.0;
	p[1] = fac * 5.0;
	p[2] = fac * 6.0;
	p[3] = fac * 7.0;
	p[4] = fac * 8.0;
	p[5] = fac * 9.0;
	bradius_top.reset(new bspline(3, p));
	p[0] = fac * 5.0;
	p[1] = fac * 5.0;
	p[2] = fac * 5.2;
	p[3] = fac * 5.4;
	p[4] = fac * 5.6;
	p[5] = fac * 5.8;
	bradius_bottom.reset(new bspline(3, p));
	p[0] = fac * 1.0;
	p[1] = fac * 1.0;
	p[2] = fac * 0.75;
	p[3] = fac * 0.5;
	p[4] = fac * 0.25;
	p[5] = fac * 0.0;
	balpha.reset(new bspline(3, p));
}



void water_splash::simulate(double delta_time)
{
	sea_object::simulate(delta_time);

	resttime -= delta_time;
	if (resttime <= -0.5)
		kill();
}



void water_splash::display() const
{
	const texture& tex = *texturecache().find("splashring.png");

	if (lifetime - resttime > 0.5) {
		double t = (lifetime - resttime - 0.5)/lifetime;
		double rt = bradius_top->value(t) * 0.8;
		double rb = bradius_bottom->value(t) * 0.8;
		double a = balpha->value(t);
		render_cylinder(rb, rt, compute_height(lifetime - resttime - 0.5) * 1.2, a, tex);
	}
	if (resttime > 0) {
		double t = (lifetime - resttime)/lifetime;
		double rt = bradius_top->value(t);
		double rb = bradius_bottom->value(t);
		double a = balpha->value(t);
		render_cylinder(rb, rt, compute_height(lifetime - resttime), a, tex);
	}
}



void water_splash::display_mirror_clip() const
{
	display();
}
