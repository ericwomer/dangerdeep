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

#define MAPGRIDSIZE 1000	// meters
#define WATER_BUMP_FRAMES 256

class game;
class user_display;
class logbook_display;
class ships_sunk_display;

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

	// command panel
	bool panel_visible;
	class widget* panel;
	class widget_list* panel_messages;
	class widget_text* panel_valuetexts[5];

	// used in various screens
	angle bearing;
	angle elevation;	// -90...90 deg (look down ... up)
	unsigned viewmode;
	sea_object* target;

	logbook_display* captains_logbook;
	ships_sunk_display* ships_sunk_disp;

	// periscope
	bool zoom_scope;	// use 6x instead 1.5 fixme implement, fixme: maybe to submarine

	// map
	float mapzoom;	// factor pixel/meter
	vector2 mapclick;
	double mapclickdist;
	vector2 mapoffset;	// additional offset used for display, relative to player

	// sky, clouds
	texture* stars;
	texture* skycolor;
	texture* sunglow;
	texture* clouds;
	// fixme: world data maybe should move to class game
	float cloud_animphase;	// 0-1 phase of interpolation
	vector<vector<Uint8> > noisemaps_0, noisemaps_1;	// interpolate to animate clouds
	unsigned clouds_dl;	// display list for sky hemisphere
	unsigned cloud_levels, cloud_coverage, cloud_sharpness;
	vector<Uint8> cloud_alpha;	// precomputed alpha texture
	vector<unsigned> cloud_interpolate_func;	// give fraction as Uint8
	
//	vector<char> landsea;	// a test hack. 0 = land, 1 = sea
	coastmap mycoastmap;	// this may get moved to game.h, yet it is used for display only, that's why it is here
	list<pair<vector2, string /* fixme maybe unsigned for texts */> > cities;

	// free view mode
	float freeviewsideang, freeviewupang;	// global spectators viewing angles
	vector3 freeviewpos;

	user_interface& operator= (const user_interface& other);
	user_interface(const user_interface& other);
	user_interface(sea_object* player, game& gm);//why not store reference to game? fixme

	texture* water_bumpmaps[WATER_BUMP_FRAMES];
	
	// waves are stored in display lists to speed up drawing.
	// this increases fps > 100% compared to vertex arrays / glDrawElements
	// the display lists can take MUCH ram!
	unsigned wavedisplaylists;		// # of first display list
	vector<vector<float> > wavetileh;	// wave tile heights (generated)
	vector<vector<vector3f> > wavetilen;	// wave tile normals (generated)
	vector<float> wavefoam;			// 2d array with foam values (0-1), maybe use fixed point integer here
	// display lists are const, but we need dynamic data for foam. So we store foam in a texture and update this
	// texture each frame or each 1/10th second. The texture has a texel for each wave vertex and each tile,
	// thus it is #tiles*#vertices_per_tile wide and high. We use an color table indexed texture, so
	// we have to transfer (32*8)^2=64k or (64*16)^2=256k per frame, far less memory than updating geometry data
	// each frame (like with vertex arrays).
	// alternative foam generation: clear texture every frame (or 1/10th second), draw lines from ship trails
	// into the texture (if they're inside) and use this texture
	vector<Uint8> wavefoamtexdata;
	unsigned wavefoamtex;
	
	void init(void);
	void deinit(void);

	inline virtual sea_object* get_player(void) const { return player_object; }
	virtual bool keyboard_common(int keycode, game& gm) = 0;

	// generate new clouds, fac (0-1) gives animation phase. animation is cyclic.
	void advance_cloud_animation(float fac);	// 0-1
	void compute_clouds(void);
	vector<vector<Uint8> > compute_noisemaps(void);
	Uint8 get_value_from_bytemap(unsigned x, unsigned y, unsigned level,
		const vector<Uint8>& nmap);
	void smooth_and_equalize_bytemap(unsigned s, vector<Uint8>& map1);

	static texture* torptex(unsigned type);

	// color funtions.
	virtual void set_display_color ( color_mode mode ) const;
	virtual void set_display_color ( const game& gm ) const;
	
	// 2d drawing must be turned on for them
	void draw_infopanel(game& gm) const;
	void draw_gauge(game& gm, unsigned nr, int x, int y, unsigned wh, angle a,
		const string& text, angle a2) const;
	void draw_gauge(game& gm, unsigned nr, int x, int y, unsigned wh, angle a,
		const string& text) const {
			draw_gauge(gm, nr, x, y, wh, a, text, a);
	}
	// draws turnable switch. parameters: pos, first index and number of descriptions,
	// selected description, extra description text number and title text nr.
	void draw_turnswitch(game& gm, int x, int y,
		unsigned firstdescr, unsigned nrdescr, unsigned selected, unsigned extradescr, unsigned title) const;
	// Matching input function, give pos 0-255,0-255.
	unsigned turnswitch_input(int x, int y, unsigned nrdescr) const;
	
	void draw_manometer_gauge (game& gm, unsigned nr,
		int x, int y, unsigned wh, float value, const string& text ) const;
	void draw_clock(game& gm, int x, int y, unsigned wh, double t,
	        const string& text) const;
	void draw_vessel_symbol(const vector2& offset, sea_object* so, color c);
	void draw_trail(sea_object* so, const vector2& offset);
	virtual void draw_pings(game& gm, const vector2& offset);
	virtual void draw_sound_contact(game& gm, const sea_object* player,
		double max_view_dist, const vector2& offset);
	virtual void draw_visual_contacts(game& gm,
		const sea_object* player, const vector2& offset);
	virtual void draw_square_mark (game& gm,
		const vector2& mark_pos, const vector2& offset, const color& c );

	// Display functions for screens.
	virtual void display_gauges(game& gm);
	virtual void display_bridge(game& gm);
	virtual void display_map(game& gm);
	virtual void display_logbook(game& gm);
	virtual void display_successes(game& gm);
	virtual void display_freeview(game& gm);
	virtual void display_glasses(game& gm);
	virtual void display_damagestatus(game& gm) = 0;

	virtual sound* get_sound_effect ( sound_effect se ) const;

public:	
	virtual ~user_interface();
	virtual void display(game& gm) = 0;

	// create ui matching to player type (requested from game)
	static user_interface* create(game& gm);

	// helper functions
	void update_foam(double deltat);
	void spawn_foam(const vector2& pos);
	
	// this rotates the modelview matrix to match the water surface normal
	// rollfac (0...1) determines how much the ship is influenced by wave movement
	virtual void rotate_by_pos_and_wave(const vector3& pos, double timefac,
		double rollfac = 0.05, bool inverse = false) const;
	// height depends by time factor (wave shift) t in [0...1)
	virtual float get_water_height(const vector2& pos, double t) const;
	// give f as multiplier for difference to (0,0,1)
	virtual vector3f get_water_normal(const vector2& pos, double t, double f = 1.0) const;

	// 3d drawing functions
	virtual void draw_water(const vector3& viewpos, angle dir, double t, double max_view_dist, bool onlyflatwater = false) const;
	virtual void draw_terrain(const vector3& viewpos, angle dir, double max_view_dist) const;
	virtual void draw_view(game& gm, const vector3& viewpos,
		angle dir, angle elev, bool aboard, bool drawbridge, bool withunderwaterweapons);
	virtual bool paused(void) const { return pause; }
	virtual unsigned time_scaling(void) const { return time_scale; }
	virtual void add_message(const string& s);
	virtual void add_captains_log_entry ( game& gm, const string& s);
	virtual bool time_scale_up(void);	// returns true on success
	virtual bool time_scale_down(void);
	virtual void record_sunk_ship ( const ship* so );
	/** This method creates a message about the rudder state. */
	virtual void add_rudder_message();
	virtual void play_sound_effect ( sound_effect se, double volume = 1.0f ) const;
	virtual void play_sound_effect_distance ( sound_effect se, double distance ) const;
};

#endif
