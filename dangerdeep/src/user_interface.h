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
#include "sea_object.h"
#include "color.h"
#include "coastmap.h"
#include "ptrvector.h"
#include "user_display.h"
#include "user_popup.h"

class game;

///\defgroup interfaces In-game user interfaces
///\brief Base class for a user interface for playing the game.
///\ingroup interfaces
class user_interface
{
	user_interface();
public:
	enum color_mode { day_color_mode, night_color_mode };

protected:
	game* mygame;	// pointer to game object that is displayed

	bool pause;
	bool abort_request; // by user (back to game menu)
	unsigned time_scale;

	// command panel, to submarine_interface!
	// display texts above panel, fading out, no widget! fixme
	bool panel_visible;
	std::auto_ptr<class widget> panel;
	class widget_text* panel_valuetexts[6];

	// screen selector menu
	std::auto_ptr<class widget> screen_selector;
	bool screen_selector_visible;

	// music playlist
	std::auto_ptr<class widget> music_playlist;
	bool playlist_visible;
	class widget_checkbox* playlist_repeat_checkbox;
	class widget_checkbox* playlist_shuffle_checkbox;
	class widget_checkbox* playlist_mute_checkbox;

	// main menu
	std::auto_ptr<class widget> main_menu;
	bool main_menu_visible;

	/// holds the last n messages. They're displayed above the panel and fading out over time.
	std::list<std::pair<double, std::string> > messages;

	// used in various screens
	angle bearing;
	angle elevation;	// -90...90 deg (look down ... up)
	bool bearing_is_relative;	// bearing means angle relative to course or absolute? (default = true)

	// fixme: keep this, common data else: panel, sky, water, coastmap
	sea_object* target; // fixme: stored in player now, so remove this

	// which display is active
	mutable unsigned current_display;

	// fixme replace the above with: THE ONE AND ONLY DATA UI SHOULD HAVE
	ptrvector<user_display> displays;

	// which popup is shown (0 = none)
	mutable unsigned current_popup;

	// possible popups
	ptrvector<user_popup> popups;

	// environmental data
	std::auto_ptr<class sky> mysky;	// the one and only sky
	std::auto_ptr<class water> mywater;	// the ocean water
	coastmap mycoastmap;	// this may get moved to game.h, yet it is used for display only, that's why it is here

	// is display in day mode (or night/redlight mode)?
	bool daymode;

	// weather graphics
	ptrvector<class texture> raintex;	// images (animation) of rain drops
	ptrvector<class texture> snowtex;	// images (animation) of snow flakes

	// free view mode
//	float freeviewsideang, freeviewupang;	// global spectators viewing angles
//	vector3 freeviewpos;

	user_interface& operator= (const user_interface& other);
	user_interface(const user_interface& other);
	user_interface(game& gm);

//	inline virtual sea_object* get_player() const { return player_object; }

//	void draw_clock(game& gm, int x, int y, unsigned wh, double t,
//	        const string& text) const;

	// adjusts "current_popup" if not set to allowed popup
	// must be const because it can be called by display(), only mutable members are changed.
	void set_allowed_popup() const;

	// set "current_display" only via this function, so that checks can be performed autom.
	// must be const because it can be called by display(), only mutable members are changed.
	void set_current_display(unsigned curdis) const;

	virtual void playlist_mode_changed();
	virtual void playlist_mute();

	virtual void show_screen_selector();
	virtual void toggle_popup();
	virtual void show_playlist();

public:	
	virtual ~user_interface();

	// display (const) and input handling
	virtual void display() const;

	// set global time for display (needed for water/sky animation)
	void set_time(double tm);

	// process common events (common keys, mouse input to panel)
	virtual void process_input(const SDL_Event& event);
	virtual void process_input(std::list<SDL_Event>& events);

	// create ui matching to player type (requested from game)
	static user_interface* create(game& gm);

	const sky& get_sky() const { return *(mysky.get()); }
	const water& get_water() const { return *(mywater.get()); }
	const coastmap& get_coastmap() const { return mycoastmap; }

	// helper functions

	virtual angle get_relative_bearing() const;
	virtual angle get_absolute_bearing() const;
	virtual angle get_elevation() const;
	// add angles to change bearing/elevation
	virtual void add_bearing(angle a);
	virtual void add_elevation(angle a);

	// 2d drawing must be on for this
	void draw_infopanel(bool onlytexts = false) const;

	// this rotates the modelview matrix to match the water surface normal
	// rollfac (0...1) determines how much the ship is influenced by wave movement
	virtual void rotate_by_pos_and_wave(const vector3& pos, angle heading,
					    float length, float width,
					    double rollfac = 0.05, bool inverse = false) const;

	virtual sea_object* get_target() const { return target; }
	virtual void set_target(sea_object* tgt) { target = tgt; }

	// render red triangle for target in view. give viewport coordinates.
	virtual void show_target(double x, double y, double w, double h, const vector3& viewpos);

	// 3d drawing functions
	virtual void draw_terrain(const vector3& viewpos, angle dir, double max_view_dist) const;

	virtual void draw_weather_effects() const;

	virtual void toggle_pause();
	virtual bool paused() const { return pause; }
	virtual unsigned time_scaling() const { return time_scale; }
	virtual void add_message(const std::string& s);
	virtual bool time_scale_up();	// returns true on success
	virtual bool time_scale_down();
//	virtual void record_sunk_ship ( const class ship* so );
	/** This method creates a message about the rudder state. */
	virtual void add_rudder_message();
	virtual void play_sound_effect(const std::string &se,
				       const sea_object* noise_source, bool loop = false) const;
	virtual void play_fade_sound_effect(const std::string &se,
					    const sea_object* noise_source, bool loop = false) const;
	virtual void stop_sound_effect(const std::string &se) const;
	virtual void stop_fade_sound_effect(const std::string &se) const;	
	virtual void pause_all_sound() const;
	virtual void resume_all_sound() const;

	// get current game of user_interface
	virtual game& get_game() { return *mygame; }
	virtual const game& get_game() const { return *mygame; }

	bool abort_requested() const { return abort_request; }
	void request_abort(bool abrt = true) { abort_request = abrt; }
};

#endif
