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

// game event
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef EVENT_H
#define EVENT_H

class user_interface;
#include "vector3.h"

/// interface for any event that needs special handling by user interface
class event
{
 public:
	virtual ~event() {}
	virtual void evaluate(user_interface& ui) = 0;
};

/// torpedo dud because range was too short
class event_torpedo_dud_shortrange : public event
{
 public:
	void evaluate(user_interface& ui);
};

/// torpedo dud because of torpedo failure
class event_torpedo_dud : public event
{
 public:
	void evaluate(user_interface& ui);
};

/// ship was sunk
class event_ship_sunk : public event
{
 public:
	void evaluate(user_interface& ui);
};

/// dive preparations
class event_preparing_to_dive : public event
{
 public:
	void evaluate(user_interface& ui);
};

/// diving
class event_diving : public event
{
 public:
	void evaluate(user_interface& ui);
};

/// unmanning deck gun
class event_unmanning_gun : public event
{
 public:
	void evaluate(user_interface& ui);
};

/// deck gun manned and ready
class event_gun_manned : public event
{
 public:
	void evaluate(user_interface& ui);
};

/// deck gun unmanned and secured
class event_gun_unmanned : public event
{
 public:
	void evaluate(user_interface& ui);
};

/// depth charge hitting water surface
class event_depth_charge_in_water : public event
{
	vector3 source;
 public:
	event_depth_charge_in_water(const vector3& src) : source(src) {}
	void evaluate(user_interface& ui);
};

/// depth charge exploding
class event_depth_charge_exploding : public event
{
	vector3 source;
 public:
	event_depth_charge_exploding(const vector3& src) : source(src) {}
	void evaluate(user_interface& ui);
};

/// light gun fires
class event_gunfire_light : public event
{
	vector3 source;
 public:
	event_gunfire_light(const vector3& src) : source(src) {}
	void evaluate(user_interface& ui);
};

/// medium gun fires
class event_gunfire_medium : public event
{
	vector3 source;
 public:
	event_gunfire_medium(const vector3& src) : source(src) {}
	void evaluate(user_interface& ui);
};

/// heavy gun fires
class event_gunfire_heavy : public event
{
	vector3 source;
 public:
	event_gunfire_heavy(const vector3& src) : source(src) {}
	void evaluate(user_interface& ui);
};

/// shell exploding
class event_shell_explosion : public event
{
	vector3 source;
 public:
	event_shell_explosion(const vector3& src) : source(src) {}
	void evaluate(user_interface& ui);
};

/// shell splashes water
class event_shell_splash : public event
{
	vector3 source;
 public:
	event_shell_splash(const vector3& src) : source(src) {}
	void evaluate(user_interface& ui);
};

/// torpedo explodes
class event_torpedo_explosion : public event
{
	vector3 source;
 public:
	event_torpedo_explosion(const vector3& src) : source(src) {}
	void evaluate(user_interface& ui);
};

/// ping is sent in water
class event_ping : public event
{
	vector3 source;
 public:
	event_ping(const vector3& src) : source(src) {}
	void evaluate(user_interface& ui);
};

#endif
