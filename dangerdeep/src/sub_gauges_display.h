/*
Danger from the Deep - Open source submarine simulation
Copyright (C) 2003-2006  Thorsten Jordan, Luis Barrancos and others.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

// user display: submarine's gauges
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef SUB_GAUGES_DISPLAY_H
#define SUB_GAUGES_DISPLAY_H

#include "user_display.h"

#include "texture.h"
#include "image.h"
#include "angle.h"
#include <vector>

///\brief Class for display and input of submarine's main gauges.
///\ingroup displays
class sub_gauges_display : public user_display
{
	// pointers to images/textures of the interface
	std::auto_ptr<image> controlscreen_normallight;
	std::auto_ptr<image> controlscreen_nightlight;

	struct indicator {
		texture* mytexday;
		texture* mytexnight;
		unsigned x, y, w, h;
		indicator();
		~indicator();
		void display(bool is_day_mode, double angle) const;
		// mytexnight can be 0 if daymode image should always be used (e.g. compass)
		void set(SDL_Surface* sday, SDL_Surface* snight, unsigned x_, unsigned y_, unsigned w_, unsigned h_);
		bool is_over(int mx, int my) const;
		angle get_angle(int mx, int my) const;
	};

	std::vector<indicator> indicators;
// old control screen, now for type II, new one is for type VII, follows
//	enum { compass, battery, compressor, diesel, bow_depth_rudder, stern_depth_rudder,
//	       depth, knots, main_rudder, mt, nr_of_indicators };
	enum { compass, bow_depth_rudder, stern_depth_rudder,
	       depth, knots, main_rudder, mt, nr_of_indicators };

	mutable int throttle_angle;        // mutable because it will be changed in display()

protected:
	int compute_throttle_angle(int throttle_pos) const;

public:
	sub_gauges_display(class user_interface& ui_);
	virtual ~sub_gauges_display();

	virtual void display(class game& gm) const;
	virtual void process_input(class game& gm, const SDL_Event& event);
};

#endif
