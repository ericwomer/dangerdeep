// user display: submarine's torpedo setup display
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef SUB_TORPSETUP_DISPLAY_H
#define SUB_TORPSETUP_DISPLAY_H

#include "user_display.h"
#include "image.h"
#include <vector>

class sub_torpsetup_display : public user_display
{
	class scheme {
	public:
		auto_ptr<image> background;
		rotat_tex rundepthptr;
		rotat_tex secondaryrangeptr;
		rotat_tex primaryrangeptr;
		rotat_tex torpspeeddial;
		rotat_tex turnangledial;
		rotat_tex primaryrangedial;
		// everything that does not rotate could also be an "image"...
		// but only when this doesn't trash the image cache
		auto_ptr<texture> torpspeed[3];	// slow/medium/fast
		auto_ptr<texture> firstturn[2];	// left/right
		auto_ptr<texture> secondaryrange[2];	// 800/1600m
		auto_ptr<texture> preheating[2];	// on/off
		auto_ptr<texture> temperaturescale;
		rotat_tex primaryrangeknob[6];
		rotat_tex turnangleknob[6];
		rotat_tex rundepthknob[6];
		scheme() {}
	protected:
		scheme(const scheme& );
		scheme& operator= (const scheme& );
	};

	enum turnknobtype {
		TK_NONE = -1,
		TK_PRIMARYRANGE = 0,
		TK_TURNANGLE = 1,
		TK_RUNDEPTH = 2,
		TK_NR = 3
	};

	scheme daylight, redlight;

	turnknobtype turnknobdrag;
	std::vector<float> turnknobang;

	/*
	unsigned selected_tube;	// 0-5
	unsigned selected_mode;	// 0-1 (automatic on / off)
	*/

public:
	sub_torpsetup_display(class user_interface& ui_);

	virtual void process_input(class game& gm, const SDL_Event& event);
	virtual void display(class game& gm) const;
};

#endif
