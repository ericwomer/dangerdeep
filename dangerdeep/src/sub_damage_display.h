// Object to display the damage status of a submarine.
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef SUB_DAMAGE_DISPLAY_H
#define SUB_DAMAGE_DISPLAY_H

#include "user_display.h"

#include "vector3.h"
#include "submarine.h"
#include <vector>
using namespace std;

class sub_damage_display : public user_display
{
	texture repairlight, repairmedium, repairheavy, repaircritical, repairwrecked;
	int mx, my;	// last mouse position, needed for popup display
public:
	sub_damage_display(class user_interface& ui_);
	virtual ~sub_damage_display();

	virtual void display_popup (int x, int y, const string& text, bool atleft, bool atbottom) const;

	virtual void display(class game& gm) const;
	virtual void process_input(class game& gm, const SDL_Event& event);
};

#endif /* SUB_DAMAGE_DISPLAY_H */
