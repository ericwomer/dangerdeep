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

// Lighting system - manages astronomical calculations for sun/moon and lighting
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "lighting_system.h"
#include "global_constants.h"
#include "global_data.h"
#include "matrix4.h"
#include <cmath>

lighting_system::lighting_system() : current_time(0.0) {
}

double lighting_system::compute_light_brightness(const vector3 &viewpos, vector3 &sundir) const {
    // fixme: if sun is blocked by clouds, light must be darker...
    sundir = compute_sun_pos(viewpos).normal();
    // in reality the brightness is equal to sundir.z, but the sun is so bright that
    // we stretch and clamp this value
    double lightbrightness = sundir.z * 2.0;
    if (lightbrightness > 1.0)
        lightbrightness = 1.0;
    if (lightbrightness < 0.0)
        lightbrightness = 0.0;
    // fixme add moon light at night
    return lightbrightness * 0.8 + 0.2; // some ambient value
}

colorf lighting_system::compute_light_color(const vector3 &viewpos) const {
    // fixme: sun color can be yellow/orange at dusk/dawn
    // attempt at having some warm variation at light color, previously it was
    // uniform, so we'll try a function of elevation (sundir.z to be precise)
    // Ratios of R, G, B channels are meant to remain in the orange area
    vector3 sundir;
    double lbrit = compute_light_brightness(viewpos, sundir);
    double color_elevation = sundir.z;
    // check for clamping here...
    double lr = lbrit * (1 - pow(cos(color_elevation + .47), 25));
    double lg = lbrit * (1 - pow(cos(color_elevation + .39), 20));
    double lb = lbrit * (1 - pow(cos(color_elevation + .22), 15));

    return colorf(lr, lg, lb);
}

/*	************** sun and moon *********************
        The model:
        Sun, moon and earth have an local space, moon and earth rotate around their y-axis.
        y-axes are all up, that means earth's y-axis points to the north pole.
        The moon rotates counter clockwise around the earth in 27 1/3 days (one sidereal month).
        The earth rotates counter clockwise around the sun in 365d 5h 48m 46.5s.
        The earth rotates around itself in 23h 56m 4.1s (one sidereal day).
        Earths rotational axis is rotated by 23.45 degrees.
        Moon orbits in a plane that is 5,15 degress rotated to the xz-plane (plane that
        earth rotates in, sun orbit). The moon is at its southmost position when it is a full moon
        Due to the earth rotation around the sun, the days/months appear longer (the earth
        rotation must compensate the movement).
        So the experienced lengths are 24h for a day and 29.5306 days for a full moon cycle.
        Earth rotational axis points towards the sun at top of summer on the northern hemisphere
        (around 21st. of June).
        On top of summer (northern hemisphere) the earth orbit pos is 0.
        On midnight at longitude 0, the earth rotation is 0.
        At a full moon the moon rotation/orbit position is 0.
        As result the earth takes ~ 366 rotations per year (365d 5h 48m 46.5s / 23h 56m 4.09s = 366.2422)
        We need the exact values/configuration on 1.1.1939, 0:0am.
        And we need the configuration of the moon rotational plane at this date and time.
*/

/*
what has to be fixed for sun/earth/moon simulation:
get exact distances and diameters (done)
get exact rotation times (sidereal day, not solar day) for earth and moon (done)
get exact orbit times for earth and moon around sun / earth (done)
get angle of rotational axes for earth and moon (fixme, 23.45 and 5.15) (done)
get direction of rotation for earth and moon relative to each other (done)
get position of objects and axis states for a fix date (optimum 1.1.1939) (!only moon needed, fixme!)
compute formulas for determining the positions for the following years (fixme)
write code that computes sun/moon pos relative to earth and relative to local coordinates (fixme)
draw moon with phases (fixme)
*/

vector3 lighting_system::compute_sun_pos(const vector3 &viewpos) const {
    double yearang = 360.0 * myfrac((current_time + 10 * 86400) / EARTH_ORBIT_TIME);
    double dayang = 360.0 * (viewpos.x / EARTH_PERIMETER + myfrac(current_time / 86400.0));
    double longang = 360.0 * viewpos.y / EARTH_PERIMETER;
    matrix4 sun2earth =
        matrix4::rot_y(-90.0) *
        matrix4::rot_z(-longang) *
        matrix4::rot_y(-(yearang + dayang)) *
        matrix4::rot_z(EARTH_ROT_AXIS_ANGLE) *
        matrix4::rot_y(yearang) *
        matrix4::trans(-EARTH_SUN_DISTANCE, 0, 0) *
        matrix4::rot_y(-yearang);
    return sun2earth.column3(3);
}

vector3 lighting_system::compute_moon_pos(const vector3 &viewpos) const {
    double yearang = 360.0 * myfrac((current_time + 10 * 86400) / EARTH_ORBIT_TIME);
    double dayang = 360.0 * (viewpos.x / EARTH_PERIMETER + myfrac(current_time / 86400.0));
    double longang = 360.0 * viewpos.y / EARTH_PERIMETER;
    double monthang = 360.0 * myfrac(current_time / MOON_ORBIT_TIME_SYNODIC) + MOON_POS_ADJUST;

    matrix4 moon2earth =
        matrix4::rot_y(-90.0) *
        matrix4::rot_z(-longang) *
        matrix4::rot_y(-(yearang + dayang)) *
        matrix4::rot_z(EARTH_ROT_AXIS_ANGLE) *
        matrix4::rot_y(yearang) *
        matrix4::rot_z(-MOON_ORBIT_AXIS_ANGLE) *
        matrix4::rot_y(monthang + MOON_POS_ADJUST) *
        matrix4::trans(MOON_EARTH_DISTANCE, 0, 0);

    return moon2earth.column3(3);
}

bool lighting_system::is_day_mode(const vector3 &viewpos) const {
    vector3 sundir;
    double br = compute_light_brightness(viewpos, sundir);
    return (br > 0.3); // fixme: a bit crude. brightness has 0.2 ambient...
}
