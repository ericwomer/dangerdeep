// Object to display the damage status of a submarine.
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "user_display.h"

#ifndef SUB_DAMAGE_DISPLAY_H
#define SUB_DAMAGE_DISPLAY_H

#include "vector3.h"
#include <vector>
using namespace std;

class sub_damage_display : public user_display
{
	// fixme: replace german names by correct translations
	enum damageable_parts {
		outer_stern_tubes,
		rudder,
		screws,
		screw_shaft,
		stern_dive_planes,
		inner_stern_tubes,
		stern_lenzpumpe,	//?
		stern_pressure_hull,
		stern_hatch,
		electric_engines,
		air_compressor,
		maschine_lenzpumpe,	//?
		maschine_pressure_hull,
		stern_battery,
		diesel_engines,
		kombuese_hatch,		//?	// there was only one hatch at the stern!?
		snorkel,
		trimm_tank_valves,	//?
		bow_battery,
		periscope,
		central_pressure_hull,
		bilge_lenzpumpe,	//?
		conning_tower_hatch,
		listening_device,
		radio_device,
		inner_bow_tubes,
		outer_bow_tubes,
		bow_lenzpumpe,		//?
		bow_hatch,
		bow_pressure_hull,
		bow_dive_planes,
		deck_gun,
		aa_gun,
		ammunition_depot,
		radio_detection_device,
		outer_fuel_tanks,
		
		nr_of_damageable_parts	// trick to count enum entries
		
	};
	
	enum damage {
		none,
		light,
		medium,
		heavy,
		critical,
		wrecked
	};
	
	vector<damage> all_parts;
	
public:
	sub_damage_display ();	// fixme: give sub type
	virtual ~sub_damage_display ();

	// give relative direction and amount between 0 and 1.
	virtual void add_damage ( const vector3& fromwhere, float amount );
	
	virtual void display_popup (int x, int y, const string& text) const;

	virtual void display ( class system& sys, class game& gm );
	virtual void check_key ( int keycode, class system& sys, class game& gm );
	virtual void check_mouse ( int x, int y, int mb );
};

#endif /* SUB_DAMAGE_DISPLAY_H */
