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

// Sonar operator simulation
// subsim (C) + (W). See LICENSE

#include "sonar_operator.h"
#include "submarine.h"
#include "game.h"

using std::pair;

const double sonar_operator::turn_speed_fast = 6.0;	// degrees per second.
const double sonar_operator::turn_speed_slow = 2.0;	// degrees per second.
const double sonar_operator::simulation_step = 0.1;	// in seconds

sonar_operator::sonar_operator()
	: state(initial),
	  current_signal_strength(-1),
	  last_simulation_step_time(0)
{
}



/* how the sonar operator works:
   He turns the apparatus at constant speed around the compass, with 2 or 3 degrees per
   simulation step, total speed of 6 degrees per second or so.
   When he detects a transition from growing strength to falling strengh of the signal,
   he turns the apparatus in opposite direction with slower speed (0.5-1 degree per
   simulation step) and detects the angle where the signal reaches its maximum
   (upper limit), turns back until signal gets weaker (lower limit), then
   reports the mid between the lower and upper limit as contact, turns the apparatus
   back to upper limit and starts over.
   If a signal is very strong and thus close, he switches to tracking mode.
   In tracking mode he turns the apparatus around +- 30 degrees around the strongest signal
   and redects its position with the mentioned algorithm until the signal gets weaker.
   Or he turns the apparatus away from current peak angle until signal gets weaker, then back.
   Always searching for the limit of the signal, following a direction only if signal
   gets stronger along the direction. Direction is constantly changed between clockwise
   and counter-clockwise in that case.
   fixme - when does the operator choose the frequency band switch? This must depend
   on how far/strong a signal is...
   So we have several states (circular mostly)
   1) Just turning and listening to strengths -> detect signal with growing strength
                                                 when new strength > old strength, then to 2)
   2) Following signal increase -> detect weakening of signal, then to 3)
   3) Detect upper limit by finer turning in opposite direction, when new signal is <= current
      signal. Equal -> 4), less -> 5)
   4) Detect lower limit of signal by finer turning, until new signal is weaker than current,
                                                     then 5)
   5) report mid of lower and upper as new contact, turn current angle to upper with normal speed
                                                    then back to 1)
   5) is same as one...
*/
void sonar_operator::simulate(game& gm, double delta_t)
{
	last_simulation_step_time += delta_t;
// 	printf("delta_t=%f last_simulation_step_time=%f simulation_step=%f\n",
// 	       delta_t, last_simulation_step_time, simulation_step);
	if (last_simulation_step_time < simulation_step)
		return;

	// fixme: detection is very crude, because the operator doesn't use frequency band pass filters.
	// When passing several ships closely, only 1-2 signals are reported, although
	// you can see more definitive peaks in the higher frequencies.

// 	printf("sonarman sim, time=%f\n", last_simulation_step_time);
	last_simulation_step_time -= simulation_step;

	submarine* player = dynamic_cast<submarine*>(gm.get_player());
	angle sub_heading = player->get_heading();
	double sub_turn_velocity = -player->get_turn_velocity();
	pair<double, noise> signal = gm.sonar_listen_ships(player, current_angle);

	// fixme: use integer dB values for simulation? we round to dB anyway!
// 	printf("sonar man sim, angle=%f str=%f stat=%i\n", current_angle.value(), current_signal_strength, state);

	switch (state) {
	case initial:
		current_signal_strength = signal.first;
		state = find_growing_signal;
		break;
	case find_growing_signal:
		if (signal.first > current_signal_strength) {
			state = find_max_peak_coarse;
		} else {
			advance_angle_and_erase_old_contacts((turn_speed_fast - sub_turn_velocity)
							     * simulation_step, sub_heading);
		}
		current_signal_strength = signal.first;
		break;
	case find_max_peak_coarse:
		if (signal.first < current_signal_strength) {
			// we found the upper limit or just passed it
 			find_peak_upper_limit = current_angle;
 			find_peak_lower_limit = current_angle - angle(turn_speed_fast);
 			find_peak_try = 0;
			state = find_max_peak_fine;
			current_angle -= angle(turn_speed_slow * simulation_step);
		} else {
			advance_angle_and_erase_old_contacts((turn_speed_fast - sub_turn_velocity)
							     * simulation_step, sub_heading);
		}
		current_signal_strength = signal.first;
		break;
	case find_max_peak_fine:
		if (signal.first > current_signal_strength) {
			// we finally found an angle where signal is stronger.
			// detect signal type and maybe switch to tracking mode here
			// (if signal is very strong)
			// note that we should use the frequency bandpass filter here
			// to localize the signal even better  (fixme 2x)
			shipclass sc = signal.second.determine_shipclass();
// 			printf("sonar man sim, Peak of signal found at angle %f, strength %f, class %i\n",
// 			       current_angle.value(), signal.first, sc);
			add_contact(current_angle + sub_heading, contact(signal.first, sc));
// 			printf("all contacts now:\n");
// 			for (std::map<double, contact>::iterator it = contacts.begin(); it != contacts.end(); ++it)
// 				printf("angle %f class %i strength %f\n", it->first, it->second.type, it->second.strength_dB);
			state = find_growing_signal;
			// advance angle after finding. this also done so that the new
			// contact is not erased directly after reporting it...
			current_angle += angle((turn_speed_fast - sub_turn_velocity) * simulation_step);
		} else {
			angle add = angle(( ((find_peak_try & 1) ? 1 : -1) * turn_speed_slow
					   - sub_turn_velocity) * simulation_step);
			if (keeps_in_find_peak_limit(add)) {
				// continue turning in that direction
				current_angle += add;
			} else {
				// out of valid area for searching peak, try opposite direction
				++find_peak_try;
// 				printf("reached border, try another directio, currang=%f, fpt=%u", current_angle.value(), find_peak_try);
				if (find_peak_try > 1) {
					// tried once ccw and once cw, abort
// 					printf("couldn't find fine peak angle, report current angle as contact\n");
					shipclass sc = signal.second.determine_shipclass();
					add_contact(current_angle + sub_heading, contact(signal.first, sc));
					state = find_growing_signal;
					current_angle += angle((turn_speed_fast - sub_turn_velocity) * simulation_step);
				}
			}
		}
		current_signal_strength = signal.first;
		break;
	case track_signal:
		// fixme
		break;
	}
}



bool sonar_operator::keeps_in_find_peak_limit(angle add)
{
	// current_angle must be within find_peak_lower_limit, find_peak_upper_limit
	// check if current_angle + add is also within limit, if not return false,
	// if it is, add it
	angle new_ang = current_angle + add;
	if (find_peak_lower_limit.is_cw_nearer(new_ang) &&
	    new_ang.is_cw_nearer(find_peak_upper_limit)) {
		current_angle = new_ang;
		return true;
	}
	return false;
}



void sonar_operator::advance_angle_and_erase_old_contacts(double addang, angle sub_heading)
{
	double curr_angle = (current_angle + sub_heading).value();
	double next_angle = curr_angle + addang;
// 	printf("try next angle %f->%f\n", curr_angle, next_angle);
	// now erase all keys between current_angle and next_angle
	if (next_angle > curr_angle) {
		std::map<double, contact>::iterator beg = contacts.lower_bound(curr_angle);
		std::map<double, contact>::iterator end = contacts.upper_bound(next_angle);
		//  	printf("check delete %f->%f\n", curr_angle, next_angle);
		//  	for (std::map<double, contact>::iterator it = beg; it != end; ++it)
		//  		printf("making contact at angle %f obsolete\n", it->first);
		contacts.erase(beg, end);
		if (next_angle >= 360.0) {
			curr_angle -= 360.0;
			next_angle -= 360.0;
			beg = contacts.lower_bound(curr_angle);
			end = contacts.upper_bound(next_angle);
			// 		printf("check delete %f->%f\n", curr_angle, next_angle);
			// 		for (std::map<double, contact>::iterator it = beg; it != end; ++it)
			// 			printf("making contact at angle %f obsolete\n", it->first);
			contacts.erase(beg, end);
		}
	}
	// do not set from next_angle here, because that contains the sub's heading as additional factor
	// but add addang to current_angle. In short, don't mix relative and absolute angles! be careful...
	current_angle += angle(addang);
}



void sonar_operator::add_contact(angle absang, const contact& ct)
{
	// erase any values within +-n degree range around the new contact, could be same contact
	// when sub is turning
	const angle limit(2.0);
	angle abeg = absang - limit;
	angle aend = absang + limit;
	double ab = abeg.value();
	double ae = aend.value();
	// handle 360->0 wrap
	if (ab < ae) {
		contacts.erase(contacts.lower_bound(ab), contacts.upper_bound(ae));
	} else {
		contacts.erase(contacts.lower_bound(ab), contacts.upper_bound(360.0));
		contacts.erase(contacts.lower_bound(0.0), contacts.upper_bound(ae));
	}
// 	printf("add contact at %f\n", absang.value());
	contacts[absang.value()] = ct;
}



void sonar_operator::load(const xml_elem& parent)
{
}



void sonar_operator::save(xml_elem& parent) const
{
	xml_elem so = parent.add_child("sonar_operator");

// 	states state;
// 	angle current_angle;
// 	double current_signal_strength;
// 	std::map<double, contact> contacts;
// 	bool active;
// 	double last_simulation_step_time;

// 	dv.set_attr(dive_speed, "dive_speed");
// 	dv.set_attr(max_depth, "max_depth");
// 	dv.set_attr(dive_to, "dive_to");
// 	dv.set_attr(permanent_dive, "permanent_dive");
}
