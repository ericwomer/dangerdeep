// ai
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "ai.h"
#include "game.h"

void ai::search_enemy(void)
{
	if (state != attackenemy)
		state = searchenemy;
}

void ai::attack(sea_object* t)
{
	target = t;
	if (target != 0) {
		state = attackenemy;
		parent->set_throttle(sea_object::aheadflank);
	} else {
		state = searchenemy;
		parent->set_throttle(sea_object::aheadsonar);
	}
}

void ai::follow(sea_object* t)
{
	target = t;
	state = followtarget;
}

void ai::act(class game& gm, double delta_time)
{
	if (gm.get_time() < last_thought_time + AI_THINK_CYCLE_TIME) {
		return;
	} else {
		last_thought_time = gm.get_time();
	}

	switch (type) {
		case escort: act_escort(gm, delta_time); break;
		default: act_dumb(gm, delta_time); break;
	}
	
	if (zigzagstate > 0) {
		if (zigzagstate == 5)
			parent->rudder_left(0.5);
		else if (zigzagstate == 15)
			parent->rudder_right(0.5);
		++zigzagstate;
		if (zigzagstate > 20)
			zigzagstate = 1;
	}
}

void ai::set_zigzag(bool stat)
{
	if (stat)
		zigzagstate = 1;
	else
		zigzagstate = 0;
}

void ai::act_escort(game& gm, double delta_time)
{
	if (state == followpath || state == followtarget) {
		act_dumb(gm, delta_time);
	} else if (state == searchenemy) {	// search activly for subs
		// watch around	
		list<submarine*> subs = gm.get_submarines();
		double dist = AI_AWARENESS_DISTANCE;
		for (list<submarine*>::iterator it = subs.begin(); it != subs.end(); ++it) {
			double d = (*it)->get_pos().xy().distance(parent->get_pos().xy());
			if (d < dist) {
				if (!parent->can_see(*it)) continue;
				attack(*it);
			}
		}
		if (state == attackenemy) return;	// further actions next turn
		
		// ping around to find something
		list<sea_object*> contacts = gm.ping_ASDIC(parent->get_pos().xy(),
			parent->get_heading()+angle(rand()%360));	//fixme
		if (contacts.size() > 0) {
			// fixme: add noise to position!
			// fixme: choose best contact!
			attack(contacts.front());
			return;
		}
		
		// nothing found?
		state = (target != 0) ? followtarget : followpath;
		act_dumb(gm, delta_time);
		state = searchenemy;

	} else if (state == attackenemy) {	// attack target
/*
		if (!parent->can_see(target)) {
			target = 0;
			state = searchenemy;
			return;
		}
*/		
		set_course_to_target();
		if (target->get_pos().xy().distance(parent->get_pos().xy()) < 100) {
			gm.spawn_depth_charge(new depth_charge(*parent, -target->get_pos().z));
			// fixme: just ai hacking/testing.
			// after spawning a DC start pinging again.
			if (!parent->can_see(target)) {
				target = 0;
				state = searchenemy;
				return;
			}
		}
	}
}

void ai::set_course_to_target(void)
{
	set_course_to_pos(target->get_pos().xy());
}

void ai::set_course_to_pos(const vector2& pos)
{
	vector2 d = pos - parent->get_pos().xy();
	vector2 hd = parent->get_heading().direction();
	double a = d.x*hd.x + d.y*hd.y;
	double b = d.x*hd.y - d.y*hd.x;
	// if a is < 0 then target lies behind our pos.
	// if b is < 0 then target is left, else right of our pos.
	double r1 = (b == 0) ? 1e10 : (a*a + b*b)/fabs(2*b);
	double r2 = 1.0/parent->get_turn_rate().rad();
	if (a <= 0) {	// target is behind us
		if (b < 0) {	// target is left
			parent->head_to_ang(parent->get_heading() - angle(180), true);
		} else {
			parent->head_to_ang(parent->get_heading() + angle(180), false);
		}
	} else if (r2 > r1) {	// target can not be reached with smallest curve possible
		if (b < 0) {	// target is left
			parent->head_to_ang(parent->get_heading() + angle(180), false);
		} else {
			parent->head_to_ang(parent->get_heading() - angle(180), true);
		}
	} else {	// target can be reached, steer curve
		parent->head_to_ang(angle::from_math(atan2(d.y, d.x)), (b < 0));
//	this code computes the curve that hits the target
//	but it is much better to turn fast and then steam straight ahead
/*
		double needed_turn_rate = (r1 == 0) ? 0 : 1.0/r1; //parent->speed/r1;
		double fac = ((180.0*needed_turn_rate)/PI)/fabs(parent->turn_rate.value_pm180());
		parent->head_chg = (b < 0) ? -fac : fac;
*/		
	}
}

void ai::act_dumb(game& gm, double delta_time)
{
	if (state == followtarget) {
		set_course_to_target();
	} else if (state == followpath) {
		if (waypoints.size() > 0) {
			set_course_to_pos(waypoints.front());
			if (parent->get_pos().xy().square_distance(waypoints.front()) < WPEXACTNESS*WPEXACTNESS) {
				if (cyclewaypoints)
					waypoints.push_back(waypoints.front());
				waypoints.erase(waypoints.begin());
			}
		}
	}
}
