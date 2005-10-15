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

// pop-ups:
// can be used in various screens, so they should be stored in submarine_interface
// each screen states which popups are allowed when they're displayed and where to place
// the popus.
// input: every mouse event inside the popup rectangle goes to the popup.
// only clicks outside go to the screen.
// should popups receive keyboard messages?
// if yes then let the popup handle them, and pass unhandled keys to the main screen.
// popup-concept could also be declared in user_interface

class submarine_interface : public user_interface
{
public:

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
		display_mode_torpsetup,
		nr_of_displays
	};

	enum {
		popup_mode_control,
		popup_mode_tdc,
		nr_of_popups
	};

 private:
	submarine_interface();
	submarine_interface& operator= (const submarine_interface& other);
	submarine_interface(const submarine_interface& other);

 protected:
	unsigned selected_tube;

 public:
	submarine_interface(class game& gm);
	virtual ~submarine_interface();

	void fire_tube(submarine* player, int nr);

	virtual void display(void) const;
	virtual void process_input(const SDL_Event& events);
	virtual unsigned get_selected_tube() const { return selected_tube; }
	virtual void select_tube(unsigned nr) { selected_tube = nr; }
};

#endif
