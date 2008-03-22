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

// user display: submarine's torpedo room
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "image.h"
#include "game.h"
#include "font.h"
#include "sub_recogmanual_display.h"
#include "submarine_interface.h"
#include "global_data.h"

using namespace std;

sub_recogmanual_display::widget_button_next::widget_button_next(int x, int y, int w, int h, int dir, int& att_page, const std::string& text_,  const std::string& bg_image_, widget* parent_)
	: widget_button(x, y, w, h, text_, parent_, bg_image_), direction(dir), page(att_page)
{
}

void sub_recogmanual_display::widget_button_next::draw() const
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

void sub_recogmanual_display::widget_button_next::on_release ()
{
	pressed = false;
	page+=direction;
}

sub_recogmanual_display::sub_recogmanual_display(user_interface& ui_) :
	user_display(ui_), page(0), btn_left(82, 681, 11, 31, -1, page, "", "BG_btn_left.png"), btn_right(931, 681, 11, 31, 1, page, "", "BG_btn_right.png")
{
}
	
void sub_recogmanual_display::display(class game& gm) const
{
	int off_x = 82;
	int off_y = 82;
	int off_text_x = 112;
	int off_text_y = 237;
	int step_y = 199;
	int step_x = 450;
	
	// draw display without display color.
	glColor4f(1,1,1,1);
	// draw background
	sys().prepare_2d_drawing();

	background->draw(0, 0);

	for (int i=page*3; (i<page*3+6)&&(i<(int)silhouettes.size()); i++) {
		if (i==page*3+3) {
			off_x += step_x;
			off_text_x += step_x;
		}
		glColor4f(1.0, 1.0, 1.0, 0.75);
		silhouettes[i]->draw(off_x,off_y+step_y*(i%3));
		glColor4f(1.0, 1.0, 1.0, 1.0);
		
		//fixme: change this after the authentic overlay is implemented
		font_vtremington12->print(off_text_x, off_text_y+step_y*(i%3), classes[i]->c_str(), color(0, 0, 0));
		font_vtremington12->print(off_text_x, off_text_y+15+step_y*(i%3), string("Length: ")+lengths[i]->c_str()+string("   Displacement:")+displacements[i]->c_str(), color(0, 0, 0));
		font_vtremington12->print(off_text_x, off_text_y+30+step_y*(i%3), string("Countries: ")+countries[i]->c_str(), color(0, 0, 0));
		font_vtremington12->print(off_text_x, off_text_y+45+step_y*(i%3), string("Weapons: ")+weapons[i]->c_str(), color(0, 0, 0));
	}
	
	btn_left.draw();
	btn_right.draw();
	ui.draw_infopanel();
	sys().unprepare_2d_drawing();	
}

void sub_recogmanual_display::process_input(class game& gm, const SDL_Event& event)
{
	btn_left.check_for_mouse_event(event);
	btn_right.check_for_mouse_event(event);
	if (page<0) page=0;
	if (page>=(int)silhouettes.size()/3) page--;
}

void sub_recogmanual_display::enter(bool is_day)
{
	background.reset(new image(get_image_dir() + "shiprecogmanual_background.jpg"));
	
	std::list<string> ship_ids = data_file_handler::instance().get_ship_list();
	for (list<string>::iterator it = ship_ids.begin(); it != ship_ids.end(); it++) {
		try {
			silhouettes.push_back(auto_ptr<image>(new image(data_file_handler::instance().get_path(*it) + (*it) + "_silhouette.png")));
			
			xml_doc doc(data_file_handler::instance().get_filename(*it));
			doc.load();
			xml_elem elem = doc.child("dftd-ship"); // will this get destroyed on leaving function?
			elem = elem.child("shipmanual");
			displacements.push_back(auto_ptr<string>(new string(elem.attr("displacement"))));
			lengths.push_back(auto_ptr<string>(new string(elem.attr("length"))));			
			classes.push_back(auto_ptr<string>(new string(elem.attr("class"))));
			weapons.push_back(auto_ptr<string>(new string(elem.attr("weapons"))));
			countries.push_back(auto_ptr<string>(new string(elem.attr("countries"))));
		}
		catch (xml_error& xe) { //fixme: remove when all .data files are updated
			displacements.push_back(auto_ptr<string>(new string("foo")));
			lengths.push_back(auto_ptr<string>(new string("foo")));		
			classes.push_back(auto_ptr<string>(new string("foo")));
			weapons.push_back(auto_ptr<string>(new string("foo")));
			countries.push_back(auto_ptr<string>(new string("foo")));		
		}
		catch (error& e) {} //fixme: remove when all silhouettes are in place
	}
}

void sub_recogmanual_display::leave()
{
	background.reset();
	silhouettes.clear();
	displacements.clear();
	lengths.clear();
	classes.clear();
	weapons.clear();
	countries.clear();
}
