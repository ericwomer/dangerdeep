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
	image controlscreen_normallight;
	image controlscreen_nightlight;

	struct indicator {
		texture* mytex;
		unsigned x, y, w, h;
		indicator();
		~indicator();
		void display(const double& angle) const;
		void set(SDL_Surface* s, unsigned x_, unsigned y_, unsigned w_, unsigned h_);
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
