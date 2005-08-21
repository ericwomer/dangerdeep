// user display: submarine's torpedo setup
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "system.h"
#include "image.h"
#include "game.h"
#include "sub_torpsetup_display.h"
#include "submarine_interface.h"
#include "submarine.h"
#include "keys.h"
#include "cfg.h"
#include <sstream>
using namespace std;

/*
static int tubelightdx[6] = { 31,150,271,392,518,641};
static int tubelightdy[6] = {617,610,612,612,617,611};
static int tubelightnx[6] = { 34,154,276,396,521,647};
static int tubelightny[6] = {618,618,618,618,618,618};
static int tubeswitchx = 760, tubeswitchy = 492;
*/

static const double TK_ANGFAC = 360.0/512.0;
static const unsigned TK_PHASES = 6;

sub_torpsetup_display::sub_torpsetup_display(user_interface& ui_)
	: user_display(ui_), turnknobdrag(TK_NONE)
{
	/*
	selected_tube = 0;
	selected_mode = 0;
	*/

	daylight.background.reset(new image(get_image_dir() + "torpsetup_daylight_background.jpg|png"));
	redlight.background.reset(new image(get_image_dir() + "torpsetup_redlight_background.jpg|png"));
	daylight.rundepthptr.set("torpsetup_daylight_rundepthptr.png", 609, 66, 638, 169);
	redlight.rundepthptr.set("torpsetup_redlight_rundepthptr.png", 609, 66, 638, 169);
	daylight.secondaryrangeptr.set("torpsetup_daylight_secondaryrangeptr.png", 228, 157, 257, 262);
	redlight.secondaryrangeptr.set("torpsetup_redlight_secondaryrangeptr.png", 228, 157, 257, 262);
	daylight.primaryrangeptr.set("torpsetup_daylight_primaryrangeptr.png", 241, 90, 260, 263);
	redlight.primaryrangeptr.set("torpsetup_redlight_primaryrangeptr.png", 241, 90, 260, 263); 
	daylight.torpspeeddial.set("torpsetup_daylight_torpspeed.png", 541, 77, 636, 172);
	redlight.torpspeeddial.set("torpsetup_redlight_torpspeed.png", 541, 77, 636, 172); 
	daylight.turnangledial.set("torpsetup_daylight_turnangle.png", 469, 508, 619, 658);
	redlight.turnangledial.set("torpsetup_redlight_turnangle.png", 469, 508, 619, 658);
	daylight.primaryrangedial.set("torpsetup_daylight_primaryrunlength.png", 230, 507, 381, 658);
	redlight.primaryrangedial.set("torpsetup_redlight_primaryrunlength.png", 230, 507, 381, 658);
	daylight.torpspeed[0].reset(new texture(get_image_dir() + "torpsetup_daylight_speedslow.png"));
	redlight.torpspeed[0].reset(new texture(get_image_dir() + "torpsetup_redlight_speedslow.png"));
	daylight.torpspeed[1].reset(new texture(get_image_dir() + "torpsetup_daylight_speedmedium.png"));
	redlight.torpspeed[1].reset(new texture(get_image_dir() + "torpsetup_redlight_speedmedium.png"));
	daylight.torpspeed[2].reset(new texture(get_image_dir() + "torpsetup_daylight_speedhigh.png"));
	redlight.torpspeed[2].reset(new texture(get_image_dir() + "torpsetup_redlight_speedhigh.png"));
	daylight.firstturn[0].reset(new texture(get_image_dir() + "torpsetup_daylight_turnleft.png"));
	redlight.firstturn[0].reset(new texture(get_image_dir() + "torpsetup_redlight_turnleft.png"));
	daylight.firstturn[1].reset(new texture(get_image_dir() + "torpsetup_daylight_turnright.png"));
	redlight.firstturn[1].reset(new texture(get_image_dir() + "torpsetup_redlight_turnright.png"));
	daylight.secondaryrange[0].reset(new texture(get_image_dir() + "torpsetup_daylight_secondaryrange_short.png"));
	redlight.secondaryrange[0].reset(new texture(get_image_dir() + "torpsetup_redlight_secondaryrange_short.png"));
	daylight.secondaryrange[1].reset(new texture(get_image_dir() + "torpsetup_daylight_secondaryrange_long.png"));
	redlight.secondaryrange[1].reset(new texture(get_image_dir() + "torpsetup_redlight_secondaryrange_long.png"));
	daylight.preheating[0].reset(new texture(get_image_dir() + "torpsetup_daylight_preheatoff.png"));
	redlight.preheating[0].reset(new texture(get_image_dir() + "torpsetup_daylight_preheatoff.png"));
	daylight.preheating[1].reset(new texture(get_image_dir() + "torpsetup_daylight_preheaton.png"));
	redlight.preheating[1].reset(new texture(get_image_dir() + "torpsetup_daylight_preheaton.png"));
	daylight.temperaturescale.reset(new texture(get_image_dir() + "torpsetup_daylight_tempscale.png"));
	redlight.temperaturescale.reset(new texture(get_image_dir() + "torpsetup_daylight_tempscale.png"));

	// read knobs images and cut to separate images
	image primaryrangeknobs_day(get_image_dir() + "torpsetup_daylight_primaryrangeknobs.png");
	image primaryrangeknobs_red(get_image_dir() + "torpsetup_redlight_primaryrangeknobs.png");
	image turnangleknobs_day(get_image_dir() + "torpsetup_daylight_turnangleknobs.png");
	image turnangleknobs_red(get_image_dir() + "torpsetup_redlight_turnangleknobs.png");
	image rundepthknobs_day(get_image_dir() + "torpsetup_daylight_rundepthknobs.png");
	image rundepthknobs_red(get_image_dir() + "torpsetup_redlight_rundepthknobs.png");
	for (unsigned i = 0; i < TK_PHASES; ++i) {
		daylight.primaryrangeknob[i].set(new texture(primaryrangeknobs_day.get_SDL_Surface(), 0, i*192, 192, 192), 277, 571, 373, 667);
		redlight.primaryrangeknob[i].set(new texture(primaryrangeknobs_red.get_SDL_Surface(), 0, i*192, 192, 192), 277, 571, 373, 667);
		daylight.turnangleknob[i].set(new texture(turnangleknobs_day.get_SDL_Surface(), 0, i*192, 192, 192), 528, 571, 624, 667);
		redlight.turnangleknob[i].set(new texture(turnangleknobs_red.get_SDL_Surface(), 0, i*192, 192, 192), 528, 571, 624, 667);
		daylight.rundepthknob[i].set(new texture(rundepthknobs_day.get_SDL_Surface(), 0, i*192, 192, 192), 819, 17, 915, 113);
		redlight.rundepthknob[i].set(new texture(rundepthknobs_red.get_SDL_Surface(), 0, i*192, 192, 192), 819, 17, 915, 113);
	}
}



void sub_torpsetup_display::process_input(class game& gm, const SDL_Event& event)
{
	submarine* sub = dynamic_cast<submarine*>(gm.get_player());
	bool is_day = gm.is_day_mode();
	const scheme& s = (is_day) ? daylight : redlight;
	int mx, my, mb;
	switch (event.type) {
	case SDL_MOUSEBUTTONDOWN:
		mx = event.button.x;
		my = event.button.y;
		// check if mouse is over turn knobs
		if (s.primaryrangeknob[0].is_mouse_over(mx, my)) {
			turnknobdrag = TK_PRIMARYRANGE;
		} else if (s.turnangleknob[0].is_mouse_over(mx, my)) {
			turnknobdrag = TK_TURNANGLE;
		} else if (s.rundepthknob[0].is_mouse_over(mx, my)) {
			turnknobdrag = TK_RUNDEPTH;
		} else {
			turnknobdrag = TK_NONE;
		}
		break;
	case SDL_MOUSEMOTION:
		mx = event.motion.xrel;
		my = event.motion.yrel;
		mb = event.motion.state;
		if (event.motion.state & SDL_BUTTON_LMASK) {
			if (turnknobdrag != TK_NONE) {
				turnknobang[unsigned(turnknobdrag)%TK_NR]
					+= angle(mx * TK_ANGFAC);
			}
		}
		break;
	case SDL_MOUSEBUTTONUP:
		mx = event.button.x;
		my = event.button.y;
		turnknobdrag = TK_NONE;
	}


#if 0
	int* tubelightx = (is_day) ? tubelightdx : tubelightnx;
	int* tubelighty = (is_day) ? tubelightdy : tubelightny;
	const scheme& s = (is_day) ? daylight : redlight;
	int mx, my;
	switch (event.type) {

	case SDL_MOUSEMOTION:
		mx = event.motion.x;
		my = event.motion.y;
		mb = event.motion.state;

		// if mouse is over turnswitch and button is down, set switch
		if (event.motion.state & SDL_BUTTON_LMASK) {

	case SDL_MOUSEBUTTONDOWN:
		mx = event.button.x;
		my = event.button.y;
		// check if mouse is over tube indicators
		for (unsigned i = 0; i < 6; ++i) {
			if (mx >= tubelightx[i] && my >= tubelighty[i] &&
			    mx < tubelightx[i] + int(s.tubelight[i]->get_width()) &&
			    my < tubelighty[i] + int(s.tubelight[i]->get_height())) {
				dynamic_cast<submarine_interface&>(ui).fire_tube(sub, i);
			}
		}
		if (mx >= tubeswitchx && my >= tubeswitchy &&
		    mx < tubeswitchx + int(s.tubeswitch[0]->get_width()) &&
		    my < tubeswitchy + int(s.tubeswitch[0]->get_height())) {
			// fixme: better make angle switch?
			unsigned tn = (6 * (mx - tubeswitchx)) / s.tubeswitch[0]->get_width();
			if (tn < sub->get_nr_of_bow_tubes() + sub->get_nr_of_stern_tubes())
				selected_tube = tn;
		}

/*
		//if mouse is over control c, compute angle a, set matching command, fixme
		if (indicators[compass].is_over(mx, my)) {
			angle mang = angle(180)-indicators[compass].get_angle(mx, my);
			sub->head_to_ang(mang, mang.is_cw_nearer(sub->get_heading()));
		} else if (indicators[depth].is_over(mx, my)) {
			angle mang = angle(-39) - indicators[depth].get_angle(mx, my);
			if (mang.value() < 270) {
				sub->dive_to_depth(unsigned(mang.value()));
			}
		} else if (indicators[mt].is_over(mx, my)) {
			unsigned opt = (indicators[mt].get_angle(mx, my) - angle(210)).value() / 20;
			if (opt >= 15) opt = 14;
			switch (opt) {
			case 0: sub->set_throttle(ship::aheadflank); break;
			case 1: sub->set_throttle(ship::aheadfull); break;
			case 2: sub->set_throttle(ship::aheadhalf); break;
			case 3: sub->set_throttle(ship::aheadslow); break;
			case 4: sub->set_throttle(ship::aheadlisten); break;
			case 7: sub->set_throttle(ship::stop); break;
			case 11: sub->set_throttle(ship::reverse); break;//fixme: various reverse speeds!
			case 12: sub->set_throttle(ship::reverse); break;
			case 13: sub->set_throttle(ship::reverse); break;
			case 14: sub->set_throttle(ship::reverse); break;
			case 5: // diesel engines
			case 6: // attention
			case 8: // electric engines
			case 9: // surface
			case 10:// dive
				break;
			}
		}
*/
		break;
	default:
		break;
	}

/*
	switch (event.type) {
	case SDL_KEYDOWN:
		//fixme
	default: break;
	}
*/
#endif
}



// transform real angles to knob image index numbers and rotational angles.
// each image is 7.5deg rotated to its neighbour
static inline pair<unsigned, double> angle2idxang(double ang)
{
	double a = myfmod(ang, 360);
	unsigned phase = unsigned(floor(a/7.5+0.5));
	return make_pair(phase % 6, a - phase * 7.5);
}

void sub_torpsetup_display::display(class game& gm) const
{
	submarine* player = dynamic_cast<submarine*>(gm.get_player());

	// test: clear background, later this is not necessary, fixme
	glClearColor(1, 0, 1, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	sys().prepare_2d_drawing();
	glColor3f(1,1,1);

	// determine time of day
	bool is_day = gm.is_day_mode();

#if 0
	int* tubelightx = (is_day) ? tubelightdx : tubelightnx;
	int* tubelighty = (is_day) ? tubelightdy : tubelightny;
#endif

	const scheme& s = (is_day) ? daylight : redlight;

	// testing:
	double ctr = gm.get_time();

	// as first draw lowest layer: sliding temperature scale
	double temperature = myfmod(ctr, 35);//22.5;	// degrees, fixme
	int tempscalex = 549 - int(36 + 435*temperature/35.0f);
	s.temperaturescale->draw(tempscalex, 364);

	// as next draw lower layer: dials
	double torpspeed = myfmod(ctr, 55);//44.0; // knots
	s.torpspeeddial.draw(-(torpspeed * 330/55.0)); // 55kts = 0deg+x*330deg

	unsigned primaryrangedial = myfmod(ctr,32)*100;//2500;	// meters
	s.primaryrangedial.draw(primaryrangedial / -10.0f);	// 1 degree = 10meters

	float firstturnangle = myfmod(ctr,30)*6;//180;
	s.turnangledial.draw(firstturnangle * -1.8f); // 18 degrees = 10 turn degrees

	// draw background
	s.background->draw(0, 0);

	// draw objects from upper layer: knobs/switches/pointers
	unsigned torpspeedidx = 2; // 0-2 slow-fast
	s.torpspeed[torpspeedidx]->draw(834, 251);

	unsigned firstturnidx = 1; // 0-1 left-right
	s.firstturn[firstturnidx]->draw(64, 574);

	unsigned secondaryrangeidx = 1; // 0-1 800 or 1600m
	s.secondaryrange[secondaryrangeidx]->draw(803, 552);

	unsigned preheatingidx = 0; // 0-1 off-on
	s.preheating[preheatingidx]->draw(730, 377);

	s.primaryrangeknob[unsigned(turnknobang[TK_PRIMARYRANGE].value()/(45.0/TK_PHASES)+0.5)%TK_PHASES].draw(0);

	s.turnangleknob[unsigned(turnknobang[TK_TURNANGLE].value()/(45.0/TK_PHASES)+0.5)%TK_PHASES].draw(0);

	s.rundepthknob[unsigned(turnknobang[TK_RUNDEPTH].value()/(45.0/TK_PHASES)+0.5)%TK_PHASES].draw(0);

	double rundepth = myfmod(ctr,25);//10.0;	// meters
	s.rundepthptr.draw(rundepth * 300/25.0 + 30); // 25m = 30deg+x*300deg

	double secondaryrange = myfmod(ctr,32)*50;//800.0; // meters
	s.secondaryrangeptr.draw(secondaryrange * 320/1600.0 + 20.0); // 1600m = 20deg+x*320deg

	double primaryrange = myfmod(ctr,32)*500;//2300.0; // meters
	s.primaryrangeptr.draw(primaryrange * 320/16000.0 + 20.0); // 16000m = 20deg+x*320deg

	sys().unprepare_2d_drawing();
}
