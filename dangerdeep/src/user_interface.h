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

class game;
class user_display;

class user_interface
{
	user_interface();
public:
	enum color_mode { day_color_mode, night_color_mode };
	enum sound_effect { se_submarine_torpedo_launch, se_torpedo_detonation };

protected:
	bool pause;
	unsigned time_scale;
	sea_object* player_object;
	
	// command panel, to submarine_interface!
	// display texts above panel, fading out, no widget! fixme
	bool panel_visible;
	class widget* panel;
	class widget_list* panel_messages;
	class widget_text* panel_valuetexts[5];

	// used in various screens, fixme to submarine_interface!
	angle bearing;
	angle elevation;	// -90...90 deg (look down ... up)
	unsigned viewmode;

	// fixme: keep this, common data else: panel, sky, water, coastmap
	sea_object* target;

	// fixme replace the above with: THE ONE AND ONLY DATA UI SHOULD HAVE
	vector<user_display*> displays;

	// periscope
	bool zoom_scope;	// use 6x instead 1.5 fixme implement, fixme: maybe to submarine

	// to submarine_interface... fixme. how do displays handle water,if there are
	//several displays using it?!
	class sky* mysky;		// the one and only sky
	
	class water* mywater;		// the ocean water
	
	coastmap mycoastmap;	// this may get moved to game.h, yet it is used for display only, that's why it is here

	// free view mode
	float freeviewsideang, freeviewupang;	// global spectators viewing angles
	vector3 freeviewpos;

	user_interface& operator= (const user_interface& other);
	user_interface(const user_interface& other);
	user_interface(sea_object* player, game& gm);//why not store reference to game? fixme

	inline virtual sea_object* get_player(void) const { return player_object; }

	// replace by sdl_event handler for keyboard events! fixme
	virtual bool keyboard_common(int keycode, game& gm) = 0;
	//virtual bool process_common_keys(class game& gm, const SDL_Event& event) = 0;

	// color funtions.
	virtual void set_display_color ( color_mode mode ) const;
	virtual void set_display_color ( const game& gm ) const;
	
	// 2d drawing must be turned on for them
	// soon they're senseless (Luis makes new gauges). Are thy used yet?! yes, in scope mode
	void draw_gauge(game& gm, unsigned nr, int x, int y, unsigned wh, angle a,
		const string& text, angle a2) const;
	void draw_gauge(game& gm, unsigned nr, int x, int y, unsigned wh, angle a,
		const string& text) const {
			draw_gauge(gm, nr, x, y, wh, a, text, a);
	}

	void draw_manometer_gauge (game& gm, unsigned nr,
		int x, int y, unsigned wh, float value, const string& text ) const;
	void draw_clock(game& gm, int x, int y, unsigned wh, double t,
	        const string& text) const;

	// Display functions for screens.
	//fixme: we have to divide display and input processing!
	//make user_display-heir for each display and get rid of these functions.
	//virtual void display(class game& gm) const = 0;
	virtual void display_gauges(game& gm);
	virtual void display_bridge(game& gm);
	virtual void display_map(game& gm);
	virtual void display_logbook(game& gm);//ok, own display ready
	virtual void display_successes(game& gm);//ok, own display ready
	virtual void display_freeview(game& gm);//make view3d_display class with heirs free,glasses,bridge,scope?
	virtual void display_glasses(game& gm);
	virtual void display_damagestatus(game& gm) = 0;//ok, own display ready

	virtual sound* get_sound_effect ( sound_effect se ) const;
	
	void init(game& gm);

public:	
	virtual ~user_interface();

	// display (const) and input handling
	virtual void display(game& gm) const = 0;
	virtual void process_input(const list<SDL_Event>& events) = 0;

	// create ui matching to player type (requested from game)
	static user_interface* create(game& gm);

	// helper functions

	// 2d drawing must be on for this
	void draw_infopanel(game& gm) const;

	// this rotates the modelview matrix to match the water surface normal
	// rollfac (0...1) determines how much the ship is influenced by wave movement
	virtual void rotate_by_pos_and_wave(const vector3& pos,
		double rollfac = 0.05, bool inverse = false) const;

	virtual sea_object* get_target(void) const { return target; }
	virtual void set_target(sea_object* tgt) { target = tgt; }

	// 3d drawing functions
	virtual void draw_terrain(const vector3& viewpos, angle dir, double max_view_dist) const;
	//fixme: should be const!!!!!! input handling in another function!!!
	virtual void draw_view(game& gm, const vector3& viewpos,
		int vpx, int vpy, int vpw, int vph,
		angle dir, angle elev, bool aboard, bool drawbridge, bool withunderwaterweapons);
	virtual bool paused(void) const { return pause; }
	virtual unsigned time_scaling(void) const { return time_scale; }
	virtual void add_message(const string& s);
	virtual void add_captains_log_entry ( game& gm, const string& s);
	virtual bool time_scale_up(void);	// returns true on success
	virtual bool time_scale_down(void);
	virtual void record_sunk_ship ( const class ship* so );
	/** This method creates a message about the rudder state. */
	virtual void add_rudder_message();
	virtual void play_sound_effect ( sound_effect se, double volume = 1.0f ) const;
	virtual void play_sound_effect_distance ( sound_effect se, double distance ) const;
};

#endif
