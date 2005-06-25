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
			torptex(st.type).draw(pos.x, pos.y);
			torpload->draw(pos.x, pos.y);
		} else if (st.status == 2) {	// unloading
			torpempty->draw(pos.x, pos.y);
			torpunload->draw(pos.x, pos.y);
		} else {		// loaded
			torptex(st.type).draw(pos.x, pos.y);
		}
	} else {
		if (st.status == 0) {	// empty
			torpempty->draw_hm(pos.x, pos.y);
		} else if (st.status == 1) {	// reloading
			torptex(st.type).draw_hm(pos.x, pos.y);
			torpload->draw_hm(pos.x, pos.y);
		} else if (st.status == 2) {	// unloading
			torpempty->draw_hm(pos.x, pos.y);
			torpunload->draw_hm(pos.x, pos.y);
		} else {		// loaded
			torptex(st.type).draw_hm(pos.x, pos.y);
		}
	}
}



const texture& sub_torpedo_display::torptex(unsigned type) const
{
	switch (type) {
		case torpedo::T1: return *torp1;
		case torpedo::T2: return *torp2;
		case torpedo::T3: return *torp3;
		case torpedo::T3a: return *torp3alut1;//fixme 3a?
		case torpedo::T4: return *torp4;
		case torpedo::T5: return *torp5;
		case torpedo::T11: return *torp5b;
		case torpedo::T1FAT: return *torp1fat1;
		case torpedo::T3FAT: return *torp3fat2;
		case torpedo::T6LUT: return *torp6lut1;
	}
	return *torpempty;
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



// obsolete:
void sub_torpedo_display::check_turnswitch_input(game& gm, int x, int y)
{
	submarine* sub = dynamic_cast<submarine*>(gm.get_player());
	if (y >= 384 && y < 640) {
		if (x < 256)
			sub->set_trp_primaryrange(turnswitch_input(x, y-384, 17));
		else if (x < 512)
			sub->set_trp_secondaryrange(turnswitch_input(x-256, y-384, 2));
		else if (x < 768)
			sub->set_trp_initialturn(turnswitch_input(x-512, y-384, 2));
		else
			sub->set_trp_searchpattern(turnswitch_input(x-768, y-384, 2));
	}
}



// obsolete:
//maybe make common for all displays, so make function of user_display? but currently
//only needed for this display.
void sub_torpedo_display::draw_turnswitch(class game& gm, int x, int y,
	unsigned firstdescr, unsigned nrdescr, unsigned selected, unsigned extradescr, unsigned title) const
{
	double full_turn = (nrdescr <= 2) ? 90 : 270;
	double begin_turn = (nrdescr <= 2) ? -45 : -135;
	turnswitchbackgr->draw(x, y);
	double degreesperpos = (nrdescr > 1) ? full_turn/(nrdescr-1) : 0;
	glColor4f(1,1,1,1);
	for (unsigned i = 0; i < nrdescr; ++i) {
		vector2 d = angle(begin_turn+degreesperpos*i).direction();
		glBindTexture(GL_TEXTURE_2D, 0);
		glBegin(GL_LINES);
		glVertex2f(x+128+d.x*36,y+128-d.y*36);
		glVertex2f(x+128+d.x*80,y+128-d.y*80);
		glEnd();
		font_arial->print_c(x+int(d.x*96)+128, y-int(d.y*96)+128, texts::get(firstdescr+i));
	}
	font_arial->print_c(x+128, y+196, texts::get(extradescr));
	turnswitch->draw_rot(x+128, y+128, begin_turn+degreesperpos*selected);
	font_arial->print_c(x+128, y+228, texts::get(title));
}



// obsolete:
unsigned sub_torpedo_display::turnswitch_input(int x, int y, unsigned nrdescr) const
{
	if (nrdescr <= 1) return 0;
	angle a(vector2(x-128, 128-y));
	double full_turn = (nrdescr <= 2) ? 90 : 270;
	double begin_turn = (nrdescr <= 2) ? -45 : -135;
	double degreesperpos = full_turn/(nrdescr-1);
	double ang = a.value_pm180() - begin_turn;
	if (ang < 0) ang = 0;
	if (ang > full_turn) ang = full_turn;
	return unsigned(round(ang/degreesperpos));
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

	// draw torpedo programming buttons, obsolete:
	draw_turnswitch(gm,   0, 384, 142, 17, sub->get_trp_primaryrange(), 175, 138);
	draw_turnswitch(gm, 256, 384, 159, 2, sub->get_trp_secondaryrange(), 0, 139);
	draw_turnswitch(gm, 512, 384, 161, 2, sub->get_trp_initialturn(), 0, 140);
	draw_turnswitch(gm, 768, 384, 163, 2, sub->get_trp_searchpattern(), 176, 141);

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
		torptex(torpedoes[torptranssrc].type).draw(mx-124/2, my-12/2);
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
			if (torpedoes[tb].type != torpedo::none
			    && torpedoes[tb].status == submarine::stored_torpedo::st_loaded) {
				color::white().set_gl_color();
				notepadsheet->draw(mx, my);
				torptex(torpedoes[tb].type).draw(mx+64, my+36);
				font_olympiaworn->print(mx+16, my+60,
						  texts::get(300+torpedoes[tb].type-1),
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
			// if mouse is over turnswitch, set it
			check_turnswitch_input(gm, event.button.x, event.button.y);
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

		// if mouse is over turnswitch and button is down, set switch
		if (event.motion.state & SDL_BUTTON_LMASK) {
			check_turnswitch_input(gm, mx, my);
		}
		break;
	default:
		break;
	}
}
