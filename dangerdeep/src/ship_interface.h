// user interface for controlling a ship
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef SHIP_INTERFACE_H
#define SHIP_INTERFACE_H

#include <list>
#include <vector>
using namespace std;
#include "ship.h"
#include "global_data.h"
#include "user_interface.h"
#include "color.h"

#define MAPGRIDSIZE 1000	// meters
#define MAXTRAILNUMBER 20
#define TRAILTIME 10

class ship_interface : public user_interface
{
protected:
	bool zoom_glasses;	// fixme implement

	// map
	float mapzoom;	// factor pixel/meter

	// free view mode
	float viewsideang, viewupang;	// global spectators viewing angles
	vector3 viewpos;

	// used in various screens
	angle bearing;
	unsigned viewmode;
	ship* player;
	class submarine* target;

	double last_trail_time;	
	
	ship_interface();
	ship_interface& operator= (const ship_interface& other);
	ship_interface(const ship_interface& other);
	
	list<string> panel_texts;
	void add_panel_text(const string& s);

	// returns true if processed
	bool keyboard_common(int keycode, class system& sys, class game& gm);

//	bool object_visible(sea_object* so, const vector2& dl, const vector2& dr) const;

//	static texture* torptex(unsigned type);
	
	// 2d drawing must be turned on for them
	void draw_infopanel(class system& sys) const;
	void draw_gauge(class system& sys, unsigned nr, int x, int y, unsigned wh, angle a,
		const char* text) const;
	void draw_vessel_symbol(class system& sys,
		const vector2& offset, const sea_object* so, color c) const;
	void draw_trail(sea_object* so, const vector2& offset);
//	void draw_torpedo(class system& sys, bool usebow, int x, int y,
//		const ship::stored_torpedo& st);

	void display_gauges(class system& sys, class game& gm);
	void display_sonar(class system& sys, class game& gm); // or better display_guns?
	void display_glasses(class system& sys, class game& gm);
	void display_bridge(class system& sys, class game& gm);
	void display_map(class system& sys, class game& gm);
	void display_dc_throwers(class system& sys, class game& gm);
	void display_damagecontrol(class system& sys, class game& gm);
	void display_logbook(class system& sys, class game& gm);
	void display_successes(class system& sys, class game& gm);
	void display_freeview(class system& sys, class game& gm);

public:	
	ship_interface(ship* player_ship);
	virtual ~ship_interface();

	virtual void display(class system& sys, class game& gm);
};

#endif
