// submarines
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "submarine.h"
#include "model.h"
#include "game.h"

submarine::submarine(unsigned type_, const vector3& pos, angle heading) : ship()
{
	type = type_;
	position = pos;
	this->heading = heading;
	head_to = heading;
	dive_speed = 0;
	permanent_dive = false;
	dive_to = position.z;
	max_dive_speed = 1;
	dive_acceleration = 0;
	scopeup = false;
	switch (type_) {
		case typeVIIc:
			speed = 0;
			max_speed = kts2ms(17.7);
			max_submerged_speed = kts2ms(7.6);
			max_rev_speed = kts2ms(5);
			acceleration = 0.6;
			turn_rate = 0.2;
			length = subVII->get_length();
			width = subVII->get_width();
			max_depth = 220+rnd(20);
			torpedoes.resize(14, stored_torpedo(torpedo::T3));
			nr_bow_tubes = 4;
			nr_stern_tubes = 1;
			nr_bow_storage = 6;
			nr_stern_storage = 1;
			nr_bow_top_storage = 1;
			nr_stern_top_storage = 1;
			torpedoes[4].type = torpedoes[11].type = torpedo::T5;
			torpedoes[5].type = torpedoes[6].type = torpedo::T3FAT;
			torpedoes[12].type = torpedoes[13].type = torpedo::T1;
			break;
		case typeXXI:
			speed = 0;
			max_speed = kts2ms(15.6);
			max_submerged_speed = kts2ms(17);
			max_rev_speed = kts2ms(5);
			acceleration = 0.6;
			turn_rate = 0.2;
			length = subXXI->get_length();
			width = subXXI->get_width();
			max_depth = 280+rnd(20);	// fixme > planned values, protypes only 170
			torpedoes.resize(23, stored_torpedo(torpedo::T6LUT));
			nr_bow_tubes = 6;
			nr_stern_tubes = 0;
			nr_bow_storage = 17;
			nr_stern_storage = 0;
			nr_bow_top_storage = 0;
			nr_stern_top_storage = 0;
			torpedoes[2].type = torpedoes[5].type = torpedoes[8].type =
				torpedoes[11].type = torpedoes[14].type = torpedoes[17].type =
				torpedoes[20].type = torpedo::T11;
				
			break;
	}
}

bool submarine::transfer_torpedo(unsigned from, unsigned to, double timeneeded)
{
	if (torpedoes[from].status == 3 && torpedoes[to].status == 0) {
		torpedoes[to].type = torpedoes[from].type;
		torpedoes[from].status = 2;
		torpedoes[to].status = 1;
		torpedoes[from].associated = to;
		torpedoes[to].associated = from;
		torpedoes[from].remaining_time =
			torpedoes[to].remaining_time = timeneeded;
		return true;
	}
	return false;
}

int submarine::find_stored_torpedo(bool usebow)
{
	pair<unsigned, unsigned> indices = (usebow) ? get_bow_storage_indices() : get_stern_storage_indices();
	int tubenr = -1;
	for (unsigned i = indices.first; i < indices.second; ++i) {
		if (torpedoes[i].status == 3) {	// loaded
			tubenr = i; break;
		}
	}
	return tubenr;
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
		
	// torpedo transfer
	for (unsigned i = 0; i < torpedoes.size(); ++i) {
		stored_torpedo& st = torpedoes[i];
		if (st.status == 1 || st.status == 2) { // reloading/unloading
			st.remaining_time -= delta_time;
			if (st.remaining_time <= 0) {
				if (st.status == 1) {	// reloading
					st.status = 3;	// loading
					torpedoes[st.associated].status = 0;	// empty
				} else {		// unloading
					st.status = 0;	// empty
					torpedoes[st.associated].status = 3;	// loaded
				}
			}
		}
	}

	// automatic reloading if desired	
	if (true /*automatic_reloading*/) {
		pair<unsigned, unsigned> bow_tube_indices = get_bow_tube_indices();
		pair<unsigned, unsigned> stern_tube_indices = get_stern_tube_indices();
		for (unsigned i = bow_tube_indices.first; i < bow_tube_indices.second; ++i) {
			if (torpedoes[i].status == 0) {
				int reload = find_stored_torpedo(true);		// bow
				if (reload >= 0) {
					transfer_torpedo(reload, i);
				}
			}
		}
		for (unsigned i = stern_tube_indices.first; i < stern_tube_indices.second; ++i) {
			if (torpedoes[i].status == 0) {
				int reload = find_stored_torpedo(false);	// stern
				if (reload >= 0) {
					transfer_torpedo(reload, i);
				}
			}
		}
	}
}

void submarine::display(void) const
{
	switch(type) {
		case typeVIIc: subVII->display(); break;
		case typeXXI: subXXI->display(); break;
	}
}

pair<unsigned, unsigned> submarine::get_bow_tube_indices(void) const
{
	return make_pair(0, nr_bow_tubes);
}

pair<unsigned, unsigned> submarine::get_stern_tube_indices(void) const
{
	return make_pair(nr_bow_tubes, nr_bow_tubes+nr_stern_tubes);
}

pair<unsigned, unsigned> submarine::get_bow_storage_indices(void) const
{
	unsigned offset = nr_bow_tubes+nr_stern_tubes;
	return make_pair(offset, offset+nr_bow_storage);
}

pair<unsigned, unsigned> submarine::get_stern_storage_indices(void) const
{
	unsigned offset = nr_bow_tubes+nr_stern_tubes+nr_bow_storage;
	return make_pair(offset, offset+nr_stern_storage);
}

pair<unsigned, unsigned> submarine::get_bow_top_storage_indices(void) const
{
	unsigned offset = nr_bow_tubes+nr_stern_tubes+nr_bow_storage+nr_stern_storage;
	return make_pair(offset, offset+nr_bow_top_storage);
}

pair<unsigned, unsigned> submarine::get_stern_top_storage_indices(void) const
{
	unsigned offset = nr_bow_tubes+nr_stern_tubes+nr_bow_storage+nr_stern_storage+
		nr_bow_top_storage;
	return make_pair(offset, offset+nr_stern_top_storage);
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
	pair<unsigned, unsigned> bow_tube_indices = get_bow_tube_indices();
	pair<unsigned, unsigned> stern_tube_indices = get_stern_tube_indices();
	unsigned torpnr = 0xffff;	// some high illegal value
	if (tubenr < 0) {
		if (usebowtubes) {
			for (unsigned i = bow_tube_indices.first; i < bow_tube_indices.second; ++i) {
				if (torpedoes[i].status == 3) {
					torpnr = i;
					break;
				}
			}
		} else {
			for (unsigned i = stern_tube_indices.first; i < stern_tube_indices.second; ++i) {
				if (torpedoes[i].status == 3) {
					torpnr = i;
					break;
				}
			}
		}
	} else {
		unsigned d = (usebowtubes) ? bow_tube_indices.second - bow_tube_indices.first :
			stern_tube_indices.second - stern_tube_indices.first;
		if (tubenr >= 0 && tubenr < d)
			torpnr = tubenr + ((usebowtubes) ? bow_tube_indices.first : stern_tube_indices.first);
	}
	if (torpnr == 0xffff)
		return false;
		
	torpedo* t = new torpedo(this, torpedoes[torpnr].type, usebowtubes);
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
	torpedoes[torpnr].status = 0;
	return true;
}
