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

#include "system.h"
#include "image.h"
#include "texture.h"
#include "game.h"
#include "font.h"
#include "sub_torpedo_display.h"
#include "texts.h"
#include "torpedo.h"
#include "user_interface.h"
#include "global_data.h"
#include <sstream>
#include <fstream>
#include <iomanip>
using namespace std;

#define ILLEGAL_TUBE 0xffffffff



sub_torpedo_display::desc_text::desc_text(const std::string& filename)
{
	ifstream ifs(filename.c_str(), ios::in);
	if (ifs.fail())
		throw error(string("couldn't open ") + filename);

	// read lines.
	while (!ifs.eof()) {
		string s;
		getline(ifs, s);
		txtlines.push_back(s);
	}
}



string sub_torpedo_display::desc_text::str(unsigned startline, unsigned nrlines) const
{
	startline = std::min(startline, unsigned(txtlines.size()));
	unsigned endline = std::min(startline + nrlines, unsigned(txtlines.size()));
	string result;
	for (unsigned i = startline; i < endline; ++i)
		result += txtlines[i] + "\n";
	return result;
}



void sub_torpedo_display::draw_torpedo(class game& gm, bool usebow,
	const vector2i& pos, const submarine::stored_torpedo& st) const
{
	if (usebow) {
		if (st.status == 0) {	// empty
			torpempty->draw(pos.x, pos.y);
		} else if (st.status == 1) {	// reloading
			torptex(st.torp->get_specfilename()).draw(pos.x, pos.y);
			torpload->draw(pos.x, pos.y);
		} else if (st.status == 2) {	// unloading
			torpempty->draw(pos.x, pos.y);
			torpunload->draw(pos.x, pos.y);
		} else {		// loaded
			torptex(st.torp->get_specfilename()).draw(pos.x, pos.y);
		}
	} else {
		if (st.status == 0) {	// empty
			torpempty->draw_hm(pos.x, pos.y);
		} else if (st.status == 1) {	// reloading
			torptex(st.torp->get_specfilename()).draw_hm(pos.x, pos.y);
			torpload->draw_hm(pos.x, pos.y);
		} else if (st.status == 2) {	// unloading
			torpempty->draw_hm(pos.x, pos.y);
			torpunload->draw_hm(pos.x, pos.y);
		} else {		// loaded
			torptex(st.torp->get_specfilename()).draw_hm(pos.x, pos.y);
		}
	}
}



const texture& sub_torpedo_display::torptex(const string& torpname) const
{
	if (torpname == "TI") return *torp1;
	if (torpname == "TI_FaTI") return *torp1fat1;
	if (torpname == "TI_LuTI") return *torp1lut1;
	if (torpname == "TI_LuTII") return *torp1lut2;
	if (torpname == "TII") return *torp2;
	if (torpname == "TIII") return *torp3;
	if (torpname == "TIII_FaTII") return *torp3fat2;
	if (torpname == "TIIIa_FaTII") return *torp3afat2;
	if (torpname == "TIIIa_LuTI") return *torp3alut1;
	if (torpname == "TIIIa_LuTII") return *torp3alut2;
	if (torpname == "TIV") return *torp4;
	if (torpname == "TV") return *torp5;
	if (torpname == "TVb") return *torp5b;
	if (torpname == "TVI_LuTI") return *torp6lut1;
	if (torpname == "TXI") return *torp1practice;
	throw error(string("illegal torpedo type ") + torpname);
}



vector<vector2i> sub_torpedo_display::get_tubecoords(submarine* sub) const
{
	vector<vector2i> tubecoords(sub->get_torpedoes().size());
	pair<unsigned, unsigned> bow_tube_indices = sub->get_bow_tube_indices();
	pair<unsigned, unsigned> stern_tube_indices = sub->get_stern_tube_indices();
	pair<unsigned, unsigned> bow_reserve_indices = sub->get_bow_reserve_indices();
	pair<unsigned, unsigned> stern_reserve_indices = sub->get_stern_reserve_indices();
	pair<unsigned, unsigned> bow_deckreserve_indices = sub->get_bow_deckreserve_indices();
	pair<unsigned, unsigned> stern_deckreserve_indices = sub->get_stern_deckreserve_indices();
	unsigned k = bow_tube_indices.second - bow_tube_indices.first;
	for (unsigned i = bow_tube_indices.first; i < bow_tube_indices.second; ++i) {
		tubecoords[i] = vector2i(23, 188+(i-k/2)*13);
	}
	for (unsigned i = bow_reserve_indices.first; i < bow_reserve_indices.second; ++i) {
		unsigned j = i - bow_reserve_indices.first;
		tubecoords[i] = vector2i(161+(j/k)*138, 188+(j%k-k/2)*13);
	}
	for (unsigned i = bow_deckreserve_indices.first; i < bow_deckreserve_indices.second; ++i) {
		unsigned j = i - bow_deckreserve_indices.first;
		tubecoords[i] = vector2i(161+(j/2)*138, 145+(j%2)*13);
	}
	for (unsigned i = stern_tube_indices.first; i < stern_tube_indices.second; ++i) {
		unsigned j = i - stern_tube_indices.first;
		tubecoords[i] = vector2i(823, 188+j*13);
	}
	for (unsigned i = stern_reserve_indices.first; i < stern_reserve_indices.second; ++i) {
		unsigned j = i - stern_reserve_indices.first;
		tubecoords[i] = vector2i(684, 188+j*13);
	}
	for (unsigned i = stern_deckreserve_indices.first; i < stern_deckreserve_indices.second; ++i) {
		unsigned j = i - stern_deckreserve_indices.first;
		tubecoords[i] = vector2i(684-(j/2)*138, 145+(j%2)*13);
	}
	return tubecoords;
}



unsigned sub_torpedo_display::get_tube_below_mouse(const vector<vector2i>& tubecoords) const
{
	for (unsigned i = 0; i < tubecoords.size(); ++i) {
		if (mx >= tubecoords[i].x && mx < tubecoords[i].x+128 &&
				my >= tubecoords[i].y && my < tubecoords[i].y+16) {
			return i;
		}
	}
	return ILLEGAL_TUBE;
}



sub_torpedo_display::sub_torpedo_display(user_interface& ui_) :
	user_display(ui_), torptranssrc(ILLEGAL_TUBE), desc_texts(get_data_dir()),
	mx(0), my(0), mb(0), torp_desc_line(0)
{
}



void sub_torpedo_display::display(class game& gm) const
{
	submarine* sub = dynamic_cast<submarine*>(gm.get_player());

	double hours = 0.0, minutes = 0.0, seconds = 0.0;

	// draw display without display color.
	glColor4f(1,1,1,1);
	// draw background
	sys().prepare_2d_drawing();

	background->draw(0, 0);

	// draw sub model
	if (subtopsideview.get())//fixme later do not accept empty data
	subtopsideview->draw(0, 0);

	// tube handling. compute coordinates for display and mouse use	
	const vector<submarine::stored_torpedo>& torpedoes = sub->get_torpedoes();
	vector<vector2i> tubecoords = get_tubecoords(sub);
	pair<unsigned, unsigned> bow_tube_indices = sub->get_bow_tube_indices();
	pair<unsigned, unsigned> stern_tube_indices = sub->get_stern_tube_indices();
	pair<unsigned, unsigned> bow_reserve_indices = sub->get_bow_reserve_indices();
	pair<unsigned, unsigned> stern_reserve_indices = sub->get_stern_reserve_indices();
	pair<unsigned, unsigned> bow_deckreserve_indices = sub->get_bow_deckreserve_indices();
	pair<unsigned, unsigned> stern_deckreserve_indices = sub->get_stern_deckreserve_indices();

	// draw tubes
	for (unsigned i = bow_tube_indices.first; i < bow_tube_indices.second; ++i)
		draw_torpedo(gm, true, tubecoords[i], torpedoes[i]);
	for (unsigned i = bow_reserve_indices.first; i < bow_reserve_indices.second; ++i)
		draw_torpedo(gm, true, tubecoords[i], torpedoes[i]);
	for (unsigned i = bow_deckreserve_indices.first; i < bow_deckreserve_indices.second; ++i)
		draw_torpedo(gm, true, tubecoords[i], torpedoes[i]);
	for (unsigned i = stern_tube_indices.first; i < stern_tube_indices.second; ++i)
		draw_torpedo(gm, false, tubecoords[i], torpedoes[i]);
	for (unsigned i = stern_reserve_indices.first; i < stern_reserve_indices.second; ++i)
		draw_torpedo(gm, false, tubecoords[i], torpedoes[i]);
	for (unsigned i = stern_deckreserve_indices.first; i < stern_deckreserve_indices.second; ++i)
		draw_torpedo(gm, false, tubecoords[i], torpedoes[i]);
	
	// draw transfer graphics if needed
	if (torptranssrc != ILLEGAL_TUBE && torpedoes[torptranssrc].status ==
	    submarine::stored_torpedo::st_loaded) {
		glColor4f(1,1,1,0.5);
		torptex(torpedoes[torptranssrc].torp->get_specfilename()).draw(mx-124/2, my-12/2);
		glColor4f(1,1,1,1);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBegin(GL_LINES);
		glVertex2i(tubecoords[torptranssrc].x+124/2,
			tubecoords[torptranssrc].y+12/2);
		glVertex2i(mx, my);
		glEnd();
	}

	// draw information about torpedo in tube if needed
	unsigned tb = get_tube_below_mouse(tubecoords);
	if (tb != ILLEGAL_TUBE) {
		// display type info.
		if (torpedoes[tb].torp != 0 && torpedoes[tb].status == submarine::stored_torpedo::st_loaded) {
			desc_text* torpdesctext = 0;
			string sfn = torpedoes[tb].torp->get_specfilename();
			try {
				torpdesctext = desc_texts.ref(data_file().get_rel_path(sfn) + sfn + "_"
							      + texts::get_language_code() + ".txt");
			}
			catch (error& e) {
				// try again with english text if other text(s) don't exist.
				torpdesctext = desc_texts.ref(data_file().get_rel_path(sfn) + sfn + "_en.txt");
			}
			// fixme: implement scrolling here!
            // torpedo description text, for notepad
			if (torp_desc_line > torpdesctext->nr_of_lines())
				torp_desc_line = torpdesctext->nr_of_lines();
			font_vtremington12->print_wrapped(100, 550, 570, 0, torpdesctext->str(torp_desc_line, 10), color(0,0,0));
		}
		if (torpedoes[tb].status == submarine::stored_torpedo::st_reloading || torpedoes[tb].status == submarine::stored_torpedo::st_unloading) {
			hours = torpedoes[tb].remaining_time/3600;
			minutes = (torpedoes[tb].remaining_time - floor(hours)*3600)/60;
			seconds = torpedoes[tb].remaining_time - floor(hours)*3600 - floor(minutes)*60;
		}
		if (mb & SDL_BUTTON_LMASK) {
			// display remaining time if possible
            // torpedo reload remaning time
			if (torpedoes[tb].status == submarine::stored_torpedo::st_reloading ||
			    torpedoes[tb].status == submarine::stored_torpedo::st_unloading) {
				glColor4f(1,1,1,1);
				notepadsheet->draw(mx, my);
				font_vtremington12->print(mx+32, my+50, texts::get(211) +
						  get_time_string(torpedoes[tb].remaining_time), color(32,0,0));
			}
		}
	}
	glColor4f(1,1,1,1);
	pointer_seconds.draw(floor(seconds)*6);
	pointer_minutes.draw(minutes*6);
	pointer_hours.draw(hours*30);


	// draw deck gun ammo remaining
	if (true == sub->has_deck_gun())
	{
		char a[10];
		sprintf(a, "%ld", sub->num_shells_remaining());
		font_vtremington12->print(400, 85, a, color(0,0,0));
	}

	ui.draw_infopanel();
	sys().unprepare_2d_drawing();
}



void sub_torpedo_display::process_input(class game& gm, const SDL_Event& event)
{
	submarine* sub = dynamic_cast<submarine*>(gm.get_player());
	const vector<submarine::stored_torpedo>& torpedoes = sub->get_torpedoes();

	//fixme:
	// increase/decrease torp_desc_line when clicking on desc text area or using mouse wheel

	switch (event.type) {
	case SDL_MOUSEBUTTONDOWN:
		// check if there is a tube below the mouse and set torptranssrc
		if (event.button.button == SDL_BUTTON_LEFT) {
			torptranssrc = get_tube_below_mouse(get_tubecoords(sub));
			if (torptranssrc != ILLEGAL_TUBE) {
				if (torpedoes[torptranssrc].status !=
				    submarine::stored_torpedo::st_loaded) {
					torptranssrc = ILLEGAL_TUBE;
				}
			}
			mb |= SDL_BUTTON_LMASK;
		} else if (event.button.button == SDL_BUTTON_WHEELUP) {
			if (torp_desc_line > 0) --torp_desc_line;
		} else if (event.button.button == SDL_BUTTON_WHEELDOWN) {
			++torp_desc_line;
		}
		break;
	case SDL_MOUSEBUTTONUP:
		// check if there is a tube below and if its empty
		if (event.button.button == SDL_BUTTON_LEFT) {
			unsigned torptransdst = get_tube_below_mouse(get_tubecoords(sub));
			if (torptransdst != ILLEGAL_TUBE && torptranssrc != ILLEGAL_TUBE) {
				if (torpedoes[torptransdst].status ==
				    submarine::stored_torpedo::st_empty) {
					sub->transfer_torpedo(torptranssrc, torptransdst);
				}
			}
			torptranssrc = ILLEGAL_TUBE;
			mb &= ~SDL_BUTTON_LMASK;
		}
		break;
	case SDL_MOUSEMOTION:
		mx = event.motion.x;
		my = event.motion.y;
		mb = event.motion.state;

		break;
	default:
		break;
	}
}



void sub_torpedo_display::enter(bool is_day)
{
	torpempty.reset(new texture(get_image_dir() + "torpmanage_emptytube.png"));
	torpload.reset(new texture(get_image_dir() + "torpmanage_tubeload.png"));
	torpunload.reset(new texture(get_image_dir() + "torpmanage_tubeunload.png"));
	torp1fat1.reset(new texture(get_image_dir() + "torpmanage_torp1fat1.png"));
	torp1lut1.reset(new texture(get_image_dir() + "torpmanage_torp1lut1.png"));
	torp1lut2.reset(new texture(get_image_dir() + "torpmanage_torp1lut2.png"));
	torp1.reset(new texture(get_image_dir() + "torpmanage_torp1.png"));
	torp1practice.reset(new texture(get_image_dir() + "torpmanage_torp1practice.png"));
	torp2.reset(new texture(get_image_dir() + "torpmanage_torp2.png"));
	torp3afat2.reset(new texture(get_image_dir() + "torpmanage_torp3afat2.png"));
	torp3alut1.reset(new texture(get_image_dir() + "torpmanage_torp3alut1.png"));
	torp3alut2.reset(new texture(get_image_dir() + "torpmanage_torp3alut2.png"));
	torp3fat2.reset(new texture(get_image_dir() + "torpmanage_torp3fat2.png"));
	torp3.reset(new texture(get_image_dir() + "torpmanage_torp3.png"));
	torp4.reset(new texture(get_image_dir() + "torpmanage_torp4.png"));
	torp5b.reset(new texture(get_image_dir() + "torpmanage_torp5b.png"));
	torp5.reset(new texture(get_image_dir() + "torpmanage_torp5.png"));
	torp6lut1.reset(new texture(get_image_dir() + "torpmanage_torp6lut1.png"));
	if (is_day)
		background.reset(new image(get_image_dir() + "tmanage_cleanbase_daylight.jpg"));
	else
		background.reset(new image(get_image_dir() + "tmanage_cleanbase_redlight.jpg"));
	const submarine* pl = dynamic_cast<const submarine*>(ui.get_game().get_player());
	// fixme: catch errors for load, later do not accept missing images
	try {
	std::cout << "loading '" << get_data_dir() + data_file().get_rel_path(pl->get_specfilename()) + pl->get_torpedomanage_img_name() << "'\n";
	subtopsideview.reset(new image(get_data_dir()
				       + data_file().get_rel_path(pl->get_specfilename())
				       + pl->get_torpedomanage_img_name()));
	}
	catch (std::exception& e) {
		std::cout << "ERROR: " << e.what() << "\n";
	}
	catch (...) {}
	pointer_seconds.set("tmanage_seconds_pointer.png", 863, 524, 868, 621);
	pointer_minutes.set("tmanage_minutes_pointer.png", 864, 528, 868, 621);
	pointer_hours.set("tmanage_hours_pointer.png", 863, 548, 868, 621);
}



void sub_torpedo_display::leave()
{
	torpempty.reset();
	torpload.reset();
	torpunload.reset();
	torp1fat1.reset();
	torp1lut1.reset();
	torp1lut2.reset();
	torp1.reset();
	torp1practice.reset();
	torp2.reset();
	torp3afat2.reset();
	torp3alut1.reset();
	torp3alut2.reset();
	torp3fat2.reset();
	torp3.reset();
	torp4.reset();
	torp5b.reset();
	torp5.reset();
	torp6lut1.reset();
	background.reset();
	subtopsideview.reset();
/*
	pointer_seconds.reset();
	pointer_minutes.reset();
	pointer_seconds.reset();
*/
}
