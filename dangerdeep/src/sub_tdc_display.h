// user display: submarine's tdc
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef SUB_TDC_DISPLAY_H
#define SUB_TDC_DISPLAY_H

#include "user_display.h"
#include <vector>
using namespace std;

class sub_tdc_display : public user_display
{
	class image* background_normallight;
//	class image* background_nightlight;

	vector<class texture*> tube_textures_daylight;
//	vector<class texture*> tube_textures_nightlight;

	texture* clock_big_pointer_daylight;
//	texture* clock_big_pointer_nightlight;
	texture* clock_small_pointer_daylight;
//	texture* clock_small_pointer_nightlight;
	
public:
	sub_tdc_display(class user_interface& ui_);
	virtual ~sub_tdc_display();

	//overload for zoom key handling ('y') and TDC input
	virtual void process_input(class game& gm, const SDL_Event& event);
	virtual void display(class game& gm) const;
};

#endif
