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

#define WAVES 32        // must be 2^x
#define WATERSIZE 64   // must be 2^x
#define WAVESIZE 10	// meters from wave to wave
#define WATERRANGE WATERSIZE*WAVESIZE/2
#define WAVETIDEHEIGHT 1.5      // half amplitude of waves in meters
#define WAVETIDECYCLETIME 2.0

#define MAPGRIDSIZE 1000	// meters

class user_interface
{
protected:
	bool quit;		// whishes user to quit?
	bool pause;
	unsigned time_scale;
    sea_object* player_object;
   	list<string> panel_texts;
    list<unsigned> tonnage_sunk;

   	// used in various screens
	angle bearing;
	unsigned viewmode;
	sea_object* target;

   	// periscope
	bool zoom_scope;	// use 6x instead 1.5 fixme implement

	// map
	float mapzoom;	// factor pixel/meter

	// free view mode
	float viewsideang, viewupang;	// global spectators viewing angles
	vector3 viewpos;
            
	user_interface();
	user_interface& operator= (const user_interface& other);
	user_interface(const user_interface& other);
   	user_interface(sea_object* player);

	static vector<float> allwaveheights;
	static void init_water_data(void);
	static float get_waterheight(int x, int y, int wave);
	static float get_waterheight(float x_, float y_, int wave);	// bilinear sampling
    
  	inline virtual sea_object* get_player(void) const { return player_object; }
    virtual bool keyboard_common(int keycode, class system& sys, class game& gm) = 0;

	static texture* torptex(unsigned type);

   	// 2d drawing must be turned on for them
	void draw_infopanel(class system& sys) const;
	void draw_gauge(class system& sys, unsigned nr, int x, int y, unsigned wh, angle a,
		const char* text) const;
	void draw_clock(class system& sys, int x, int y, unsigned wh, double t,
	        const char* text) const;
	void draw_vessel_symbol(class system& sys, const vector2& offset, 
                                const sea_object* so, color c) const;
	void draw_trail(sea_object* so, const vector2& offset);
    virtual void draw_pings(class game& gm, const vector2& offset);
    virtual void draw_sound_contact(class game& gm, const sea_object* player,
        const double& max_view_dist);
    virtual void draw_visual_contacts(class system& sys, class game& gm,
    const sea_object* player, const vector2& offset);

    // Display functions for screens.
	virtual void display_gauges(class system& sys, class game& gm);
	virtual void display_bridge(class system& sys, class game& gm);
	virtual void display_map(class system& sys, class game& gm);
	virtual void display_damagecontrol(class system& sys, class game& gm);
	virtual void display_logbook(class system& sys, class game& gm);
	virtual void display_successes(class system& sys, class game& gm);
	virtual void display_freeview(class system& sys, class game& gm);

public:	
	virtual ~user_interface() {};
	virtual void display(class system& sys, class game& gm) = 0;
	virtual void draw_view(class system& sys, class game& gm, const vector3& viewpos,
		angle direction, bool withplayer, bool withunderwaterweapons);
	virtual bool user_quits(void) const { return quit; }
	virtual bool paused(void) const { return pause; }
	virtual unsigned time_scaling(void) const { return time_scale; }
	virtual void add_message(const string& s);
	virtual bool time_scale_up(void);	// returns true on success
	virtual bool time_scale_down(void);
    virtual void record_ship_tonnage(unsigned tons) { tonnage_sunk.push_back(tons); }
};

#endif
