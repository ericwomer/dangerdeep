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

class submarine_interface : public user_interface
{
protected:

	// the indices for the displays
	enum {
		display_mode_gauges,
		display_mode_periscope,
		display_mode_uzo,
		display_mode_bridge,
		display_mode_map,
		display_mode_torpedoroom,
		display_mode_damagestatus,
		display_mode_logbook,
		display_mode_successes,
		display_mode_freeview,
		display_mode_tdc,
		nr_of_displays
	};

	submarine_interface();
	submarine_interface& operator= (const submarine_interface& other);
	submarine_interface(const submarine_interface& other);

public:
	submarine_interface(class game& gm);
	virtual ~submarine_interface();

	void fire_tube(submarine* player, int nr);

	virtual void display(void) const;
	virtual void process_input(const SDL_Event& events);
};

#endif
