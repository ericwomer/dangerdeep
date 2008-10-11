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

// user display: submarine's kdb hearing device
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "sub_kdb_display.h"
#include "system.h"
#include "image.h"
#include "texture.h"
#include "game.h"
#include "submarine_interface.h"
#include "submarine.h"
#include "keys.h"
#include "cfg.h"
#include "global_data.h"
#include <sstream>
using namespace std;

static const double TK_ANGFAC = 360.0/512.0;
static const unsigned TK_PHASES = 6;

sub_kdb_display::scheme::scheme(bool day)
{
	const string x = day ? "KDB_daylight" : "KDB_redlight";
	background.reset(new image(get_image_dir() + x + "_background.jpg"));
	direction_ptr.set(x + "_pointer.png", 323, 122, 377, 373);

	for (unsigned i = 0; i < TK_PHASES; ++i) {
		ostringstream osn;
		osn << (i+1) ;
		turn_wheel[i].set(x + "_gauge" + osn.str() + ".png", 166, 682);
		volume_knob[i].set(x + "_knob" + osn.str() + ".png", 683, 667);
	}
}



sub_kdb_display::sub_kdb_display(user_interface& ui_)
	: user_display(ui_), turnknobdrag(TK_NONE), turnknobang(TK_NR)
{
}



void sub_kdb_display::process_input(class game& gm, const SDL_Event& event)
{
	int mx, my, mb;

	if (!myscheme.get()) throw error("sub_kdb_display::process_input without scheme!");
	const scheme& s = *myscheme;

	switch (event.type) {
	case SDL_MOUSEBUTTONDOWN:
		mx = event.button.x;
		my = event.button.y;
		// check if mouse is over turn knobs
		turnknobdrag = TK_NONE;
		if (s.volume_knob[0].is_mouse_over(mx, my)) {
			turnknobdrag = TK_VOLUME;
		} else if (s.turn_wheel[0].is_mouse_over(mx, my, 128)) {
			turnknobdrag = TK_DIRECTION;
		}
		break;
	case SDL_MOUSEMOTION:
		mx = event.motion.xrel;
		my = event.motion.yrel;
		mb = event.motion.state;
		if (event.motion.state & SDL_BUTTON_LMASK) {
			if (turnknobdrag != TK_NONE) {
				float& ang = turnknobang[unsigned(turnknobdrag)];
				ang += mx * TK_ANGFAC;
				switch (turnknobdrag) {
				case TK_DIRECTION:
					// bring to 0...360 degree value
					ang = myfmod(ang, 720.0f);
					//sub->set_kdb_direction(ang); // fixme: set angle of player
					break;
				case TK_VOLUME:
					// 0-360 degrees possible
					ang = myclamp(ang, 0.0f, 360.0f);
					break;
				default:	// can never happen
					break;
				}
			}
		}
		break;
	case SDL_MOUSEBUTTONUP:
		mx = event.button.x;
		my = event.button.y;
		turnknobdrag = TK_NONE;
		break;
	default:
		break;
	}
}



// part of sonar operator simulation
// fixme: if noise source has same signal for angles a1...a2, return roughly (a1+a2)/2 as result
// fixme2: if there is no signal, report that somehow, do not report false, random peak
// fixme3: we can't find ONE global peek, but one for each side now...
/* 
   the sonarman should report all signals he hears around the compass.
   But the simulation or action takes time, so he can't report all signals at once.
   He takes his time to search the whole compass around, say, 1 minute.
   While doing that he listens for signals, and if he detects one, he tries to localize it
   as good as he can, then reports the signal (angle, strength (?) = distance, type)
   "Escort detected in 30° (off bow), weak signal" -> far away destroyer etc
   Problems: when a signal is very close (a hunting escort), you don't want the sonarman
   to listen/detect signals all around the clock, but to track that signal. When should
   he start tracking one specific signal? when to stop that? should the captain ask
   for tracking of a signal? But while tracking one signal, the sonar man should not
   forget or ignore the other side of the sub, maybe there is a destroyer closing, too.

   Summary:
   The sonar man scans around the compass for signals. If he detects a signal, he tries to
   locate it as exactly as he can. He then notes down the signal to the notebook. Older
   signal reports there that are nearby the current angle are discarded.
   When the sonarman detects a loud (and thus close) signal of an escort, he starts tracking
   that signal. Other notebook entries are then kept. When the tracked signal gets less loud
   and thus drives away, he starts sweeping again.

   The sonarman localizes a signal by detecting at which angle a signal reaches a maximum strength
   and at which angle this strength goes down. The center of both angles must be the direction
   of the noise source.

   So the simulation has a state (track / scan), an angle that is currently used for tracking/scanning
   and a list of reported signals. Last time of simulation is also stored.
   The simulation is run for delta_t seconds. If last_time + delta_t > simulation_step then run next
   step.
*/

pair<angle, double> find_peak_noise(angle startangle, double step, double maxstep, game& gm)
{
	submarine* player = dynamic_cast<submarine*>(gm.get_player());
	angle ang_peak = startangle;
	double peak_val = gm.sonar_listen_ships(player, startangle).first;
	startangle += step;
	bool direction_found = false;
	double ang_scan_step = step;
	for (double ang_scanned = 0; ang_scanned < maxstep; ang_scanned += ang_scan_step) {
		double tstr = gm.sonar_listen_ships(player, startangle).first;
		if (tstr >= peak_val) {
			// getting closer to peak
			ang_peak = startangle;
			peak_val = tstr;
			direction_found = true;
		} else if (direction_found) {
			// missed peak, back
			startangle -= step;
			break;
		} else {
			// wrong direction on first try, find new peak.
			peak_val = tstr;
			ang_scanned -= ang_scan_step;
			step = -step;
			direction_found = true;
		}
		startangle += step;
	}

	return make_pair(ang_peak, peak_val);
}




void sub_kdb_display::display(game& gm) const
{
	submarine* player = dynamic_cast<submarine*>(gm.get_player());

	sys().prepare_2d_drawing();

	// get hearing device angle from submarine, if it has one

	if (!myscheme.get()) throw error("sub_kdb_display::display without scheme!");
	const scheme& s = *myscheme;

	s.background->draw(0, 0);
	s.volume_knob[unsigned(myfmod(-turnknobang[TK_VOLUME]*0.5f, 90.0f)) * TK_PHASES / 90].draw();
	s.turn_wheel[unsigned(myfmod(-turnknobang[TK_DIRECTION] * 2.0f, 90.0f)) * TK_PHASES / 90].draw();
	s.direction_ptr.draw(turnknobang[TK_DIRECTION] * 0.5f /* fixme: get angle from player*/);

	// fixme: some/most of this code should be moved to sonar.cpp

	// test hack: test signal strengths
	angle app_ang = angle(turnknobang[TK_DIRECTION]*0.5);
	pair<double, noise> nstr = gm.sonar_listen_ships(player, app_ang);
// 	printf("noise strengths, rel ang=%f, L=%f M=%f H=%f U=%f TTL=%f\n",
// 	       app_ang.value(), nstr.second.frequencies[0], nstr.second.frequencies[1], nstr.second.frequencies[2], nstr.second.frequencies[3],
// 	       nstr.first);
	//shipclass cls = noise_signature::determine_shipclass_by_signal(noise_strengths);
	//printf("ship class is %i\n", cls);

	// find peak value.
	pair<angle, double> pkc = find_peak_noise(angle(0), 3.0, 360.0, gm);
// 	printf("peak found (%f) somewhere near %f\n", pkc.second, pkc.first.value());
	pkc = find_peak_noise(pkc.first, 1.0, 6.0, gm);
// 	printf("peak found (%f) closer, somewhere near %f\n", pkc.second, pkc.first.value());

	// simulate sonarman
	//sonarman.simulate(gm, 0.016666);	// 60fps, fixme ugly hack

	// fixme: add test here
	// Simulate sonar man.
	// From current apparatus angle turn some degrees left or right, until the operator
	// can tell wether the signal gets stronger or weaker.
	// If it gets weaker, choose the other direction as initial direction.
	// Then turn the apparatus by larger steps (10-30 degrees), as long as signal gets
	// stronger. If it gets weaker, try again from strongest direction with half the
	// distance, but with opposite direction. If that direction gives weaker signal on
	// second try, reverse direction (same as global search, but with less step length).
	// If we found two angles where the signal gets weaker in between, try from the strongest
	// and reverse direction with 1° steps iterativly.
	// Example: strongest signal at 33°, initial angle at 50°, initial direction right/
	// clockwise. Operator turns at 60° and hears that signal is weaker, so he uses
	// 50° as initial angle and left/counter-clockwise as initial direction.
	// He turns left 10° to 40°, signal gets stronger.
	// He turns left 10° to 30°, signal gets stronger.
	// He turns left 10° to 20°, signal gets weaker. So he changes direction, back at 30°. Stepping down to 5°.
	// He turns right 5° to 35°, signal gets stronger.
	// He turns right 5° to 40°, signal gets weaker. So he changes direction, back at 35°. Stepping down to 1°.
	// He turns left 1° to 34°, signal gets stronger.
	// He turns left 1° to 33°, signal gets stronger.
	// He turns left 1° to 32°, signal gets weaker. So he turns back to strongest signal and stops. -> 33°
	// Example2: strongest signal at 29°, initial angle at 50°, initial direction right/
	// clockwise. Operator turns at 60° and hears that signal is weaker, so he uses
	// 50° as initial angle and left/counter-clockwise as initial direction.
	// He turns left 10° to 40°, signal gets stronger.
	// He turns left 10° to 30°, signal gets stronger.
	// He turns left 10° to 20°, signal gets weaker. So he changes direction, back at 30°. Stepping down to 5°.
	// He turns right 5° to 35°, signal gets weaker. So initial direction wrong, turn left.
	// He turns left 5° to 25°, signal gets weaker. So he changes direction, back at 30°. Stepping down to 1°.
	// He turns right 1° to 31°, signal gets weaker. So initial direction wrong, turn left.
	// He turns left 1° to 29°, signal gets stronger.
	// He turns left 1° to 28°, signal gets weaker. So he turns back to strongest signal and stops. -> 29°
	// We could simulate four steps. 30°, 10°, 5°, 1°.
	// fixme 2:
	// but user would turn sonar in rather small steps (1° ?)
	// so simulation is simpler: turn apparatus in 1-3° steps and recognize peak, localize
	// peak afterwards in smaller steps (1°).

	ui.draw_infopanel();

	sys().unprepare_2d_drawing();
}



void sub_kdb_display::enter(bool is_day)
{
	myscheme.reset(new scheme(is_day));
}



void sub_kdb_display::leave()
{
	myscheme.reset();
}
