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

// global constants
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef GLOBAL_CONSTANTS_H
#define GLOBAL_CONSTANTS_H

// a very "global" constant
const double GRAVITY = 9.806;

// computed with an earth perimeter of 40030.17359km
const double DEGREE_IN_METERS = 111194.9266388889;
const double MINUTE_IN_METERS = 1853.248777315;
const double SECOND_IN_METERS = 30.887479622;

// earth perimeter in meters
const double EARTH_PERIMETER = 40030173.59;

// Brutto register ton in cubic meters (100 cubic feet, 1 feet = 30.48cm)
const double BRT_VOLUME = 2.8316846592;

const double WGS84_A = 6378137.0;
const double WGS84_B = 6356752.314;
const double WGS84_K = sqrt(WGS84_A*WGS84_A-WGS84_B*WGS84_B)/WGS84_A;

// earth radius in meters (radius of a globe with same volume as the GRS 80 ellipsoide)
const double EARTH_RADIUS = 6371000.785;			// 6371km
const double SUN_RADIUS = 696e6;			// 696.000km
const double MOON_RADIUS = 1.738e6;			// 1738km
const double EARTH_SUN_DISTANCE = 149600e6;		// 149.6 million km.
const double MOON_EARTH_DISTANCE = 384.4e6;		// 384.000km
const double EARTH_ROT_AXIS_ANGLE = 23.45;		// degrees.
const double MOON_ORBIT_TIME_SIDEREAL = 27.3333333 * 86400.0;	// sidereal month is 27 1/3 days
const double MOON_ORBIT_TIME_SYNODIC = 29.5306 * 86400.0;	// synodic month is 29.5306 days
//more precise values:
//29.53058867
//new moon was on 18/11/1998 9:36:00 pm
const double MOON_ORBIT_AXIS_ANGLE = 5.15;		// degrees
const double EARTH_ROTATION_TIME = 86164.09;		// 23h56m4.09s, one sidereal day!
const double EARTH_ORBIT_TIME = 31556926.5;		// in seconds. 365 days, 5 hours, 48 minutes, 46.5 seconds

const double MOON_POS_ADJUST = 300.0;	// in degrees. Moon pos in its orbit on 1.1.1939 fixme: research the value

#endif
