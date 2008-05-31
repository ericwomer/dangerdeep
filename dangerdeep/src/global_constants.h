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
const double SECOND_IN_METERS = MINUTE_IN_METERS/60.0;

// earth perimeter in meters
const double EARTH_PERIMETER      = 40030173.59;

// Brutto register ton in cubic meters (100 cubic feet, 1 feet = 30.48cm)
const double BRT_VOLUME = 2.8316846592;

#endif
