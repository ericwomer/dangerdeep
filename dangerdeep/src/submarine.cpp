// submarines
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "model.h"
#include "game.h"
#include "tokencodes.h"
#include "sensors.h"
#include "date.h"
#include "submarine.h"
#include "depth_charge.h"
#include "tinyxml/tinyxml.h"

// fixme: this was hard work. most values are a rather rough approximation.
// if we want a real accurate simulation, these values have to be (re)checked.
// also they have to fit with "rect" values in sub_damage_display.cpp.
submarine::damage_data_scheme submarine::damage_schemes[submarine::nr_of_damageable_parts] = {
	damage_data_scheme(vector3f(0.090517,0.108696,0.892857), vector3f(0.066810,0.297101,0.083333), 0.2, 3600, true, true),	// rudder
	damage_data_scheme(vector3f(0.131466,0.086957,0.678571), vector3f(0.114224,0.253623,0.321429), 0.2, 3600, true, false),	// screws
	damage_data_scheme(vector3f(0.260776,0.130435,0.678571), vector3f(0.132543,0.231884,0.321429), 0.2, 3600, true, true),	// screw shaft
	damage_data_scheme(vector3f(0.112069,0.137681,0.880952), vector3f(0.087284,0.181159,0.119048), 0.2, 3600, true, true),	// stern dive planes
	damage_data_scheme(vector3f(0,0,0), vector3f(0,0,0), 0.2, 3600, false, true),	// stern water pump
	damage_data_scheme(vector3f(0.271552,0.108696,0.892857), vector3f(0.000000,0.543478,0.107143), 0.2, 3600, false, true),	// stern pressure hull
	damage_data_scheme(vector3f(0.273707,0.536232,0.607143), vector3f(0.248922,0.579710,0.392857), 0.2, 3600, false, true),	// stern hatch
	damage_data_scheme(vector3f(0.332974,0.094203,0.797619), vector3f(0.285560,0.246377,0.202381), 0.2, 3600, false, true),	// electric engines
	damage_data_scheme(vector3f(0,0,0), vector3f(0,0,0), 0.2, 3600, false, true),	// air compressor
	damage_data_scheme(vector3f(0,0,0), vector3f(0,0,0), 0.2, 3600, false, true),	// machine water pump
	damage_data_scheme(vector3f(0.451509,0.072464,0.833333), vector3f(0.271552,0.543478,0.190476), 0.2, 3600, false, true),	// machine pressure hull
	damage_data_scheme(vector3f(0.627155,0.086957,0.750000), vector3f(0.501078,0.253623,0.297619), 0.2, 3600, false, true),	// aft battery
	damage_data_scheme(vector3f(0.451509,0.086957,0.797619), vector3f(0.353448,0.268116,0.202381), 0.2, 3600, false, true),	// diesel engines
	damage_data_scheme(vector3f(0,0,0), vector3f(0,0,0), 0.2, 3600, false, true),	// kitchen hatch (replacce by second periscope?)
	damage_data_scheme(vector3f(0,0,0), vector3f(0,0,0), 0.2, 3600, false, true),	// balance tank valves
	damage_data_scheme(vector3f(0.728448,0.086957,0.750000), vector3f(0.627155,0.253623,0.297619), 0.2, 3600, false, true),	// forward battery
	damage_data_scheme(vector3f(0.535560,0.702899,0.559524), vector3f(0.528017,0.942029,0.476190), 0.2, 3600, true, true),	// periscope
	damage_data_scheme(vector3f(0.721983,0.072464,0.833333), vector3f(0.451509,0.543478,0.190476), 0.2, 3600, false, true),	// central pressure hull
	damage_data_scheme(vector3f(0,0,0), vector3f(0,0,0), 0.2, 3600, false, true),	// bilge water pump
	damage_data_scheme(vector3f(0.524784,0.710145,0.607143), vector3f(0.509698,0.760870,0.392857), 0.2, 3600, false, true),	// conning tower hatch
	damage_data_scheme(vector3f(0,0,0), vector3f(0,0,0), 0.2, 3600, false, true),	// listening device
	damage_data_scheme(vector3f(0,0,0), vector3f(0,0,0), 0.2, 3600, false, true),	// radio device
	damage_data_scheme(vector3f(0.828664,0.195652,0.678571), vector3f(0.818966,0.384058,0.369048), 0.2, 3600, false, true),	// inner bow tubes
	damage_data_scheme(vector3f(0.967672,0.195652,0.678571), vector3f(0.925647,0.384058,0.369048), 0.2, 3600, true, false),	// outer bow tubes
	damage_data_scheme(vector3f(0,0,0), vector3f(0,0,0), 0.2, 3600, false, true),	// bow water pump
	damage_data_scheme(vector3f(0.766164,0.536232,0.607143), vector3f(0.740302,0.579710,0.392857), 0.2, 3600, false, true),	// bow hatch
	damage_data_scheme(vector3f(0.915948,0.072464,0.833333), vector3f(0.721983,0.543478,0.190476), 0.2, 3600, false, true),	// bow pressure hull
	damage_data_scheme(vector3f(0.935345,0.115942,0.940476), vector3f(0.896552,0.181159,0.130952), 0.2, 3600, true, true),	// bow dive planes
	damage_data_scheme(vector3f(0.480603,0.731884,0.678571), vector3f(0.453664,0.898551,0.369048), 0.2, 3600, true, false),	// aa gun
	damage_data_scheme(vector3f(0.480603,0.579710,0.678571), vector3f(0.451509,0.681159,0.369048), 0.2, 3600, true, false),	// ammo depot
	damage_data_scheme(vector3f(0.672414,0.413043,0.190476), vector3f(0.299569,0.528986,0.000000), 0.2, 3600, true, false),	// outer fuel tanks left
	damage_data_scheme(vector3f(0.672414,0.413043,1.000000), vector3f(0.299569,0.528986,0.821429), 0.2, 3600, true, false),	// outer fuel tanks right
	damage_data_scheme(vector3f(0.062500,0.304348,0.607143), vector3f(0.043103,0.362319,0.392857), 0.2, 3600, true, false),	// outer stern tubes
	damage_data_scheme(vector3f(0.171336,0.304348,0.607143), vector3f(0.161638,0.362319,0.392857), 0.2, 3600, false, true),	// inner stern tubes
	damage_data_scheme(vector3f(0,0,0), vector3f(0,0,0), 0.2, 3600, true, true),	// snorkel
	damage_data_scheme(vector3f(0.656250,0.572464,0.642857), vector3f(0.586207,0.710145,0.357143), 0.2, 3600, true, false),	// deck gun
	damage_data_scheme(vector3f(0,0,0), vector3f(0,0,0), 0.2, 3600, true, true),	// radio detection device
	damage_data_scheme(vector3f(0,0,0), vector3f(0,0,0), 0.2, 3600, true, true),	// radar
};

submarine::submarine() : ship(), dive_speed(0.0f), dive_acceleration(0.0f), max_dive_speed(1.0f),
	max_depth(150.0f), dive_to(0.0f), permanent_dive(false),
	scopeup(false), periscope_depth(12.0f), hassnorkel (false), snorkel_depth(10.0f),
	snorkelup(false),
	battery_level ( 1.0f ), battery_value_a ( 0.0f ), battery_value_t ( 1.0f ),
	battery_recharge_value_a ( 0.0f ), battery_recharge_value_t ( 1.0f ),
	damageable_parts(nr_of_damageable_parts),
	trp_primaryrange(0), trp_secondaryrange(0), trp_initialturn(0), trp_searchpattern(0),
	trp_addleadangle(0)
{
	// set all common damageable parts to "no damage"
	for (unsigned i = 0; i < unsigned(outer_stern_tubes); ++i)
		damageable_parts[i] = damageable_part(0, 0);
}



submarine::submarine(const string& specfilename_) : ship(specfilename_)
{
	TiXmlDocument doc(get_ship_dir() + specfilename + ".xml");
	doc.LoadFile();
	TiXmlHandle hdoc(&doc);
	TiXmlHandle hdftdship = hdoc.FirstChild("dftd-ship");
/*
	TiXmlElement* eclassification = hdftdship.FirstChildElement("classification").Element();
	system::sys().myassert(eclassification != 0, string("classification node missing in ")+specfilename);
	modelname = eclassification->Attribute("modelname");
	string typestr = eclassification->Attribute("type");
	if (typestr == "warship") shipclass = WARSHIP;
	else if (typestr == "escort") shipclass = ESCORT;
	else if (typestr == "merchant") shipclass = MERCHANT;
	else system::sys().myassert(false, string("illegal ship type in ") + specfilename);
	string country = eclassification->Attribute("country");
	TiXmlHandle hdescription = hdftdship.FirstChild("description");//fixme: parse
	TiXmlElement* emotion = hdftdship.FirstChildElement("motion").Element();
	system::sys().myassert(emotion != 0, string("motion node missing in ")+specfilename);
	max_speed = atof(emotion->Attribute("maxspeed"));
	max_rev_speed = atof(emotion->Attribute("maxrevspeed"));
	acceleration = atof(emotion->Attribute("acceleration"));
	turn_rate = atof(emotion->Attribute("turnrate"));
	TiXmlElement* etonnage = hdftdship.FirstChildElement("tonnage").Element();
	system::sys().myassert(etonnage != 0, string("tonnage node missing in ")+specfilename);
	unsigned minton = atoi(etonnage->Attribute("min"));
	unsigned maxton = atoi(etonnage->Attribute("max"));
	tonnage = minton + rnd(maxton - minton + 1);
	TiXmlHandle hsmoke = hdftdship.FirstChild("smoke");//fixme parse
	TiXmlHandle hsensors = hdftdship.FirstChild("sensors");//fixme parse
	TiXmlElement* eai = hdftdship.FirstChildElement("ai").Element();
	system::sys().myassert(eai != 0, string("ai node missing in ")+specfilename);
	string aitype = eai->Attribute("type");
	if (aitype == "dumb") myai = new ai(this, ai::dumb);
	else if (aitype == "escort") myai = new ai(this, ai::escort);
	else system::sys().myassert(false, string("illegal AI type in ") + specfilename);
	TiXmlElement* efuel = hdftdship.FirstChildElement("fuel").Element();
	system::sys().myassert(efuel != 0, string("fuel node missing in ")+specfilename);
	fuel_level = atof(efuel->Attribute("capacity"));
	fuel_value_a = atof(efuel->Attribute("consumption_a"));
	fuel_value_t = atof(efuel->Attribute("consumption_t"));

	// fixme smoke
	mysmoke = 0;
*/
	// fixme: resize torpedoes vector according to # of tubes
	// compute max depth with rnd
	// get width and height from model! also in class ship!
	// create sensor arrays after data! also in class ship!
}



submarine::submarine(parser& p)
{
}


	
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
					case TKN_TXTNONE: torpedoes[i].status = stored_torpedo::st_empty; break;
					case TKN_T1: torpedoes[i] = stored_torpedo(torpedo::T1); break;
					case TKN_T2: torpedoes[i] = stored_torpedo(torpedo::T2); break;
					case TKN_T3: torpedoes[i] = stored_torpedo(torpedo::T3); break;
					case TKN_T3A: torpedoes[i] = stored_torpedo(torpedo::T3a); break;
					case TKN_T4: torpedoes[i] = stored_torpedo(torpedo::T4); break;
					case TKN_T5: torpedoes[i] = stored_torpedo(torpedo::T5); break;
					case TKN_T11: torpedoes[i] = stored_torpedo(torpedo::T11); break;
					case TKN_T1FAT: torpedoes[i] = stored_torpedo(torpedo::T1FAT); break;
					case TKN_T3FAT: torpedoes[i] = stored_torpedo(torpedo::T3FAT); break;
					case TKN_T6LUT: torpedoes[i] = stored_torpedo(torpedo::T6LUT); break;
					default: p.error("Expected torpedo type");
				}
				p.consume();
				if (p.type() == TKN_SRPARAN) break;
				p.parse(TKN_COMMA);
			}
			p.parse(TKN_SRPARAN);
			break;
		case TKN_SNORKEL:
			p.consume();
			p.parse(TKN_ASSIGN);
			hassnorkel = p.parse_bool();
			p.parse(TKN_SEMICOLON);
			break;
		case TKN_BATTERY:
			p.consume ();
			p.parse ( TKN_ASSIGN );
			battery_level = p.parse_number () / 100.0f;
			p.parse ( TKN_SEMICOLON );
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

void submarine::load(istream& in, game& g)
{
	ship::load(in, g);

//fixme: add values from xml read
	dive_speed = read_double(in);
	dive_acceleration = read_double(in);
	max_dive_speed = read_double(in);
	max_depth = read_double(in);
	dive_to = read_double(in);
	permanent_dive = read_bool(in);
	max_submerged_speed = read_double(in);

	torpedoes.clear();
	for (unsigned s = read_u8(in); s > 0; --s)
		torpedoes.push_back(stored_torpedo(in));

	scopeup = read_bool(in);
	periscope_depth = read_double(in);
	electric_engine = read_bool(in);
	hassnorkel = read_bool(in);
	snorkel_depth = read_double(in);
	snorkelup = read_bool(in);
	battery_level = read_double(in);
	battery_value_a = read_double(in);
	battery_value_t = read_double(in);
	battery_recharge_value_a = read_double(in);
	battery_recharge_value_t = read_double(in);
    
	damageable_parts.clear();
	for (unsigned s = read_u8(in); s > 0; --s)
		damageable_parts.push_back(damageable_part(in));
		
	trp_primaryrange = read_u8(in);
	trp_secondaryrange = read_u8(in);
	trp_initialturn = read_u8(in);
	trp_searchpattern = read_u8(in);
	trp_addleadangle = read_double(in);
}

void submarine::save(ostream& out, const game& g) const
{
	ship::save(out, g);

//fixme: add values from xml read
	write_double(out, dive_speed);
	write_double(out, dive_acceleration);
	write_double(out, max_dive_speed);
	write_double(out, max_depth);
	write_double(out, dive_to);
	write_bool(out, permanent_dive);
	write_double(out, max_submerged_speed);

	write_u8(out, torpedoes.size());
	for (vector<stored_torpedo>::const_iterator it = torpedoes.begin(); it != torpedoes.end(); ++it) {
		it->save(out);
	}

	write_bool(out, scopeup);
	write_double(out, periscope_depth);
	write_bool(out, electric_engine);
	write_bool(out, hassnorkel);
	write_double(out, snorkel_depth);
	write_bool(out, snorkelup);
	write_double(out, battery_level);
	write_double(out, battery_value_a);
	write_double(out, battery_value_t);
	write_double(out, battery_recharge_value_a);
	write_double(out, battery_recharge_value_t);
    
	write_u8(out, damageable_parts.size());
	for (vector<damageable_part>::const_iterator it = damageable_parts.begin(); it != damageable_parts.end(); ++it) {
		it->save(out);
	}
	
	write_u8(out, trp_primaryrange);
	write_u8(out, trp_secondaryrange);
	write_u8(out, trp_initialturn);
	write_u8(out, trp_searchpattern);
	write_double(out, trp_addleadangle.value());
}



void submarine::transfer_torpedo(unsigned from, unsigned to)
{
	if (torpedoes[from].status == stored_torpedo::st_loaded &&
			torpedoes[to].status == stored_torpedo::st_empty) {
		torpedoes[to].type = torpedoes[from].type;
		torpedoes[from].status = stored_torpedo::st_unloading;
		torpedoes[to].status = stored_torpedo::st_reloading;
		torpedoes[from].associated = to;
		torpedoes[to].associated = from;
		torpedoes[from].remaining_time =
		torpedoes[to].remaining_time = 
			get_torp_transfer_time(from, to);	// fixme: add time for torpedos already in transfer (one transfer may block another!)
	}
}

int submarine::find_stored_torpedo(bool usebow)
{
	pair<unsigned, unsigned> indices = (usebow) ? get_bow_storage_indices() : get_stern_storage_indices();
	int tubenr = -1;
	for (unsigned i = indices.first; i < indices.second; ++i) {
		if (torpedoes[i].status == stored_torpedo::st_loaded) {	// loaded
			tubenr = i; break;
		}
	}
	return tubenr;
}

void submarine::simulate(class game& gm, double delta_time)
{
	ship::simulate(gm, delta_time);

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
		if (st.status == stored_torpedo::st_reloading ||
			st.status == stored_torpedo::st_unloading) { // reloading/unloading
			st.remaining_time -= delta_time;
			if (st.remaining_time <= 0) {
				if (st.status == stored_torpedo::st_reloading) {	// reloading
					st.status = stored_torpedo::st_loaded;	// loading
//					torpedoes[st.associated].type = torpedo::none;
//					torpedoes[st.associated].status = stored_torpedo::st_empty;	// empty
				} else {		// unloading
					st.status = stored_torpedo::st_empty;	// empty
					st.type = torpedo::none;
//					torpedoes[st.associated].status = stored_torpedo::st_loaded;	// loaded
					// fixme: message: torpedo reloaded
				}
			}
		}
	}

	// automatic reloading if desired, fixed: move to fire_torpedo.
	if (false /*true*/ /*automatic_reloading*/) {
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


pair<unsigned, unsigned> submarine::get_bow_tube_indices(void) const
{
	unsigned off = 0;
	return make_pair(off, off+get_nr_of_bow_tubes());
}

pair<unsigned, unsigned> submarine::get_stern_tube_indices(void) const
{
	unsigned off = get_nr_of_bow_tubes();
	return make_pair(off, off+get_nr_of_stern_tubes());
}

pair<unsigned, unsigned> submarine::get_bow_storage_indices(void) const
{
	unsigned off = get_nr_of_bow_tubes()+get_nr_of_stern_tubes();
	return make_pair(off, off+get_nr_of_bow_reserve());
}

pair<unsigned, unsigned> submarine::get_stern_storage_indices(void) const
{
	unsigned off = get_nr_of_bow_tubes()+get_nr_of_stern_tubes()+get_nr_of_bow_reserve();
	return make_pair(off, off+get_nr_of_stern_reserve());
}

pair<unsigned, unsigned> submarine::get_bow_top_storage_indices(void) const
{
	unsigned off = get_nr_of_bow_tubes()+get_nr_of_stern_tubes()+get_nr_of_bow_reserve()+get_nr_of_stern_reserve();
	return make_pair(off, off+get_nr_of_bow_deckreserve());
}

pair<unsigned, unsigned> submarine::get_stern_top_storage_indices(void) const
{
	unsigned off = get_nr_of_bow_tubes()+get_nr_of_stern_tubes()+get_nr_of_bow_reserve()+get_nr_of_stern_reserve()+get_nr_of_bow_deckreserve();
	return make_pair(off, off+get_nr_of_stern_deckreserve());
}

unsigned submarine::get_location_by_tubenr(unsigned tn) const
{
	pair<unsigned, unsigned> idx = get_bow_tube_indices();
	if (tn >= idx.first && tn < idx.second) return 1;
	idx = get_stern_tube_indices();
	if (tn >= idx.first && tn < idx.second) return 2;
	idx = get_bow_storage_indices();
	if (tn >= idx.first && tn < idx.second) return 3;
	idx = get_stern_storage_indices();
	if (tn >= idx.first && tn < idx.second) return 4;
	idx = get_bow_top_storage_indices();
	if (tn >= idx.first && tn < idx.second) return 5;
	idx = get_stern_top_storage_indices();
	if (tn >= idx.first && tn < idx.second) return 6;
	return 0;
}


double submarine::get_torp_transfer_time(unsigned from, unsigned to) const
{
	unsigned fl = get_location_by_tubenr(from), tl = get_location_by_tubenr(to);
	if (fl == 0 || tl == 0) return 0.0;
	if (fl == tl) return 0.0;
	// possible path of transportation is: 1 <-> 3 <-> 5 <-> 6 <-> 4 <-> 2
	// each connection has a type specific time
	unsigned transl[7] = { 0, 1, 6, 2, 5, 3, 4 };	// translate to linear order
	unsigned flin = transl[fl], tlin = transl[tl];
	if (flin > tlin) { unsigned tmp = flin; flin = tlin; tlin = tmp; }
	double tm = 0.0;
	for (unsigned i = flin; i < tlin; ++i) {
		switch(i) {
			case 1: tm += get_bow_reload_time(); break;
			case 2: tm += get_bow_deck_reload_time(); break;
			case 3: tm += get_bow_stern_deck_transfer_time(); break;
			case 4: tm += get_stern_deck_reload_time(); break;
			case 5: tm += get_stern_reload_time(); break;
		}
	}
	return tm;
}


double submarine::get_max_speed(void) const
{
	double ms;

	if ( is_electric_engine() )
	{
		ms = max_submerged_speed;
	}
	else
	{
		ms = ship::get_max_speed ();

		// When submarine is submerged and snorkel is used the maximum
		// diesel speed is halved.
		if ( has_snorkel() && is_submerged () && snorkelup )
			ms *= 0.5f;
	}

	return ms;
}

float submarine::surface_visibility(const vector2& watcher) const
{
	double depth = get_depth();
	float dive_factor = 0.0f;

	if ( depth >= 0.0f && depth < 10.0f )
	{
		dive_factor = 0.1f * ( 10.0f - depth ) * 
			ship::surface_visibility(watcher);
	}

	// Some modifiers when submarine is submerged.
	if ( depth >= 10.0f && depth <= periscope_depth )
	{
		double diverse_modifiers = 0.0f;

		// Periscope.
		if ( is_scope_up () )
		{
			// The visibility of the periscope also depends on the speed its moves
			// through the water. A fast moving periscope with water splashed is
			// much farther visible than a still standing one.
			diverse_modifiers += 0.1;//CROSS_SECTION_VIS_PERIS;
		}

		if ( is_snorkel_up () )
		{
			// A snorkel is much larger than a periscope.
			diverse_modifiers += 3.0f * 0.1;//CROSS_SECTION_VIS_PERIS;
		}

		dive_factor += diverse_modifiers * ( 0.5f + 0.5f * speed / max_speed );
	}

	return dive_factor;
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

	diveFactor *= 1.0/700.0 * get_cross_section ( watcher );

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

// mostly the same code as launch_torpedo. cut & paste is ugly, but sometimes unavoidable.
bool submarine::can_torpedo_be_launched(class game& gm, int tubenr, sea_object* target) const
{
	if (target == 0) return false;	// maybe assert this?
	if (target == this) return false;  // maybe assert this?
	
	bool usebowtubes = false;

	// if tubenr is < 0, choose a tube
	if (tubenr < 0) {	// check if target is behind
		angle a = angle(target->get_pos().xy() - get_pos().xy())
			- get_heading() + trp_addleadangle;
		usebowtubes = (a.ui_abs_value180() <= 90.0);
		// search for a filled tube
		pair<unsigned, unsigned> idx = usebowtubes ? get_bow_tube_indices() : get_stern_tube_indices();
		for (unsigned i = idx.first; i < idx.second; ++i) {
			if (torpedoes[i].status == stored_torpedo::st_loaded) {
				tubenr = int(i);
				break;
			}
		}
		if (tubenr < 0) return false;	// no torpedo found
	} else {	// check if tube nr is bow or stern
		unsigned tn = unsigned(tubenr);
		pair<unsigned, unsigned> idx = get_bow_tube_indices();
		if (tn >= idx.first && tn < idx.second) {
			usebowtubes = true;
		} else {
			idx = get_stern_tube_indices();
			if (tn < idx.first || tn >= idx.second)
				return false;	// illegal tube nr.
		}
	}

	// check if torpedo can be fired with that tube, if yes, then fire it
	pair<angle, bool> launchdata = torpedo::compute_launch_data(
		torpedoes[tubenr].type, this, target, usebowtubes, trp_addleadangle);
	return launchdata.second;
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
		// When a submarine uses its snorkel its maximum diesel speed is
		// reduced by 50%. This reduces the noise level returned by method
		// sea_object::get_noise_factor and must be corrected here by
		// multiply the actual noise factor with 2.
		if ( has_snorkel() && is_submerged () && snorkelup )
			noisefac *= 2.0f;
	}

	return noisefac;
}

#include <sstream>
#include "system.h"
#include "menu.h"
void submarine::depth_charge_explosion(const class depth_charge& dc)
{
	// fixme: this should'nt be linear but exponential, e^(-fac*depth) or so...
	double damage_radius = DAMAGE_DC_RADIUS_SURFACE +
		get_pos().z * DAMAGE_DC_RADIUS_200M / 200;
	double deadly_radius = DEADLY_DC_RADIUS_SURFACE +
		get_pos().z * DEADLY_DC_RADIUS_200M / 200;
	vector3 relpos = get_pos() - dc.get_pos();
	
	// this factor is used later in damage strength calculation.
	// strength is >= 1.0 at deadly radius or nearer.
	// strength is <= 0.01 at damage radius or farther.
	double expfac = 4.605170186 / (damage_radius - deadly_radius);	// -4.6 = ln(0.01)

	// fixme: check if relative position is translated correctly to sub pos, e.g.
	// z coordinates seemed to be reversed, a dc exploding below the sub damages the
	// aa gun more than the hull (2003/07/31)
	
	// depth differences change destructive power
	// but only when dc explodes above... fixme
	// explosion and shock wave strength depends on water depth... fixme
	// damage/deadly radii depend on this power and on sub type. fixme
		
		ostringstream dcd;
		dcd << "depth charge explosion distance to sub: " << relpos.length() << "m.";
		system::sys().add_console(dcd.str());
	vector3 sdist(relpos.x, relpos.y, relpos.z /* *2.0 */);
	double sdlen = sdist.length();

	// is submarine killed immidiatly?
	if (sdlen <= deadly_radius) {
			system::sys().add_console("depth charge hit!");
		kill();
			menu m(103, killedimg);	// move this to game! fixme
			m.add_item(105, 0);
			m.run();
			// ui->add_message(TXT_Depthchargehit[language]);

	} else if (sdlen <= damage_radius) {	// handle damages
		// this is useless. strength must be calculated indivually for each part. fixme
		double strength = (damage_radius - sdlen) / (damage_radius - deadly_radius);

		// add damage
		vector3f bb = modelcache.find(get_modelname())->get_boundbox_size();
	
		// project relative position to circle on 2d plane (y,z) parallel to sub.
		// circle's radius is proportional to strength.
		// all parts within this circle are affected to damage relative to their distance to the
		// circles center.
	
		for (unsigned i = 0; i < nr_of_damageable_parts; ++i) {
			if (damageable_parts[i].status < 0) continue;	// avoid non existent parts.

			vector3f tmp = (damage_schemes[i].p1 + damage_schemes[i].p2) * 0.5;
				if (tmp.square_length() == 0) continue;//hack to avoid yet not exsisting data fixme
			vector3 part_center = get_pos() + vector3(
				(tmp.x - 0.5) * bb.x,
				(tmp.y - 0.5) * bb.y,
				(tmp.z - 0.5) * bb.z );
			vector3 relpos = part_center - dc.get_pos();
			// depth differences change destructive power etc. see above
			double sdlen = relpos.length();
			
//			double strength = (damage_radius - sdlen) / (damage_radius - deadly_radius);
			double strength = exp((deadly_radius - sdlen) * expfac);
			
			if (strength > 0 && strength <= 1) {
				add_saturated(damageable_parts[i].status, strength, 1.0);
			}
	
				ostringstream os;
				os << "DC caused damage! relpos " << relpos.x << "," << relpos.y << "," << relpos.z << " dmg " << strength;
				system::sys().add_console(os.str());
		}
	}
}

void submarine::calculate_fuel_factor ( double delta_time )
{
	if ( electric_engine )
	{
		if ( battery_level >= 0.0f && battery_level <= 1.0f )
			battery_level -= delta_time * get_battery_consumption_rate ();
	}
	else
	{
		ship::calculate_fuel_factor ( delta_time );

		// Recharge battery.
		if ( battery_level >= 0.0f && battery_level <= 1.0f )
			battery_level += delta_time * get_battery_recharge_rate ();
	}
}

bool submarine::set_snorkel_up ( bool snorkelup )
{
	// Snorkel can be toggled only when it is available 
	// and the submarine is at least at snorkel depth.
	if ( has_snorkel() && get_depth () <= snorkel_depth )
	{
		this->snorkelup = snorkelup;

		// Activate diesel or electric engines if snorkel is up or down.
		if ( snorkelup )
			electric_engine = false;
		else
			electric_engine = true;

		return true;
	}

	return false;
}

void submarine::launch_torpedo(class game& gm, int tubenr, sea_object* target)
{
	if (target == 0) return;	// maybe assert this?
	if (target == this) return;
	
	bool usebowtubes = false;

	// if tubenr is < 0, choose a tube
	if (tubenr < 0) {	// check if target is behind
		angle a = angle(target->get_pos().xy() - get_pos().xy())
			- get_heading() + trp_addleadangle;
		usebowtubes = (a.ui_abs_value180() <= 90.0);
		// search for a filled tube
		pair<unsigned, unsigned> idx = usebowtubes ? get_bow_tube_indices() : get_stern_tube_indices();
		for (unsigned i = idx.first; i < idx.second; ++i) {
			if (torpedoes[i].status == stored_torpedo::st_loaded) {
				tubenr = int(i);
				break;
			}
		}
		if (tubenr < 0) return;	// no torpedo found
	} else {	// check if tube nr is bow or stern
		unsigned tn = unsigned(tubenr);
		pair<unsigned, unsigned> idx = get_bow_tube_indices();
		if (tn >= idx.first && tn < idx.second) {
			usebowtubes = true;
		} else {
			idx = get_stern_tube_indices();
			if (tn < idx.first || tn >= idx.second)
				return;	// illegal tube nr.
		}
	}

	// check if torpedo can be fired with that tube, if yes, then fire it
	pair<angle, bool> launchdata = torpedo::compute_launch_data(
		torpedoes[tubenr].type, this, target, usebowtubes, trp_addleadangle);
	if (launchdata.second) {
		torpedo* t = new torpedo(this, torpedoes[tubenr].type, usebowtubes, launchdata.first,
			trp_primaryrange, trp_secondaryrange, trp_initialturn, trp_searchpattern);
		gm.spawn_torpedo(t);    // fixme add command
		torpedoes[tubenr].type = torpedo::none;
		torpedoes[tubenr].status = stored_torpedo::st_empty;
	}
}
