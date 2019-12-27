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

#ifndef SUB_RECOGMANUAL_POPUP_H
#define SUB_RECOGMANUAL_POPUP_H

#include "user_popup.h"
#include "image.h"
#include "system.h"
#include "game.h"
#include "datadirs.h"
#include "widget.h"
#include "ptrvector.h"
#include "global_data.h"
#include "log.h"

class sub_recogmanual_popup : public user_popup
{
protected:
	class widget_button_next : public widget_button 
	{
	protected:
		int direction;
		int& page;
	public:
		widget_button_next(int x, int y, int w, int h, int dir, int& att_page, const std::string& text_,  const std::string& bg_image_, widget* parent_ =0);
		void draw() const;
		void on_release ();		
	};
	
	int page;
	std::auto_ptr<image> background_daylight;
	std::auto_ptr<image> background_nightlight;
	ptrvector<image> silhouettes;
	std::vector<std::string> classes;
	std::vector<std::string> lengths;
	std::vector<std::string> displacements;
	std::vector<std::string> weapons;
	std::vector<std::string> countries;
	widget_button_next btn_left;
	widget_button_next btn_right;	

public:
	sub_recogmanual_popup(class user_interface& ui_);

	void display(class game& gm) const;

	bool process_input(class game& gm, const SDL_Event& event);
};

#endif /* SUB_RECOGMANUAL_POPUP_H */
