// user display: submarine's torpedo room
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "system.h"
#include "image.h"
#include "texture.h"
#include "game.h"
#include "command.h"
#include "sub_torpedo_display.h"
#include "texts.h"


void sub_torpedo_display::draw_torpedo(class game& gm, bool usebow,
	const vector2i& pos, const submarine::stored_torpedo& st) const
{
	if (usebow) {
		if (st.status == 0) {	// empty
			torpempty->draw(pos.x, pos.y);
		} else if (st.status == 1) {	// reloading
			torptex(st.type)->draw(pos.x, pos.y);
			torpreload->draw(pos.x, pos.y);
		} else if (st.status == 2) {	// unloading
			torpempty->draw(pos.x, pos.y);
			torpunload->draw(pos.x, pos.y);
		} else {		// loaded
			torptex(st.type)->draw(pos.x, pos.y);
		}
	} else {
		if (st.status == 0) {	// empty
			torpempty->draw_hm(pos.x, pos.y);
		} else if (st.status == 1) {	// reloading
			torptex(st.type)->draw_hm(pos.x, pos.y);
			torpreload->draw_hm(pos.x, pos.y);
		} else if (st.status == 2) {	// unloading
			torpempty->draw_hm(pos.x, pos.y);
			torpunload->draw_hm(pos.x, pos.y);
		} else {		// loaded
			torptex(st.type)->draw_hm(pos.x, pos.y);
		}
	}
}



texture* sub_torpedo_display::torptex(unsigned type)
{
	switch (type) {
		case torpedo::T1: return torpt1;
		case torpedo::T2: return torpt2;
		case torpedo::T3: return torpt3;
		case torpedo::T3a: return torpt3a;
		case torpedo::T4: return torpt4;
		case torpedo::T5: return torpt5;
		case torpedo::T11: return torpt11;
		case torpedo::T1FAT: return torpt1fat;
		case torpedo::T3FAT: return torpt3fat;
		case torpedo::T6LUT: return torpt6lut;
	}
	return torpempty;
}



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



sub_torpedo_display::sub_torpedo_display() :
	torptranssrc(0xffff)
{
	// reference images here and fill pointers
}



sub_torpedo_display::~sub_torpedo_display()
{
}



void sub_torpedo_display::display(class game& gm) const
{
	submarine* player = dynamic_cast<submarine*>(gm.get_player());

	// draw display without display color.
	glColor4f(1,1,1,1);
	// draw background
	system::sys().prepare_2d_drawing();
	background->draw_tiles(0, 0, 1024, 768);
	glClear(GL_DEPTH_BUFFER_BIT);

	// draw sub model	
	glPushMatrix();
	glTranslatef(512, 160, 1);
	glScalef(1024/80.0, 1024/80.0, 0.001);
	glRotatef(90, 0, 0, 1);
	glRotatef(-90, 0, 1, 0);
	player->display();
	glPopMatrix();
	
	// draw torpedo programming buttons
	draw_turnswitch(gm,   0, 384, 142, 17, player->get_trp_primaryrange(), 175, 138);
	draw_turnswitch(gm, 256, 384, 159, 2, player->get_trp_secondaryrange(), 0, 139);
	draw_turnswitch(gm, 512, 384, 161, 2, player->get_trp_initialturn(), 0, 140);
	draw_turnswitch(gm, 768, 384, 163, 2, player->get_trp_searchpattern(), 176, 141);

	// tube handling. compute coordinates for display and mouse use	
	const vector<submarine::stored_torpedo>& torpedoes = player->get_torpedoes();
	vector<vector2i> tubecoords(torpedoes.size());
	pair<unsigned, unsigned> bow_tube_indices = player->get_bow_tube_indices();
	pair<unsigned, unsigned> stern_tube_indices = player->get_stern_tube_indices();
	pair<unsigned, unsigned> bow_reserve_indices = player->get_bow_reserve_indices();
	pair<unsigned, unsigned> stern_reserve_indices = player->get_stern_reserve_indices();
	pair<unsigned, unsigned> bow_deckreserve_indices = player->get_bow_deckreserve_indices();
	pair<unsigned, unsigned> stern_deckreserve_indices = player->get_stern_deckreserve_indices();
	unsigned k = bow_tube_indices.second - bow_tube_indices.first;
	for (unsigned i = bow_tube_indices.first; i < bow_tube_indices.second; ++i) {
		tubecoords[i] = vector2i(0, 192+(i-k/2)*16);
	}
	for (unsigned i = bow_reserve_indices.first; i < bow_reserve_indices.second; ++i) {
		unsigned j = i - bow_reserve_indices.first;
		tubecoords[i] = vector2i(192+(j/k)*128, 192+(j%k-k/2)*16);
	}
	for (unsigned i = bow_deckreserve_indices.first; i < bow_deckreserve_indices.second; ++i) {
		unsigned j = i - bow_deckreserve_indices.first;
		tubecoords[i] = vector2i(192+(j/2)*128, 96+(j%2)*16);
	}
	for (unsigned i = stern_tube_indices.first; i < stern_tube_indices.second; ++i) {
		unsigned j = i - stern_tube_indices.first;
		tubecoords[i] = vector2i(896, 160+j*16);
	}
	for (unsigned i = stern_reserve_indices.first; i < stern_reserve_indices.second; ++i) {
		unsigned j = i - stern_reserve_indices.first;
		tubecoords[i] = vector2i(704, 160+j*16);
	}
	for (unsigned i = stern_deckreserve_indices.first; i < stern_deckreserve_indices.second; ++i) {
		unsigned j = i - stern_deckreserve_indices.first;
		tubecoords[i] = vector2i(704-(j/2)*128, 96+(j%2)*16);
	}

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
	


//INPUT:



	// mouse handling
	int mx, my, mb = system::sys().get_mouse_buttons();
	system::sys().get_mouse_position(mx, my);

	unsigned mouseovertube = 0xffff;
	for (unsigned i = 0; i < torpedoes.size(); ++i) {
		if (mx >= tubecoords[i].x && mx < tubecoords[i].x+128 &&
				my >= tubecoords[i].y && my < tubecoords[i].y+16) {
			mouseovertube = i;
			break;
		}
	}

	if (mb & system::sys().left_button) {
		// button down
		if (	mouseovertube < torpedoes.size()
			&& torptranssrc >= torpedoes.size()
			&& torpedoes[mouseovertube].status == submarine::stored_torpedo::st_loaded)
		{
			torptranssrc = mouseovertube;
		} else {
			if (torpedoes[mouseovertube].status == submarine::stored_torpedo::st_reloading ||
					torpedoes[mouseovertube].status == submarine::stored_torpedo::st_unloading) {
				glColor4f(1,1,1,1);
				notepadsheet->draw(mx, my);
				unsigned seconds = unsigned(torpedoes[mouseovertube].remaining_time + 0.5);	
				unsigned hours = seconds / 3600;
				unsigned minutes = (seconds % 3600) / 60;
				seconds = seconds % 60;
				ostringstream oss;
				oss << texts::get(211) << hours << ":" << minutes << ":" << seconds;
				font_arial->print(mx+32, my+32, oss.str(), color(0,0,128));
			}
		}
		
		// torpedo programming buttons
		if (my >= 384 && my < 640) {
			if (mx < 256) gm.send(new command_set_trp_primaryrange(player, turnswitch_input(mx, my-384, 17)));
			else if (mx < 512) gm.send(new command_set_trp_secondaryrange(player, turnswitch_input(mx-256, my-384, 2)));
			else if (mx < 768) gm.send(new command_set_trp_initialturn(player, turnswitch_input(mx-512, my-384, 2)));
			else gm.send(new command_set_trp_searchpattern(player, turnswitch_input(mx-768, my-384, 2)));
		}
	
	} else {
		
		// button released
		if (	   mouseovertube < torpedoes.size()
			&& torptranssrc < torpedoes.size()
			&& torpedoes[mouseovertube].status == submarine::stored_torpedo::st_empty
			&& mouseovertube != torptranssrc)
		{
			// transport
			gm.send(new command_transfer_torpedo(player, torptranssrc, mouseovertube));
		}
		torptranssrc = 0xffff;

		// display type info
		if (	mouseovertube < torpedoes.size()
				&& torpedoes[mouseovertube].type != torpedo::none
				&& torpedoes[mouseovertube].status == submarine::stored_torpedo::st_loaded) {
			color::white().set_gl_color();
			notepadsheet->draw(mx, my);
			torptex(torpedoes[mouseovertube].type)->draw(mx+64, my+36);
			font_arial->print(mx+16, my+60, texts::get(300+torpedoes[mouseovertube].type-1), color(0,0,128));
		}
	}
		
	// draw line for torpedo transfer if button down
	if (mb != 0 && torptranssrc < torpedoes.size()) {
		glColor4f(1,1,1,0.5);
		torptex(torpedoes[torptranssrc].type)->draw(mx-64, my-8);
		glColor4f(1,1,1,1);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBegin(GL_LINES);
		glVertex2i(tubecoords[torptranssrc].x+64,
			tubecoords[torptranssrc].y+8);
		glVertex2i(mx, my);
		glEnd();
	}

	draw_infopanel(gm);
	system::sys().unprepare_2d_drawing();

	// keyboard processing
	int key = system::sys().get_key().sym;
	while (key != 0) {
		if (!keyboard_common(key,  gm)) {
			// specific keyboard processing
		}
		key = system::sys().get_key().sym;
	}










FROM GAUGES DISPLAY:
	submarine* player = dynamic_cast<submarine*> ( gm.get_player () );
	system::sys().prepare_2d_drawing();

	if (true /*gm.is_day() fixme*/) {
		controlscreen_normallight.draw(0, 0);
	} else {
		controlscreen_nightlight.draw(0, 0);
	}

	// the absolute numbers here depend on the graphics!
	indicators[compass].display(-player->get_heading().value());
	indicators[battery].display(0);
	indicators[compressor].display(0);
	indicators[diesel].display(0);
	indicators[bow_depth_rudder].display(0);
	indicators[stern_depth_rudder].display(0);
	indicators[depth].display(player->get_depth()*1.0-51.0);
	indicators[knots].display(fabs(player->get_speed())*22.33512-133.6);
	indicators[main_rudder].display(player->get_rudder_pos()*3.5125);
	indicators[mt].display(0);

/*	// kept as text reference for tooltips/popups.
	angle player_speed = player->get_speed()*360.0/sea_object::kts2ms(36);
	angle player_depth = -player->get_pos().z;
	draw_gauge(gm, 1, 0, 0, 256, player->get_heading(), texts::get(1),
		player->get_head_to());
	draw_gauge(gm, 2, 256, 0, 256, player_speed, texts::get(4));
	draw_gauge(gm, 4, 2*256, 0, 256, player_depth, texts::get(5));
	draw_clock(gm, 3*256, 0, 256, gm.get_time(), texts::get(61));
	draw_manometer_gauge ( gm, 1, 0, 256, 256, player->get_fuel_level (),
		texts::get(101));
	draw_manometer_gauge ( gm, 1, 256, 256, 256, player->get_battery_level (),
		texts::get(102));
*/
//	draw_infopanel(gm);

	system::sys().unprepare_2d_drawing();
}

void sub_torpedo_display::process_input(class game& gm, const SDL_Event& event)
{
	submarine* sub = dynamic_cast<submarine*>(gm.get_player());
	int mx, my;
	switch (event.type) {
	case SDL_MOUSEBUTTONDOWN:
		mx = event.button.x;
		my = event.button.y;
		//if mouse is over control c, compute angle a, set matching command, fixme
		if (indicators[compass].is_over(mx, my)) {
			angle mang = -indicators[compass].get_angle(mx, my);
			gm.send(new command_head_to_ang(sub, mang, mang.is_cw_nearer(sub->get_heading())));
		}
		break;
	default:
		break;
	}
}
