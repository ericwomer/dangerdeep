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

	enum display_mode { display_mode_gauges, display_mode_periscope,
		display_mode_uzo, display_mode_glasses, display_mode_bridge,
		display_mode_map, display_mode_torpedoroom, display_mode_damagestatus,
		display_mode_logbook, display_mode_successes, display_mode_freeview };
	submarine_interface();
	submarine_interface& operator= (const submarine_interface& other);
	submarine_interface(const submarine_interface& other);
	
public:	
	submarine_interface(class game& gm);
	virtual ~submarine_interface();

	virtual void display(class game& gm) const;
	virtual void process_input(class game& gm, const SDL_Event& events);
};

#endif
