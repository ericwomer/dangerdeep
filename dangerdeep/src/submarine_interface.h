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

	enum display_mode { display_mode_gauges, display_mode_periscope,
		display_mode_uzo, display_mode_glasses, display_mode_bridge,
		display_mode_map, display_mode_torpedoroom, display_mode_damagecontrol,
		display_mode_logbook, display_mode_successes, display_mode_freeview };
	submarine_interface();
	submarine_interface& operator= (const submarine_interface& other);
	submarine_interface(const submarine_interface& other);
	
	// returns true if processed
	virtual bool keyboard_common(int keycode, class system& sys, class game& gm);

	void draw_torpedo(class system& sys, class game& gm, bool
		usebow, int x, int y, const submarine::stored_torpedo& st);

    // Display functions for screens.
	virtual void display_periscope(class system& sys, class game& gm);
	virtual void display_UZO(class system& sys, class game& gm);
	virtual void display_torpedoroom(class system& sys, class game& gm);

public:	
	submarine_interface(submarine* player_sub);
	virtual ~submarine_interface();

	virtual void display(class system& sys, class game& gm);
	virtual void play_sound_effect ( sound_effect se, double volume = 1.0f ) const;
	virtual void play_sound_effect_distance ( sound_effect se, double distance ) const;
};

#endif
