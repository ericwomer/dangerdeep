// submarines
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "submarine.h"
#include "model.h"
#include "game.h"
#include "tokencodes.h"
#include "sensors.h"
#include "date.h"
#include "torpedo.h"
#include "depth_charge.h"
#include "date.h"
#include "tinyxml/tinyxml.h"
#include "system.h"
#include "user_interface.h"
#include "texts.h"

#include <sstream> //for depthcharge testing

// fixme: this was hard work. most values are a rather rough approximation.
// if we want a real accurate simulation, these values have to be (re)checked.
// also they have to fit with "rect" values in sub_damage_display.cpp.

/*
make as xml in ship type description file
<damageable-parts>
<part id="rudder" x0="" y0="" z0="" x1="" y1="" z1="" weakness="0.2" repairtime="3600" surfaced="true" repairable="true"/>


parts can be inside the hull and thus damaged or be part of the hull and thus flooded.
some parts are exterior and are not flooded (screws, rudder).
the more are part is damaged the more it is flooded.
the game will read the ship description xml file and a part layout file (3ds).
it can determine the total volume of all parts from the 3ds file.
it must know the ships water draught and weight.
formula, if the ship swims: draught * 1 >= weight ?
draught is given by total volume of all parts and height of waterline.
when a part is damaged, it begins to flood with water, at most to the height of the waterline.
the higher the damage, the more water per second gets into it.
the ship gets heavier: ship weight = base weight + flooded_water in cubic meters * 1
if ship weight > draught then the ship sinks with some speed that depends on the difference
between weight and draught.
When it sinks, the draught increases, and later also the amount of flooded water.
So it can happen, that the ship will not sink totally, but just lies deeper in the water.
if the waterline is too height (<1m below deck?) it will sink (for ships!) because the water
will run through the bulkheads from top, or it capsizes.

So compute volume of all floodable parts.
Compute part of volume that is below water line.
this gives the standard weight of the ship.
compute difference D to given real weight.
now at any time: A = weight = real weight - D + amount of flooded water
B = draught = sum of all parts below waterline, depends on height (pos.z) of ship.
if A > B then ship sinks, i.e. set acceleration.z to -G
if B < A set acc.z to +G, else to 0.

some parts could be mostly above the waterline and not add to the draught, but may
sink below it when the ship gets heavier.

so a part has the following attributes:
size/pos
floodable
repairable
repair time (not time to pump it dry! instead time to fix leaks, if possible)
must be surfaced to repair (subs)
weakness (to explosions, multiplier for damage)
steps of damage (binary or finer steps)
variable data:
amount of damage
time until repairs finish
amount of flooding

a part leaks more if the damage is heavier. wrecked parts will flood fastest and can not
be repaired. the time needed to pump a part dry depends on its volume and the amount
of water the pumps can handle.

damage statii: none, light, medium, heavy, wrecked. (0,25,50,75,100%)
a part can have two or five statii (binary / variable).

we must know how many pumps a ship has and how much water they can pump outside per second.
submarine's hull can be damaged independent on amount of flooded water... difficult
also flooded parts can lead to capsizing, or the ship sinks because the bow or stern dives below
the waterline although the ship would swim in total.
*/

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



submarine::stored_torpedo::stored_torpedo()
	: torp(0), status(st_empty), associated(0), remaining_time(0), preheating(false)
{
}



submarine::stored_torpedo::stored_torpedo(game& gm, const std::string& type)
	: torp(0), status(st_loaded), associated(0), remaining_time(0), preheating(false)
{
	xml_doc doc(get_torpedo_dir() + type + ".xml");
	doc.load();
	torp = new torpedo(gm, doc.first_child());
}



void submarine::stored_torpedo::load(game& gm, const xml_elem& parent)
{
	status = st_status(parent.attru("status"));
	associated = parent.attru("associated");
	remaining_time = parent.attrf("remaining_time");
	addleadangle = angle(parent.attrf("addleadangle"));
	preheating = parent.attrb("preheating");
	if (parent.has_child("torpedo")) {
		xml_elem tp = parent.child("torpedo");
		xml_doc doc(get_torpedo_dir() + tp.attr("type") + ".xml");
		doc.load();
		delete torp;
		torp = 0;
		torp = new torpedo(gm, doc.first_child());
	}
}



void submarine::stored_torpedo::save(xml_elem& parent) const
{
	parent.set_attr(unsigned(status), "status");
	parent.set_attr(associated, "associated");
	parent.set_attr(remaining_time, "remaining_time");
	parent.set_attr(addleadangle.value(), "addleadangle");
	parent.set_attr(preheating, "preheating");
	if (torp) {
		xml_elem tp = parent.add_child("torpedo");
		tp.set_attr(torp->get_specfilename(), "type");
		torp->save(tp);
	}
}



submarine::submarine(game& gm_, const xml_elem& parent)
	: ship(gm_, parent),
	  dive_speed(0),
	  max_depth(0),
	  dive_to(0),
	  permanent_dive(false),
	  delayed_dive_to_depth(0),
	  delayed_planes_down(0),
	  bow_to(0),
	  stern_to(0),
	  bow_rudder(0),
	  stern_rudder(0),
	  scopeup(false),
	  electric_engine(false),
	  snorkelup(false),
	  battery_level(0)
{
	xml_elem sm = parent.child("motion").child("submerged");
	max_submerged_speed = sm.attrf("maxspeed");
	double safedepth = sm.attrf("safedepth");
	double maxdepth = sm.attrf("maxdepth");
	max_depth = safedepth + rnd() * (maxdepth - safedepth);
	xml_elem dp = parent.child("depths");
	periscope_depth = dp.attrf("scope");
	snorkel_depth = dp.attrf("snorkel");
	alarm_depth = dp.attrf("alarm");
	xml_elem tp = parent.child("torpedoes");
	xml_elem tb = tp.child("tubes");
	number_of_tubes_at[0] = tb.attru("bow");
	number_of_tubes_at[1] = tb.attru("stern");
	number_of_tubes_at[2] = tb.attru("bowreserve");
	number_of_tubes_at[3] = tb.attru("sternreserve");
	number_of_tubes_at[4] = tb.attru("bowdeckreserve");
	number_of_tubes_at[5] = tb.attru("sterndeckreserve");
	unsigned nrtrp = 0;
	for (unsigned i = 0; i < 6; ++i) nrtrp += number_of_tubes_at[i];
	torpedoes.resize(nrtrp);
	xml_elem tf = tp.child("transfertimes");
	torp_transfer_times[0] = tf.attru("bow");
	torp_transfer_times[1] = tf.attru("stern");
	torp_transfer_times[2] = tf.attru("bowdeck");
	torp_transfer_times[3] = tf.attru("sterndeck");
	torp_transfer_times[4] = tf.attru("bowsterndeck");
	xml_elem bt = parent.child("battery");
	battery_capacity = bt.attru("capacity");
	battery_value_a = bt.attrf("consumption_a");
	battery_value_t = bt.attrf("consumption_t");
	battery_recharge_value_a = bt.attrf("recharge_a");
	battery_recharge_value_t = bt.attrf("recharge_t");

	// set all common damageable parts to "no damage", fixme move to ship?, replace by damage editor data reading
	//damageable_parts.resize(nr_of_damageable_parts);
	//for (unsigned i = 0; i < unsigned(outer_stern_tubes); ++i)
	//damageable_parts[i] = damageable_part(0, 0);
}



submarine::~submarine()
{
	for (vector<stored_torpedo>::iterator it = torpedoes.begin(); it != torpedoes.end(); ++it)
		delete it->torp;
}



void submarine::load(const xml_elem& parent)
{
	ship::load(parent);
	xml_elem dv = parent.child("diving");
	dive_speed = dv.attrf("dive_speed");
	max_depth = dv.attrf("max_depth");
	dive_to = dv.attrf("dive_to");
	permanent_dive = dv.attrb("permanent_dive");
	delayed_dive_to_depth = dv.attru("delayed_dive_to_depth");
	delayed_planes_down = dv.attrf("delayed_planes_down");
	bow_to = dv.attri("bow_to");
	stern_to = dv.attri("stern_to");
	bow_rudder = dv.attrf("bow_rudder");
	stern_rudder = dv.attrf("stern_rudder");

	xml_elem tp = parent.child("stored_torpedoes");
	torpedoes.clear();
	torpedoes.reserve(tp.attru("nr"));
	for (xml_elem::iterator it = tp.iterate("stored_torpedo"); !it.end(); it.next()) {
		stored_torpedo stp;
		stp.load(gm, it.elem());
		torpedoes.push_back(stp);
	}

	xml_elem sst = parent.child("sub_state");
	scopeup = sst.attru("scopeup");
	electric_engine = sst.attru("electric_engine");
	snorkelup = sst.attru("snorkelup");
	battery_level = sst.attrf("battery_level");
    
	// fixme: later move to ship, or even sea_object!
	xml_elem dm = parent.child("damageable_parts");
	damageable_parts.clear();
	damageable_parts.reserve(dm.attru("nr"));
	for (xml_elem::iterator it = tp.iterate("part"); !it.end(); it.next()) {
		//damageable_parts.push_back(damageable_part(it.elem()));
	}

	TDC.load(parent);
}



void submarine::save(xml_elem& parent) const
{
	ship::save(parent);
	xml_elem dv = parent.add_child("diving");
	dv.set_attr(dive_speed, "dive_speed");
	dv.set_attr(max_depth, "max_depth");
	dv.set_attr(dive_to, "dive_to");
	dv.set_attr(permanent_dive, "permanent_dive");
	dv.set_attr(delayed_dive_to_depth, "delayed_dive_to_depth");
	dv.set_attr(delayed_planes_down, "delayed_planes_down");
	dv.set_attr(bow_to, "bow_to");
	dv.set_attr(stern_to, "stern_to");
	dv.set_attr(bow_rudder, "bow_rudder");
	dv.set_attr(stern_rudder, "stern_rudder");

	xml_elem tp = parent.add_child("stored_torpedoes");
	tp.set_attr(torpedoes.size(), "nr");
	for (vector<stored_torpedo>::const_iterator it = torpedoes.begin(); it != torpedoes.end(); ++it) {
		//save a stored_torpedo node for each entry
		xml_elem stp = tp.add_child("stored_torpedo");
		it->save(stp);
	}

	xml_elem sst = parent.add_child("sub_state");
	sst.set_attr(scopeup, "scopeup");
	sst.set_attr(electric_engine, "electric_engine");
	sst.set_attr(snorkelup, "snorkelup");
	sst.set_attr(battery_level, "battery_level");
    
	// fixme: later move to ship, or even sea_object!
	xml_elem dm = parent.add_child("damageable_parts");
	dm.set_attr(damageable_parts.size(), "nr");
	for (vector<damageable_part>::const_iterator it = damageable_parts.begin(); it != damageable_parts.end(); ++it) {
		//save a part node for each entry
		//it->save(out);
	}

	TDC.save(parent);
}



void submarine::transfer_torpedo(unsigned from, unsigned to)
{
	if (torpedoes[from].status == stored_torpedo::st_loaded &&
			torpedoes[to].status == stored_torpedo::st_empty) {
		torpedoes[to].torp = torpedoes[from].torp;
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
	pair<unsigned, unsigned> indices = (usebow) ? get_bow_reserve_indices() : get_stern_reserve_indices();
	int tubenr = -1;
	for (unsigned i = indices.first; i < indices.second; ++i) {
		if (torpedoes[i].status == stored_torpedo::st_loaded) {	// loaded
			tubenr = i; break;
		}
	}
	return tubenr;
}



void submarine::simulate(double delta_time)
{
	// get goal state for the front rudder (back rudder is negative front_rudder for now
	double d_rudder_to = static_cast<double>(bow_to)*30;
	if( bow_rudder > d_rudder_to )
		--bow_rudder;
	else if( bow_rudder < d_rudder_to )
		++bow_rudder;

	ship::simulate(delta_time);

	vector3 sub_velocity = get_velocity();

	// acceleration constant (acceleration should not be constant here though)
	dive_acceleration = 1.0;

	vector3 dive_accel(0,0,dive_acceleration*sub_velocity.y);
	dive_accel += sub_velocity;

	// front rudder comes as -90 : 90
	dive_speed = dive_accel.z *   sin( (bow_rudder*0.3333)*M_PI*0.005555 ) ;
	dive_speed *= delta_time;

	//if( get_throttle()<=reverse ) dive_speed = -dive_speed;


	// still use the same variable to prevent breaking the code
	double delta_depth = dive_speed;

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
				bow_to = rudder_center;
				permanent_dive=true;
			} else {
				position.z += delta_depth;
			}
		}
	}
	if (position.z > 0) {
		position.z = 0;
		dive_speed = 0;
		bow_to = rudder_center;
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

	// simulate the TDC
	TDC.update_heading(get_heading());
	TDC.simulate(delta_time);
	if (target) {
		// fixme: limit update of bearing to each 5-30 secs or so,
		// quality depends on duration of observance and quality of crew!
		TDC.set_bearing(angle(target->get_pos().xy() - get_pos().xy()));
	}
		
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
					st.torp = 0;
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



void submarine::set_target(sea_object* s)
{
	sea_object::set_target(s);
	if (!target) return;

	TDC.set_torpedo_data(kts2ms(30), 7500);	// fixme!!!!
	// the values below should be modified by quality of guessed target data
	// well trained crews guess better and/or faster
	TDC.set_target_speed(target->get_speed());
	TDC.set_target_distance(target->get_pos().xy().distance(get_pos().xy()));
	TDC.set_bearing(angle(target->get_pos().xy() - get_pos().xy()));
	TDC.set_target_course(target->get_heading());
	TDC.set_heading(get_heading());
	//TDC.set_additional_leadangle();
}



void submarine::init_fill_torpedo_tubes(const date& d)
{
	// get date from game, fixme
	// we ignore T2, T1FAT, T4 here, fixme

	// standard1, special1 2/3, standard2, special2 1/3, 1/2 standard, 1/2 special
	// hence random 0-5, 0,1:std1, 2:std2, 3,4:spc1, 5:spc2
	string standard1, standard2, special1, special2, stern, deck;
	if (d < date(1942, 6, 1)) {
		standard1 = "TI";
		standard2 = special1 = special2 = stern = "TIII";
		deck = "TI";
	} else if (d < date(1943, 8, 1)) {
		standard1 = stern = "TIII";
		standard2 = "TI";
		special1 = special2 = "TIIIa_FaTII";
		deck = "TI_FaTI";
	} else if (d < date(1945, 4, 1)) {
		standard1 = "TIII";
		standard2 = "TIIIa_FaTII";
		special1 = "TIIIa_LuTI";
		special2 = "TV";
		stern = "TV";
		deck = "TI_LuTI";
	} else {
		standard1 = "TIII";
		standard2 = "TIIIa_FaTII";
		special1 = "TVI_LuTI";
		special2 = "TXI";
		stern = "TXI";
		deck = "TI_LuTII";
	}
	
	pair<unsigned, unsigned> idx;
	idx = get_bow_tube_indices();
	for (unsigned i = idx.first; i < idx.second; ++i) {
		unsigned r = rnd(6);
		if (r <= 1) torpedoes[i] = stored_torpedo(gm, standard1);
		else if (r <= 2) torpedoes[i] = stored_torpedo(gm, standard2);
		else if (r <= 4) torpedoes[i] = stored_torpedo(gm, special1);
		else torpedoes[i] = stored_torpedo(gm, special2);
	}
	idx = get_stern_tube_indices();
	for (unsigned i = idx.first; i < idx.second; ++i) {
		torpedoes[i] = stored_torpedo(gm, stern);
	}
	idx = get_bow_reserve_indices();
	for (unsigned i = idx.first; i < idx.second; ++i) {
		unsigned r = rnd(6);
		if (r <= 1) torpedoes[i] = stored_torpedo(gm, standard1);
		else if (r <= 2) torpedoes[i] = stored_torpedo(gm, standard2);
		else if (r <= 4) torpedoes[i] = stored_torpedo(gm, special1);
		else torpedoes[i] = stored_torpedo(gm, special2);
	}
	idx = get_stern_reserve_indices();
	for (unsigned i = idx.first; i < idx.second; ++i) {
		unsigned r = rnd(2);
		if (r < 1) torpedoes[i] = stored_torpedo(gm, stern);
		else torpedoes[i] = stored_torpedo(gm, special2);
	}
	idx = get_bow_deckreserve_indices();
	for (unsigned i = idx.first; i < idx.second; ++i) {
		// later in the war no torpedoes were stored at deck, fixme
		torpedoes[i] = stored_torpedo(gm, deck);
	}
	idx = get_stern_deckreserve_indices();
	for (unsigned i = idx.first; i < idx.second; ++i) {
		// later in the war no torpedoes were stored at deck, fixme
		torpedoes[i] = stored_torpedo(gm, deck);
	}
}



// give number from 0-5 (bow tubes first)
bool submarine::is_tube_ready(unsigned nr) const
{
	if (nr > 5) return false;

	unsigned nrb = get_nr_of_bow_tubes();
	unsigned nrs = get_nr_of_stern_tubes();
	if (nr >= nrb + nrs) return false;

	return (torpedoes[nr].status == stored_torpedo::st_loaded);
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



pair<unsigned, unsigned> submarine::get_bow_reserve_indices(void) const
{
	unsigned off = get_nr_of_bow_tubes()+get_nr_of_stern_tubes();
	return make_pair(off, off+get_nr_of_bow_reserve());
}



pair<unsigned, unsigned> submarine::get_stern_reserve_indices(void) const
{
	unsigned off = get_nr_of_bow_tubes()+get_nr_of_stern_tubes()+get_nr_of_bow_reserve();
	return make_pair(off, off+get_nr_of_stern_reserve());
}



pair<unsigned, unsigned> submarine::get_bow_deckreserve_indices(void) const
{
	unsigned off = get_nr_of_bow_tubes()+get_nr_of_stern_tubes()+get_nr_of_bow_reserve()+get_nr_of_stern_reserve();
	return make_pair(off, off+get_nr_of_bow_deckreserve());
}



pair<unsigned, unsigned> submarine::get_stern_deckreserve_indices(void) const
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
	idx = get_bow_reserve_indices();
	if (tn >= idx.first && tn < idx.second) return 3;
	idx = get_stern_reserve_indices();
	if (tn >= idx.first && tn < idx.second) return 4;
	idx = get_bow_deckreserve_indices();
	if (tn >= idx.first && tn < idx.second) return 5;
	idx = get_stern_deckreserve_indices();
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
	// fixme: that model is too crude,
	// we compute cross sections with standard draught, so the hull is ~ 1m above
	// the water. In reality it is hidden in the waves when watched from a longer
	// distance (it doesn't need to be under water, just hidden by higher waves nearby).
	// So the only visible thing of a sub is the conning tower, making it less visible.
	
	// fixme: 2004/05/16, i removed the 1/750 factor from sea_object.cpp
	// the rest of the code has to be adapted

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

		double speed = get_speed();
		dive_factor += diverse_modifiers * ( 0.5f + 0.5f * speed / max_speed_forward );
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
	bow_to += int(amount);
	if( bow_to > rudder_up_30 )
		bow_to = rudder_up_30;

//	dive_acceleration = -1;
//	dive_speed = max_dive_speed;
	permanent_dive = true;
}



void submarine::planes_down(double amount)
{
	user_interface* ui = gm.get_ui();
	if (false == is_gun_manned())
	{
		bow_to -= int(amount);
		if( bow_to < rudder_down_30 )
			bow_to = rudder_down_30;

	//	dive_acceleration = 1;
	//	dive_speed = -max_dive_speed;
		permanent_dive = true;
	}
	else
	{
		ui->add_message(texts::get(753));
		delayed_planes_down = amount;
		delayed_dive_to_depth = 0;
		toggle_gun_manning();
		ui->add_message(texts::get(754));
	}
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
	user_interface* ui = gm.get_ui();
	if (false == is_gun_manned())
	{	
		dive_to = -int(meters);
		permanent_dive = false;
		bow_to = ( dive_to < position.z )? rudder_down_30 : rudder_up_30;
		//		dive_speed = (dive_to < position.z) ? -max_dive_speed : max_dive_speed;
	}
	else
	{	
		ui->add_message(texts::get(753));
		delayed_planes_down = 0.0;
		delayed_dive_to_depth = meters;
		toggle_gun_manning();
		ui->add_message(texts::get(754));
	}
}



#if 0
// mostly the same code as launch_torpedo. cut & paste is ugly, but sometimes unavoidable.
bool submarine::can_torpedo_be_launched(int tubenr, sea_object* target, 
					stored_torpedo::st_status &tube_status) const
{
	if (target == 0) return false;	// maybe assert this?
	if (target == this) return false;  // maybe assert this?
	
	bool usebowtubes = false;

	// if tubenr is < 0, choose a tube
	if (tubenr < 0) {	// check if target is behind
		angle a = angle(target->get_pos().xy() - get_pos().xy())
			- get_heading(); // + trp_addleadangle; // angle setting is per tube!
		usebowtubes = (a.ui_abs_value180() <= 90.0);
		// search for a filled tube
		pair<unsigned, unsigned> idx = usebowtubes ? get_bow_tube_indices() : get_stern_tube_indices();
		for (unsigned i = idx.first; i < idx.second; ++i) {
			if (torpedoes[i].status == stored_torpedo::st_loaded) {
				tubenr = int(i);
				break;
			}
		}
		if (tubenr < 0) {
			tube_status = stored_torpedo::st_empty;
			return false;	// no torpedo found
		}
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
		
		if (torpedoes[tubenr].status != stored_torpedo::st_loaded) {
			tube_status = torpedoes[tubenr].status;
			return false;
		}
	}

	// check if torpedo can be fired with that tube, if yes, then fire it
	pair<angle, bool> launchdata = torpedo::compute_launch_data(
		torpedoes[tubenr].type, this, target, usebowtubes,
		tubesettings[tubenr].addleadangle);	
	
	tube_status = torpedoes[tubenr].status;
	return launchdata.second;
}
#endif



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



submarine::stored_torpedo& submarine::get_torp_in_tube(unsigned tubenr)
{
	if (tubenr < number_of_tubes_at[0] + number_of_tubes_at[1]) {
		return torpedoes[tubenr];
	}
	throw error("illegal tube nr for get_torp_in_tube");
}



const submarine::stored_torpedo& submarine::get_torp_in_tube(unsigned tubenr) const
{
	if (tubenr < number_of_tubes_at[0] + number_of_tubes_at[1]) {
		return torpedoes[tubenr];
	}
	throw error("illegal tube nr for get_torp_in_tube");
}



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
		sys().add_console(dcd.str());
	vector3 sdist(relpos.x, relpos.y, relpos.z /* *2.0 */);
	double sdlen = sdist.length();

	// is submarine killed immidiatly?
	if (sdlen <= deadly_radius) {
		sys().add_console("depth charge hit!");
		kill();
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
				sys().add_console(os.str());
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



void submarine::launch_torpedo(int tubenr, sea_object* target)
{
	if (target == 0) return;	// maybe assert this?
	if (target == this) return;
	
	bool usebowtubes = false;

	// if tubenr is < 0, choose a tube
	if (tubenr < 0) {	// check if target is behind
		angle a = angle(target->get_pos().xy() - get_pos().xy())
			- get_heading(); // + trp_addleadangle;
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
	cout << "sol valid? " << TDC.solution_valid() << "\n";
	//fixme
	if (true/*TDC.solution_valid()*/) {
		angle fired_at_angle = usebowtubes ? heading : heading + angle(180);
		angle torp_head_to = TDC.get_lead_angle() + TDC.get_additional_leadangle();
		cout << "firead at " << fired_at_angle.value() << "," << torp_head_to.value() << "," << torp_head_to.is_cw_nearer(fired_at_angle) << "\n";
		torpedoes[tubenr].torp->head_to_ang(torp_head_to, torp_head_to.is_cw_nearer(fired_at_angle));
		// just hand the torpedo object over to class game. tube is empty after that...
		vector3 torppos = position + (fired_at_angle.direction() * (get_length()/2 + 5 /*5m extra*/)).xy0();
		torpedoes[tubenr].torp->launch(torppos, fired_at_angle);
		gm.spawn_torpedo(torpedoes[tubenr].torp);
		torpedoes[tubenr].torp = 0;
		torpedoes[tubenr].status = stored_torpedo::st_empty;
	}
}



void submarine::gun_manning_changed(bool isGunManned)
{
	user_interface* ui = gm.get_ui();
	if (true == isGunManned)
		ui->add_message(texts::get(755));
	else
		ui->add_message(texts::get(756));		

	if (0.0 != delayed_planes_down) {
		planes_down(delayed_planes_down);
		delayed_planes_down = 0.0;
		ui->add_message(texts::get(757));		
	} else if (0 != delayed_dive_to_depth) {
		dive_to_depth(delayed_dive_to_depth);
		delayed_dive_to_depth = 0;
		ui->add_message(texts::get(757));		
	}
}



void submarine::set_throttle(ship::throttle_status thr)
{	
	if (get_throttle() != thr)
	{
		stop_throttle_sound();
		ship::set_throttle(thr);	
		start_throttle_sound();
	}
}



void submarine::start_throttle_sound()
{
	user_interface* ui = gm.get_ui();
	switch(get_throttle()) {
	case ship::aheadslow:
		ui->play_fade_sound_effect(se_sub_screws_slow, this, this, true); 	
		break;
	case ship::aheadhalf:
		ui->play_fade_sound_effect(se_sub_screws_normal, this, this, true); 	
		break;
	case ship::aheadfull:
		ui->play_fade_sound_effect(se_sub_screws_fast, this, this, true); 	
		break;
	case ship::aheadflank:
		ui->play_fade_sound_effect(se_sub_screws_very_fast, this, this, true); 	
		break;
	case ship::stop:
	case ship::reverse:
	case ship::aheadlisten:
	case ship::aheadsonar:
		break;
	}
}



void submarine::stop_throttle_sound()
{
	user_interface* ui = gm.get_ui();
	switch(get_throttle()) {
	case ship::aheadslow:
		ui->stop_fade_sound_effect(se_sub_screws_slow);
		break;
	case ship::aheadhalf:
		ui->stop_fade_sound_effect(se_sub_screws_normal);
		break;
	case ship::aheadfull:
		ui->stop_fade_sound_effect(se_sub_screws_fast);
		break;
	case ship::aheadflank:
		ui->stop_fade_sound_effect(se_sub_screws_very_fast);
		break;
	case ship::stop:
	case ship::reverse:
	case ship::aheadlisten:
	case ship::aheadsonar:
		break;
	}	
}
