// interface for controlling an airplane
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef AIRPLANE_INTERFACE_H
#define AIRPLANE_INTERFACE_H

#include <list>
#include <vector>
using namespace std;
#include "airplane.h"
#include "global_data.h"
#include "user_interface.h"
#include "color.h"

class airplane_interface : public user_interface
{
protected:
	airplane_interface();
	airplane_interface& operator= (const airplane_interface& other);
	airplane_interface(const airplane_interface& other);
	
	// returns true if processed
	virtual bool keyboard_common(int keycode, class game& gm);

	// Display functions for screens.
	virtual void display_cockpit(class game& gm);

	virtual void display_damagestatus(game& gm) {}

public:	
	airplane_interface(airplane* player_plane, class game& gm);
	virtual ~airplane_interface();

	virtual void display(class game& gm);
};

#endif
