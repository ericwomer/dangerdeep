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

#include "sub_recogmanual_popup.h"

sub_recogmanual_popup::widget_button_next::widget_button_next(int x, int y, int w, int h, int dir, int& att_page, const std::string& text_,  const std::string& bg_image_, widget* parent_)
	: widget_button(x, y, w, h, text_, parent_, bg_image_), direction(dir), page(att_page)
{
}

void sub_recogmanual_popup::widget_button_next::draw() const
{
	redrawme = false;
	vector2i p = get_pos();
	int bw = int(background->get_width());
	int bh = int(background->get_height());

	if (mouseover==this)
		glColor4f(1.0, 1.0, 1.0, 1.0);
	else
		glColor4f(1.0, 1.0, 1.0, 0.75);
	background->draw(p.x + size.x/2 - bw/2, p.y + size.y/2 - bh/2);

}

void sub_recogmanual_popup::widget_button_next::on_release ()
{
	pressed = false;
	page+=direction;
}

sub_recogmanual_popup::sub_recogmanual_popup(user_interface& ui_) 
	: user_popup(ui_), page(0), btn_left(15, 690, 11, 31, -1, page, "", "BG_btn_left.png"), btn_right(414, 690, 11, 31, 1, page, "", "BG_btn_right.png")
{
	x = 0;
	y = 49;
	background_daylight.reset(new image(get_image_dir() + "shiprecog_popup_daylight.png"));
	background_nightlight.reset(new image(get_image_dir() + "shiprecog_popup_redlight.png"));
	
	std::list<string> ship_ids = data_file_handler::instance().get_ship_list();
	for (list<string>::iterator it = ship_ids.begin(); it != ship_ids.end(); it++) {

		try {
			auto_ptr<image> img(new image(data_file_handler::instance().get_path(*it) + (*it) + "_silhouette.png"));
			silhouettes.push_back(auto_ptr<image>(img));
			
			xml_doc doc(data_file_handler::instance().get_filename(*it));
			doc.load();
			xml_elem elem = doc.child("dftd-ship");
			elem = elem.child("shipmanual");
			displacements.push_back(elem.attr("displacement"));
			lengths.push_back(elem.attr("length"));
			classes.push_back(elem.attr("class"));
			weapons.push_back(elem.attr("weapons"));
			countries.push_back(elem.attr("countries"));			
		} catch (exception& e) {} // no silhouette file was found
	}	
}

bool sub_recogmanual_popup::process_input(class game& gm, const SDL_Event& event)
{
	btn_left.check_for_mouse_event(event);
	btn_right.check_for_mouse_event(event);
	if (page<0) page=0;
	if (page>=(int)silhouettes.size()/3) page--;
	return false;
}

void sub_recogmanual_popup::display(class game& gm) const
{
	sys().prepare_2d_drawing();
	glColor3f(1,1,1);

	bool is_day = gm.is_day_mode();
	if (is_day)
		background_daylight->draw(x, y);
	else
		background_nightlight->draw(x, y);

	int off_x = 15;
	int off_y = 82;
	int off_text_x = 40;
	int off_text_y = 237;
	int step_y = 199;

	for (int i=page*3; (i<page*3+3)&&(i<(int)silhouettes.size()); i++) {

		glColor4f(1.0, 1.0, 1.0, 0.75);
		silhouettes[i]->draw(off_x,off_y+step_y*(i%3));
		glColor4f(1.0, 1.0, 1.0, 1.0);
		
		//fixme: change this after the authentic overlay is implemented
		font_vtremington12->print(off_text_x, off_text_y+step_y*(i%3), classes[i].c_str(), color(0, 0, 0));
		font_vtremington12->print(off_text_x, off_text_y+15+step_y*(i%3), string("Length: ")+lengths[i].c_str()+string("   Displacement:")+displacements[i].c_str(), color(0, 0, 0));
		font_vtremington12->print(off_text_x, off_text_y+30+step_y*(i%3), string("Countries: ")+countries[i].c_str(), color(0, 0, 0));
		font_vtremington12->print(off_text_x, off_text_y+45+step_y*(i%3), string("Weapons: ")+weapons[i].c_str(), color(0, 0, 0));
	}
	
	btn_left.draw();
	btn_right.draw();	
	
	sys().unprepare_2d_drawing();
}
