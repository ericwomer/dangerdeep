// submarines
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "submarine.h"
#include "model.h"
#include "game.h"

submarine::submarine(unsigned type_, const vector3& pos, angle heading)
{
	init_empty();
	type = type_;
	position = pos;
	this->heading = heading;
	head_to = heading;
	hitpoints = 1;
	dive_speed = 0;
	permanent_dive = false;
	dive_to = position.z;
	max_dive_speed = 1;
	dive_acceleration = 0;
	switch (type_) {
		case typeVIIc:
			description = "Submarine type VIIc";
			speed = 0;
			max_speed = kts2ms(17.7);
			max_submerged_speed = kts2ms(7.6);
			max_rev_speed = kts2ms(5);
			acceleration = 0.6;
			turn_rate = 0.2;
			length = subVII->get_length();
			width = subVII->get_width();
			max_depth = 220+rand()%20;
			bow_tubes.resize(4, torpedo::T3);
			stern_tubes.resize(1, torpedo::T5);
			bow_storage.resize(6, torpedo::T1);
			bow_storage[5] = torpedo::T3FAT;
			stern_storage.resize(1, torpedo::T3);
			bow_top_storage.resize(1, torpedo::T1);
			stern_top_storage.resize(1, torpedo::T1);
			break;
		case typeXXI:
			description = "Submarine type XXI";
			speed = 0;
			max_speed = kts2ms(15.6);
			max_submerged_speed = kts2ms(17);
			max_rev_speed = kts2ms(5);
			acceleration = 0.6;
			turn_rate = 0.2;
			length = subXXI->get_length();
			width = subXXI->get_width();
			max_depth = 280+rand()%20;	// fixme > planned values, protypes only 170
			bow_tubes.resize(6, torpedo::T6LUT);
			stern_tubes.resize(0, torpedo::none);
			bow_storage.resize(17, torpedo::T6LUT);
			bow_tubes[2] = bow_tubes[5] = torpedo::T11;
			bow_storage[0] = bow_storage[4] = bow_storage[8] = bow_storage[12] = 
				torpedo::T11;
			stern_storage.resize(0, torpedo::none);
			bow_top_storage.resize(0, torpedo::none);
			stern_top_storage.resize(0, torpedo::none);
			break;
	}
}

void submarine::simulate(class game& gm, double delta_time)
{
	sea_object::simulate(gm, delta_time);

	// calculate new depth (fixme this is not physically correct)
	double delta_depth = dive_speed * delta_time;
	if (dive_speed != 0) {
		if (permanent_dive) {
			position.z += delta_depth;
		} else {
			double fac = (dive_to - position.z)/delta_depth;
			if (0 <= fac && fac <= 1) {
				position.z = dive_to;
				planes_middle();
			} else {
				position.z += delta_depth;
			}
		}
	}
	if (position.z > 0) {
		position.z = 0;
		dive_speed = 0;
	}

	// fixme: the faster the sub goes, the faster it can dive.

	// fixme: this is simple and not realistic. and the values are just guessed		
//	double water_resistance = -dive_speed * 0.5;
//	dive_speed += delta_time * (2*dive_acceleration + water_resistance);
//	if (dive_speed > max_dive_speed)
//		dive_speed = max_dive_speed;
//	if (dive_speed < -max_dive_speed)
//		dive_speed = -max_dive_speed;
		
	if (-position.z > max_depth)
		kill();
}

void submarine::display(void) const
{
	switch(type) {
		case typeVIIc: subVII->display(); break;
		case typeXXI: subXXI->display(); break;
	}
}

double submarine::get_max_speed(void) const
{
	return (get_pos().z < 0) ? max_submerged_speed : max_speed;
}

void submarine::planes_up(double amount)
{
//	dive_acceleration = -1;
	dive_speed = max_dive_speed;
	permanent_dive = true;
}

void submarine::planes_down(double amount)
{
//	dive_acceleration = 1;
	dive_speed = -max_dive_speed;
	permanent_dive = true;
}

void submarine::planes_middle(void)
{
//	dive_acceleration = 0;
	dive_speed = 0;
	permanent_dive = false;
	dive_to = position.z;
}

void submarine::dive_to_depth(unsigned meters)
{
	dive_to = -int(meters);
	permanent_dive = false;
	dive_speed = (dive_to < position.z) ? -max_dive_speed : max_dive_speed;
}

bool submarine::fire_torpedo(class game& gm, bool usebowtubes, int tubenr,
	sea_object* target)
{
	unsigned ttype = torpedo::none;
	if (usebowtubes) {
		if (tubenr < 0) {
			for (unsigned i = 0; i < bow_tubes.size(); ++i) {
				if (bow_tubes[i] == torpedo::none || bow_tubes[i] == torpedo::reloading) {
					continue;
				} else {
					tubenr = i; break;
				}
			}
		}
		if (tubenr < 0 || tubenr >= bow_tubes.size()) return false;
		ttype = bow_tubes[tubenr];
	} else {
		if (tubenr < 0) {
			for (unsigned i = 0; i < stern_tubes.size(); ++i) {
				if (stern_tubes[i] == torpedo::none || stern_tubes[i] == torpedo::reloading) {
					continue;
				} else {
					tubenr = i; break;
				}
			}
		}
		if (tubenr < 0 || tubenr >= stern_tubes.size()) return false;
		ttype = stern_tubes[tubenr];
	}
	if (ttype == torpedo::none || ttype == torpedo::reloading)
		return false;
		
	torpedo* t = new torpedo(this, ttype);
	if (target) {
		if (t->adjust_head_to(target, usebowtubes)) {
			gm.spawn_torpedo(t);
		} else {
			// gyro angle invalid
			delete t;
			return false;
		}
	} else {
		gm.spawn_torpedo(t);
	}
	if (usebowtubes) {
		bow_tubes[tubenr] = torpedo::none;
	} else {
		stern_tubes[tubenr] = torpedo::none;
	}
	return true;
}
