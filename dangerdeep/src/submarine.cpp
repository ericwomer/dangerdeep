// submarines
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "submarine.h"
#include "model.h"
#include "game.h"
#include "submarine_VIIc.h"
#include "submarine_XXI.h"
#include "tokencodes.h"
#include "sensors.h"

submarine::submarine() : ship(), dive_speed(0.0f), permanent_dive(false),
    dive_to(0.0f), max_dive_speed(1.0f), dive_acceleration(0.0f), scopeup(false),
    max_depth(150.0f), periscope_depth(12.0f), snorkel_depth(-1.0f)
{}
	
bool submarine::parse_attribute(parser& p)
{
	if (ship::parse_attribute(p)) return true;
	switch (p.type()) {
		case TKN_SCOPEUP:
			p.consume();
			p.parse(TKN_ASSIGN);
			scopeup = p.parse_bool();
			p.parse(TKN_SEMICOLON);
			break;
		case TKN_MAXDEPTH:
			p.consume();
			p.parse(TKN_ASSIGN);
			max_depth = p.parse_number();
			p.parse(TKN_SEMICOLON);
			break;
		case TKN_TORPEDOES:
			p.consume();
			p.parse(TKN_SLPARAN);
			for (unsigned i = 0; i < torpedoes.size(); ++i) {
				switch (p.type()) {
					case TKN_TXTNONE: torpedoes[i].status = 0; break;
					case TKN_T1: torpedoes[i] = stored_torpedo(torpedo::T1); break;
					case TKN_T3: torpedoes[i] = stored_torpedo(torpedo::T3); break;
					case TKN_T5: torpedoes[i] = stored_torpedo(torpedo::T5); break;
					case TKN_T3FAT: torpedoes[i] = stored_torpedo(torpedo::T3FAT); break;
					case TKN_T6LUT: torpedoes[i] = stored_torpedo(torpedo::T6LUT); break;
					case TKN_T11: torpedoes[i] = stored_torpedo(torpedo::T11); break;
					default: p.error("Expected torpedo type");
				}
				p.consume();
				if (p.type() == TKN_SRPARAN) break;
				p.parse(TKN_COMMA);
			}
			p.parse(TKN_SRPARAN);
			break;
		default: return false;
	}

	// Activate electric engine if submerged.
	if (is_submerged())
	{
		electric_engine = true;
	}
    
	return true;
}

submarine* submarine::create(submarine::types type_)
{
	switch (type_) {
		case typeVIIc: return new submarine_VIIc();
		case typeXXI: return new submarine_XXI();
	}
	return 0;
}

submarine* submarine::create(parser& p)
{
	p.parse(TKN_SUBMARINE);
	int t = p.type();
	p.consume();
	switch (t) {
		case TKN_TYPEVIIC: return new submarine_VIIc(p);
		case TKN_TYPEXXI: return new submarine_XXI(p);
	}
	return 0;
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

	// Activate or deactivate electric engines.
	if ((position.z > -SUBMARINE_SUBMERGED_DEPTH) &&
		(position.z+delta_depth < -SUBMARINE_SUBMERGED_DEPTH))
	{
		// Activate electric engine.
		electric_engine = true;
	}
	else if ((position.z < -SUBMARINE_SUBMERGED_DEPTH) &&
		(position.z+delta_depth > -SUBMARINE_SUBMERGED_DEPTH))
	{
		// Activate diesel engine.
		electric_engine = false;
	}
    
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

double submarine::get_max_speed(void) const
{
	return (is_electric_engine())? max_submerged_speed : max_speed;
}

float submarine::surface_visibility(const vector2& watcher) const
{
	double depth = get_depth();
	float diveFactor = 0.0f;

	if ( depth >= 0.0f && depth < 10.0f )
	{
		diveFactor = 0.1f * ( 10.0f - depth ) * vis_cross_section_factor *
			get_profile_factor ( watcher );
	}

	// Add a value for the periscope when submarine is submerged.
	if ( is_scope_up () && depth > 10.0f && depth <= periscope_depth )
	{
		// The visibility of the periscope also depends on the speed its moves
		// through the water. A fast moving periscope with water splashed is
		// much farther visible than a still standing one.
		diveFactor += CROSS_SECTION_VIS_PERIS * ( 0.5f + 0.5f * speed / max_speed );
	}

	return diveFactor;
}

float submarine::sonar_visibility ( const vector2& watcher ) const
{
	double depth = get_depth();
	float diveFactor = 0.0f;

	if ( depth > 10.0f )
	{
		diveFactor = 1.0f;
	}
	else if ( (depth > SUBMARINE_SUBMERGED_DEPTH ) && ( depth < 10.0f ) )
	{
		// Submarine becomes visible for active sonar system while
		// diving process.
		diveFactor = 0.125f * (depth - SUBMARINE_SUBMERGED_DEPTH);
	}

	diveFactor *= sonar_cross_section_factor * get_profile_factor ( watcher );

	return diveFactor;
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

bool submarine::fire_torpedo(class game& gm, int tubenr, sea_object* target)
{
	pair<unsigned, unsigned> bow_tube_indices = get_bow_tube_indices();
	pair<unsigned, unsigned> stern_tube_indices = get_stern_tube_indices();

	bool usebowtubes = true;
	if (tubenr < 0) {
		if (target != 0) {
			angle a = angle(target->get_pos().xy() - get_pos().xy()) - get_heading();
			if (a.ui_abs_value180() > 90)
				usebowtubes = false;
		}
	} else {
		if (tubenr >= bow_tube_indices.first && tubenr < bow_tube_indices.second) {
			tubenr -= bow_tube_indices.first;
			usebowtubes = true;
		} else if (tubenr >= stern_tube_indices.first && tubenr < stern_tube_indices.second) {
			tubenr -= stern_tube_indices.first;
			usebowtubes = false;
		} else {
			return false;	// illegal tube nr
		}
	}

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

double submarine::get_noise_factor () const
{
	double noisefac = 1.0f;

	// Get engine noise factor.
	noisefac = sea_object::get_noise_factor ();

	// Noise level modification because of diesel/electric engine.
	if ( is_electric_engine () )
	{
		// This is an empirical value.
		noisefac *= 0.007f;
	}
	else
	{
		// This might be unrealistic, but its impossible for a submarine to
		// attack when it can be detected at a range of 10 nm by a drifting
		// and listening destroyer. Currently there are no empirical values
		// available.
		noisefac *= 0.1f;
	}

	return noisefac;
}
