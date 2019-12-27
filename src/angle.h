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

// An nautical angle (C)+(W) 2003 Thorsten Jordan

#ifndef ANGLE_H
#define ANGLE_H

#include "vector2.h"

#include <cmath>
#include <iostream>

#ifndef M_PI
#define M_PI 3.1415926535897932
#endif
#ifndef round
inline double round(double d) { return floor(d + 0.5); }
#endif


///\brief This class represents clockwise angles. It is used for rotations or nautical angles
/** note that mathematical angles go counter clockwise and nautical angles
    clockwise. conversion from/to mathematical angles ignores this because
    it is used for conversion of angle differences.
    so an a.rad() does not compute the corresponding mathematical angle
    for an nautical angle a but transform 0...360 degrees to 0...2pi instead. */
class angle
{
	protected:
	double val;
	static double clamped(double d) { return d - 360.0*floor(d/360.0); };
	
	public:
	angle() : val(0) {};
	angle(double d) : val(d) {};
	angle(const vector2& v) { val = (v == vector2(0,0)) ? 0 : 90-atan2(v.y, v.x)*180.0/M_PI; };
	double value() const { return clamped(val); };
	unsigned ui_value() const { return unsigned(clamped(round(val))); };
	unsigned ui_abs_value180() const { return unsigned(fabs(round(value_pm180()))); };
	double rad() const { return value()*M_PI/180.0; };
	double value_pm180() const { double d = clamped(val); return d <= 180.0 ? d : d-360.0; };
	angle operator+(const angle& other) const { return angle(val + other.val); };
	angle operator-(const angle& other) const { return angle(val - other.val); };
	angle operator-() const { return angle(-val); };
	angle operator*(double t) const { return angle(val * t); };
	/// returns true if the turn from "this" to "a" is shorter when done clockwise
	bool is_cw_nearer(const angle& a) const { return clamped(a.val - val) <= 180.0; };
	static angle from_rad(double d) { return angle(d*180.0/M_PI); };
	static angle from_math(double d) { return angle((M_PI/2-d)*180.0/M_PI); };
	angle& operator+=(const angle& other) { val += other.val; return *this; };
	angle& operator-=(const angle& other) { val -= other.val; return *this; };
	double diff(const angle& other) const { double d = clamped(other.val - val); if (d > 180.0) d = 360.0 - d; return d; };
	double diff_in_direction(bool ccw, const angle& other) const { return ccw ? clamped(val - other.val) : clamped(other.val - val); }
	double sin() const { return ::sin(rad()); };
	double cos() const { return ::cos(rad()); };
	double tan() const { return ::tan(rad()); };
	vector2 direction() const { double r = rad(); return vector2(::sin(r), ::cos(r)); };
	bool operator == ( const angle& b ) const { return value() == b.value(); }
	bool operator != ( const angle& b ) const { return value() != b.value(); }
};

inline std::ostream& operator<< (std::ostream& os, const angle& a) { os << a.value(); return os; }

#endif
