// user interface for controlling a submarine
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef SUBMARINE_INTERFACE_H
#define SUBMARINE_INTERFACE_H

#include <list>
#include <vector>
using namespace std;
#include "submarine.h"
#include "global_data.h"
#include "user_interface.h"

#define WAVES 32        // must be 2^x
#define WATERSIZE 64   // must be 2^x
#define WAVESIZE 10	// meters from wave to wave
#define WATERRANGE WATERSIZE*WAVESIZE/2
#define WAVETIDEHEIGHT 1.5      // half amplitude of waves in meters
#define WAVETIDECYCLETIME 2.0

#define MAPGRIDSIZE 1000	// meters
#define MAXTRAILNUMBER 20
#define TRAILTIME 10

class submarine_interface : public user_interface
{
private:
	// periscope
	bool zoom_scope;	// use 6x instead 1.5

	// map
	float mapzoom;	// factor pixel/meter

	// free view mode
	float viewsideang, viewupang;	// global spectators viewing angles
	vector3 viewpos;

	// used in various screens
	angle bearing;
	unsigned viewmode;
	bool quit;		// whishes user to quit?
	bool pause;
	unsigned time_scale;
	submarine *player;
	ship* target;

	double last_trail_time;	
	
	submarine_interface();
	submarine_interface& operator= (const submarine_interface& other);
	submarine_interface(const submarine_interface& other);
	
	static vector<float> allwaveheights;
	static void init_water_data(void);
	static float get_waterheight(int x, int y, int wave);
	static float get_waterheight(float x_, float y_, int wave);	// bilinear sampling

public:	
	submarine_interface(submarine* player_sub);
	virtual ~submarine_interface();

	virtual bool user_quits(void) const { return quit; };
	virtual bool paused(void) const { return pause; };
	virtual void display(class system& sys, class game& gm);
	virtual unsigned time_scaling(void) const { return time_scale; };

private:
	// returns true if processed
	bool keyboard_common(int keycode, class system& sys, class game& gm);

	bool object_visible(sea_object* so, const vector2& dl, const vector2& dr) const;

	static texture* torptex(unsigned type);
	
	// 2d drawing must be turned on for them
	void draw_infopanel(class system& sys) const;
	void draw_gauge(class system& sys, unsigned nr, int x, int y, unsigned wh, angle a,
		const char* text) const;
	void draw_vessel_symbol(class system& sys, unsigned res_x, unsigned res_y,
		const vector2& offset, const sea_object* so, float r, float g, float b) const;
	void draw_trail(sea_object* so, const vector2& offset, unsigned res_x,
	        unsigned res_y);
	void draw_view(class system& sys, class game& gm, const vector3& viewpos,
		bool withplayer);

	void display_gauges(class system& sys, class game& gm);
	void display_periscope(class system& sys, class game& gm);
	void display_UZO(class system& sys, class game& gm);
	void display_bridge(class system& sys, class game& gm);
	void display_map(class system& sys, class game& gm);
	void display_torpedoroom(class system& sys, class game& gm);
	void display_damagecontrol(class system& sys, class game& gm);
	void display_logbook(class system& sys, class game& gm);
	void display_successes(class system& sys, class game& gm);
	void display_freeview(class system& sys, class game& gm);
};

#endif
