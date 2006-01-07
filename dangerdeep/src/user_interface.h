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

// user interface for controlling a sea_object
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include <list>
#include <vector>
#include <map>
using namespace std;
#include "sea_object.h"
#include "global_data.h"
#include "color.h"
#include "coastmap.h"

#include "user_display.h"
#include "user_popup.h"

class game;

class user_interface
{
	user_interface();
public:
	enum color_mode { day_color_mode, night_color_mode };

protected:
	game* mygame;	// pointer to game object that is displayed

	bool pause;
	unsigned time_scale;

	// command panel, to submarine_interface!
	// display texts above panel, fading out, no widget! fixme
	bool panel_visible;
	class widget* panel;
	class widget_list* panel_messages;
	class widget_text* panel_valuetexts[6];

	// used in various screens
	angle bearing;
	angle elevation;	// -90...90 deg (look down ... up)
	bool bearing_is_relative;	// bearing means angle relative to course or absolute? (default = true)

	// fixme: keep this, common data else: panel, sky, water, coastmap
	sea_object* target; // fixme: stored in player now, so remove this

	// which display is active
	mutable unsigned current_display;

	// fixme replace the above with: THE ONE AND ONLY DATA UI SHOULD HAVE
	vector<user_display*> displays;

	// which popup is shown (0 = none)
	mutable unsigned current_popup;

	// possible popups
	vector<user_popup*> popups;

	// environmental data
	class sky* mysky;		// the one and only sky
	class water* mywater;		// the ocean water
	coastmap mycoastmap;	// this may get moved to game.h, yet it is used for display only, that's why it is here

	// weather graphics
	vector<texture*> raintex;	// images (animation) of rain drops
	vector<texture*> snowtex;	// images (animation) of snow flakes

	// free view mode
//	float freeviewsideang, freeviewupang;	// global spectators viewing angles
//	vector3 freeviewpos;

	user_interface& operator= (const user_interface& other);
	user_interface(const user_interface& other);
	user_interface(game& gm);

//	inline virtual sea_object* get_player(void) const { return player_object; }

	// color funtions. ?????fixme
	virtual void set_display_color( color_mode mode ) const;
	virtual void set_display_color() const;
	
//	void draw_clock(game& gm, int x, int y, unsigned wh, double t,
//	        const string& text) const;

	// adjusts "current_popup" if not set to allowed popup
	// must be const because it can be called by display(), only mutable members are changed.
	void set_allowed_popup(void) const;

	// set "current_display" only via this function, so that checks can be performed autom.
	// must be const because it can be called by display(), only mutable members are changed.
	void set_current_display(unsigned curdis) const;

public:	
	virtual ~user_interface();

	// display (const) and input handling
	virtual void display(void) const;

	// set global time for display (needed for water/sky animation)
	void set_time(double tm);

	// process common events (common keys, mouse input to panel)
	virtual void process_input(const SDL_Event& event);
	virtual void process_input(list<SDL_Event>& events);

	// create ui matching to player type (requested from game)
	static user_interface* create(game& gm);

	const sky& get_sky(void) const { return *mysky; }
	const water& get_water(void) const { return *mywater; }
	const coastmap& get_coastmap(void) const { return mycoastmap; }

	// helper functions

	virtual angle get_relative_bearing(void) const;
	virtual angle get_absolute_bearing(void) const;
	virtual angle get_elevation(void) const;
	// add angles to change bearing/elevation
	virtual void add_bearing(angle a);
	virtual void add_elevation(angle a);

	// 2d drawing must be on for this
	void draw_infopanel(void) const;

	// this rotates the modelview matrix to match the water surface normal
	// rollfac (0...1) determines how much the ship is influenced by wave movement
	virtual void rotate_by_pos_and_wave(const vector3& pos, angle heading,
					    float length, float width,
					    double rollfac = 0.05, bool inverse = false) const;

	virtual sea_object* get_target(void) const { return target; }
	virtual void set_target(sea_object* tgt) { target = tgt; }

	// 3d drawing functions
	virtual void draw_terrain(const vector3& viewpos, angle dir, double max_view_dist) const;

	virtual void draw_weather_effects(void) const;

	virtual bool paused(void) const { return pause; }
	virtual unsigned time_scaling(void) const { return time_scale; }
	virtual void add_message(const string& s);
	virtual bool time_scale_up(void);	// returns true on success
	virtual bool time_scale_down(void);
//	virtual void record_sunk_ship ( const class ship* so );
	/** This method creates a message about the rudder state. */
	virtual void add_rudder_message(void);
	virtual void play_sound_effect(const string &se, const sea_object* player, 
								   const sea_object* noise_source, bool loop = false) const;
	virtual void play_fade_sound_effect(const string &se, const sea_object* player, 
										const sea_object* noise_source, bool loop = false) const;
	virtual void stop_sound_effect(const string &se) const;
	virtual void stop_fade_sound_effect(const string &se) const;	
	virtual void pause_all_sound() const;
	virtual void resume_all_sound() const;
};

#endif
