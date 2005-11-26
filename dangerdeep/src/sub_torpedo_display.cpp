// user display: submarine's torpedo room
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "system.h"
#include "image.h"
#include "texture.h"
#include "game.h"
#include "font.h"
#include "sub_torpedo_display.h"
#include "texts.h"
#include "user_interface.h"
#include <sstream>
#include <iomanip>
using namespace std;

#define ILLEGAL_TUBE 0xffffffff



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
	user_display(ui_), torptranssrc(ILLEGAL_TUBE), mx(0), my(0), mb(0)
{
	// maybe ref (cache!) torpedo images here instead of loading them?
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
	submodelVIIc.reset(new texture(get_image_dir() + "torpmanage_submodelVIIc.png"));
	background_daylight.reset(new image(get_image_dir() + "torpmanage_daylight_background.jpg"));
	background_redlight.reset(new image(get_image_dir() + "torpmanage_redlight_background.jpg"));
}



void sub_torpedo_display::display(class game& gm) const
{
	submarine* sub = dynamic_cast<submarine*>(gm.get_player());

	// draw display without display color.
	glColor4f(1,1,1,1);
	// draw background
	sys().prepare_2d_drawing();

	bool is_day = gm.is_day_mode();
	if (is_day) {
		background_daylight->draw(0, 0);
	} else {
		background_redlight->draw(0, 0);
	}

	// draw sub model
	submodelVIIc->draw(69, 37);

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
		if (mb & SDL_BUTTON_LMASK) {
			// display remaining time if possible
			if (torpedoes[tb].status == submarine::stored_torpedo::st_reloading ||
			    torpedoes[tb].status == submarine::stored_torpedo::st_unloading) {
				glColor4f(1,1,1,1);
				notepadsheet->draw(mx, my);
				font_olympiaworn->print(mx+32, my+32, texts::get(211) +
						  get_time_string(torpedoes[tb].remaining_time), color(0,0,128));
			}
		} else {
			// display type info
			if (torpedoes[tb].torp != 0
			    && torpedoes[tb].status == submarine::stored_torpedo::st_loaded) {
				color::white().set_gl_color();
				notepadsheet->draw(mx, my);
				torptex(torpedoes[tb].torp->get_specfilename()).draw(mx+64, my+36);
				font_olympiaworn->print(mx+16, my+60,
							torpedoes[tb].torp->get_specfilename(),//texts::get(300+torpedoes[tb].torp->get_specfilename()-1),//fixme: show more info!
							color(0,0,128));
			}
		}
	}

	// draw deck gun ammo remaining
	if (true == sub->has_deck_gun())
	{
		char a[10];
		sprintf(a, "%ld", sub->num_shells_remaining());
		font_olympiaworn->print(400, 85, a, color(255,255,255));
	}
	
	ui.draw_infopanel();
	sys().unprepare_2d_drawing();
}



void sub_torpedo_display::process_input(class game& gm, const SDL_Event& event)
{
	submarine* sub = dynamic_cast<submarine*>(gm.get_player());
	const vector<submarine::stored_torpedo>& torpedoes = sub->get_torpedoes();
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
		}
		break;
	case SDL_MOUSEBUTTONUP:
		// check if there is a tube below and if its empty
		if (event.button.button == SDL_BUTTON_LEFT) {
			unsigned torptransdst = get_tube_below_mouse(get_tubecoords(sub));
			if (torptransdst != ILLEGAL_TUBE) {
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
