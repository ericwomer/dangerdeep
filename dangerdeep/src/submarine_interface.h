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

class submarine_interface : public user_interface
{
protected:
	// Manual angle correction to fire a spread of torpedoes.
	angle lead_angle;
	
	// source tube nr for manual torpedo transfer
	unsigned torptranssrc;
	
	// FAT torpedo programming data
	unsigned primaryrange;		// selected option 0-16 (1600 to 3200m)
	unsigned secondaryrange;	// selected option 0-1 (800 or 1600m)
	unsigned initialturn;		// selected option 0-1 (left or right)
	unsigned searchpattern;		// selected option 0-1 (turn 180 or 90 deg.) fixme what are historical correct patterns?

	class sub_damage_display* sub_damage_disp;

	enum display_mode { display_mode_gauges, display_mode_periscope,
		display_mode_uzo, display_mode_glasses, display_mode_bridge,
		display_mode_map, display_mode_torpedoroom, display_mode_damagestatus,
		display_mode_logbook, display_mode_successes, display_mode_freeview };
	submarine_interface();
	submarine_interface& operator= (const submarine_interface& other);
	submarine_interface(const submarine_interface& other);
	
	// returns true if processed
	virtual bool keyboard_common(int keycode, class system& sys, class game& gm);

	// panel buttons
	class widget_button* btn_menu;

	void draw_torpedo(class system& sys, class game& gm, bool
		usebow, int x, int y, const submarine::stored_torpedo& st);

	// Display functions for screens.
	virtual void display_gauges(class system& sys, class game& gm);
	virtual void display_periscope(class system& sys, class game& gm);
	virtual void display_UZO(class system& sys, class game& gm);
	virtual void display_torpedoroom(class system& sys, class game& gm);
	virtual void display_damagestatus(class system& sys, class game& gm);

public:	
	submarine_interface(submarine* player_sub, class game& gm);
	virtual ~submarine_interface();

	virtual void display(class system& sys, class game& gm);
	virtual void play_sound_effect_distance ( sound_effect se, double distance ) const;
};

#endif
