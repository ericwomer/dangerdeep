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

class ship_interface : public user_interface
{
protected:
	ship_interface();
	ship_interface& operator= (const ship_interface& other);
	ship_interface(const ship_interface& other);
	
	// returns true if processed
	bool keyboard_common(int keycode, class system& sys, class game& gm);

	// 2d drawing must be turned on for them
	void draw_gauge(class system& sys, unsigned nr, int x, int y, unsigned wh, angle a,
		const char* text) const;
	void draw_vessel_symbol(class system& sys,
		const vector2& offset, const sea_object* so, color c) const;
	void draw_trail(sea_object* so, const vector2& offset);

	// Display function for screens.
	void display_sonar(class system& sys, class game& gm); // or better display_guns?
	void display_glasses(class system& sys, class game& gm);
	void display_dc_throwers(class system& sys, class game& gm);
	void display_damagestatus(class system& sys, class game& gm);
	
public:	
	ship_interface(ship* player_ship);
	virtual ~ship_interface();

	virtual void display(class system& sys, class game& gm);
};

#endif
