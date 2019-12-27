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

// key name definitions
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef KEYS_H
#define KEYS_H

enum KEY_NAMES {
	KEY_ZOOM_MAP,
	KEY_UNZOOM_MAP,
	KEY_SHOW_GAUGES_SCREEN,
	KEY_SHOW_PERISCOPE_SCREEN,
	KEY_SHOW_UZO_SCREEN,
	KEY_SHOW_BRIDGE_SCREEN,
	KEY_SHOW_MAP_SCREEN,
	KEY_SHOW_TORPEDO_SCREEN,
	KEY_SHOW_DAMAGE_CONTROL_SCREEN,
	KEY_SHOW_LOGBOOK_SCREEN,
	KEY_SHOW_SUCCESS_RECORDS_SCREEN,
	KEY_SHOW_FREEVIEW_SCREEN,
	KEY_RUDDER_LEFT,
	KEY_RUDDER_HARD_LEFT,
	KEY_RUDDER_RIGHT,
	KEY_RUDDER_HARD_RIGHT,
	KEY_RUDDER_UP,
	KEY_RUDDER_HARD_UP,
	KEY_RUDDER_DOWN,
	KEY_RUDDER_HARD_DOWN,
	KEY_CENTER_RUDDERS,
	KEY_THROTTLE_LISTEN,
	KEY_THROTTLE_SLOW,
	KEY_THROTTLE_HALF,
	KEY_THROTTLE_FULL,
	KEY_THROTTLE_FLANK,
	KEY_THROTTLE_STOP,
	KEY_THROTTLE_REVERSE,
	KEY_THROTTLE_REVERSEHALF,
	KEY_THROTTLE_REVERSEFULL,
	KEY_FIRE_TUBE_1,
	KEY_FIRE_TUBE_2,
	KEY_FIRE_TUBE_3,
	KEY_FIRE_TUBE_4,
	KEY_FIRE_TUBE_5,
	KEY_FIRE_TUBE_6,
	KEY_SELECT_TARGET,
	KEY_SCOPE_UP_DOWN,
	KEY_CRASH_DIVE,
	KEY_GO_TO_SNORKEL_DEPTH,
	KEY_TOGGLE_SNORKEL,
	KEY_SET_HEADING_TO_VIEW,
	KEY_IDENTIFY_TARGET,
	KEY_GO_TO_PERISCOPE_DEPTH,
	KEY_GO_TO_SURFACE,
	KEY_FIRE_TORPEDO,
	KEY_SET_VIEW_TO_HEADING,
	KEY_TOGGLE_ZOOM_OF_VIEW,
	KEY_TURN_VIEW_LEFT,
	KEY_TURN_VIEW_LEFT_FAST,
	KEY_TURN_VIEW_RIGHT,
	KEY_TURN_VIEW_RIGHT_FAST,
	KEY_TIME_SCALE_UP,
	KEY_TIME_SCALE_DOWN,
	KEY_FIRE_DECK_GUN,
	KEY_TOGGLE_RELATIVE_BEARING,
	KEY_TOGGLE_MAN_DECK_GUN,
	KEY_SHOW_TDC_SCREEN,
	KEY_TOGGLE_POPUP,
	KEY_SHOW_TORPSETUP_SCREEN,
	KEY_SHOW_TORPEDO_CAMERA,
	KEY_TAKE_SCREENSHOT,
	NR_OF_KEY_IDS
};

struct key_name
{
	unsigned nr;
	const char* name;
};

extern key_name key_names[NR_OF_KEY_IDS];

#endif
