// user display: submarine's gauges
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef SUB_GAUGES_DISPLAY_H
#define SUB_GAUGES_DISPLAY_H

#include "user_display.h"

#include "texture.h"
#include "angle.h"
#include <vector>
using namespace std;

class image;

class sub_gauges_display : public user_display
{
	// pointers to images/textures of the interface
	image::ptr controlscreen_normallight;
	image::ptr controlscreen_nightlight;

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

	vector<indicator> indicators;

	enum { compass, battery, compressor, diesel, bow_depth_rudder, stern_depth_rudder,
	       depth, knots, main_rudder, mt, nr_of_indicators };

public:
	sub_gauges_display(class user_interface& ui_);
	virtual ~sub_gauges_display();

	virtual void display(class game& gm) const;
	virtual void process_input(class game& gm, const SDL_Event& event);
};

#endif
