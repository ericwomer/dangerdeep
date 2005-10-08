// user display: submarine's torpedo room
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef SUB_TORPEDO_DISPLAY_H
#define SUB_TORPEDO_DISPLAY_H

#include "user_display.h"
#include "submarine.h"
#include "vector2.h"
#include "texture.h"

class sub_torpedo_display : public user_display
{
	// source tube nr for manual torpedo transfer, used for drag & drop drawing
	unsigned torptranssrc;

	// textures for drawing the screen
	auto_ptr<texture> torpempty;
	auto_ptr<texture> torpload;
	auto_ptr<texture> torpunload;
	auto_ptr<texture> torp1fat1;
	auto_ptr<texture> torp1lut1;
	auto_ptr<texture> torp1lut2;
	auto_ptr<texture> torp1;
	auto_ptr<texture> torp1practice;
	auto_ptr<texture> torp2;
	auto_ptr<texture> torp3afat2;
	auto_ptr<texture> torp3alut1;
	auto_ptr<texture> torp3alut2;
	auto_ptr<texture> torp3fat2;
	auto_ptr<texture> torp3;
	auto_ptr<texture> torp4;
	auto_ptr<texture> torp5b;
	auto_ptr<texture> torp5;
	auto_ptr<texture> torp6lut1;
	auto_ptr<texture> submodelVIIc;
	auto_ptr<image> background_daylight;
	auto_ptr<image> background_redlight;

	void draw_torpedo(class game& gm, bool usebow, const vector2i& pos, const submarine::stored_torpedo& st) const;

	int mx, my, mb;

	const texture& torptex(unsigned type) const;

	vector<vector2i> get_tubecoords(class submarine* sub) const;

	unsigned get_tube_below_mouse(const vector<vector2i>& tubecoords) const;

public:
	sub_torpedo_display(class user_interface& ui_);

	virtual void display(class game& gm) const;
	virtual void process_input(class game& gm, const SDL_Event& event);
};

#endif
