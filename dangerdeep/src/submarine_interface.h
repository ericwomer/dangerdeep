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
#include "color.h"

#define MAPGRIDSIZE 1000	// meters
#define MAXTRAILNUMBER 20
#define TRAILTIME 10

class submarine_interface : public user_interface
{
protected:
	// periscope
	bool zoom_scope;	// use 6x instead 1.5 fixme implement

	// map
	float mapzoom;	// factor pixel/meter

	// free view mode
	float viewsideang, viewupang;	// global spectators viewing angles
	vector3 viewpos;

	// used in various screens
	angle bearing;
	unsigned viewmode;
	submarine *player;
	ship* target;

	double last_trail_time;	
	
	submarine_interface();
	submarine_interface& operator= (const submarine_interface& other);
	submarine_interface(const submarine_interface& other);
	
	unsigned sunken_ship_tonnage;
	
	list<string> panel_texts;
	void add_panel_text(const string& s);

	// returns true if processed
	bool keyboard_common(int keycode, class system& sys, class game& gm);

//	bool object_visible(sea_object* so, const vector2& dl, const vector2& dr) const;

	static texture* torptex(unsigned type);
	
	// 2d drawing must be turned on for them
	void draw_infopanel(class system& sys) const;
	void draw_gauge(class system& sys, unsigned nr, int x, int y, unsigned wh, angle a,
		const char* text) const;
	void draw_vessel_symbol(class system& sys,
		const vector2& offset, const sea_object* so, color c) const;
	void draw_trail(sea_object* so, const vector2& offset);
	void draw_torpedo(class system& sys, bool usebow, int x, int y,
		const submarine::stored_torpedo& st);

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

public:	
	submarine_interface(submarine* player_sub);
	virtual ~submarine_interface();

	virtual void display(class system& sys, class game& gm);
	virtual void record_ship_tonnage(unsigned tons) { sunken_ship_tonnage += tons; }
};

#endif
