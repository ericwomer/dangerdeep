// user display: submarine's torpedo room
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef SUB_TORPEDO_DISPLAY_H
#define SUB_TORPEDO_DISPLAY_H

#include "user_display.h"
#include "vector2.h"

class sub_torpedo_display : public user_display
{
	// source tube nr for manual torpedo transfer, used for drag & drop drawing
	unsigned torptranssrc;

	void draw_torpedo(class game& gm, bool
		usebow, const vector2i& pos, const submarine::stored_torpedo& st);

	static texture* torptex(unsigned type);

	// draws turnable switch. parameters: pos, first index and number of descriptions,
	// selected description, extra description text number and title text nr.
	// could be replaced by lighted 3d switch!
	void draw_turnswitch(game& gm, int x, int y, unsigned firstdescr, unsigned nrdescr,
			     unsigned selected, unsigned extradescr, unsigned title) const;
	// Matching input function, give pos 0-255,0-255.
	unsigned turnswitch_input(int x, int y, unsigned nrdescr) const;

public:
	sub_torpedo_display ();
	virtual ~sub_torpedo_display ();

	virtual void display(class game& gm) const;
	virtual void process_input(class game& gm, const SDL_Event& event);
};

#endif
