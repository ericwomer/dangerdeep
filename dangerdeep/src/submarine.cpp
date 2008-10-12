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

// submarines
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "submarine.h"
#include "model.h"
#include "game.h"
#include "sensors.h"
#include "date.h"
#include "torpedo.h"
#include "depth_charge.h"
#include "date.h"
#include "system.h"
#include "user_interface.h"
#include "texts.h"
#include "global_data.h"
#include "global_constants.h"
#include "log.h"

#include <sstream> //for depthcharge testing

using std::vector;
using std::string;
using std::pair;
using std::make_pair;
using std::ostringstream;

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


submarine::stored_torpedo::stored_torpedo()
	: torp(0), status(st_empty), associated(0), remaining_time(0), preheating(false)
{
}



submarine::stored_torpedo::stored_torpedo(game& gm, const std::string& type)
	: torp(0), status(st_loaded), associated(0), remaining_time(0), preheating(false)
{
	xml_doc doc(data_file().get_filename(type));
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
		xml_doc doc(data_file().get_filename(tp.attr("type")));
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



submarine::tank::tank(xml_elem e)
	: type(ballast),
	  volume(e.attrf("volume")),
	  fillrate(e.attrf("fillrate")),
	  pos(e.attrf("posx"), e.attrf("posy"), e.attrf("posz")),
	  fill(0),
	  flood_valve_open(false)
{
	if (e.attr("type") == "trim")
		type = trim;
	else if (e.attr("type") == "ballast")
		type = ballast;
	else throw error(std::string("invalid tank type in file ") + e.doc_name());
}



void submarine::tank::simulate(double delta_time)
{
	if (flood_valve_open) {
		fill = std::min(fill + fillrate * delta_time, volume);
		//log_debug("tank fill " << fill);
	}
}



void submarine::tank::set_flood_valve(bool flood)
{
	// handle damage somewhen later...
	flood_valve_open = flood;
}



double submarine::tank::push_air_inside(double amount_cbm)
{
	double used = std::min(fill, amount_cbm);
	fill -= used;
	return amount_cbm - used;
}



void submarine::tank::load(const xml_elem& parent)
{
	fill = parent.attrf("fill");
	flood_valve_open = parent.attrb("flood_valve_open");
}



void submarine::tank::save(xml_elem& parent) const
{
	parent.set_attr(fill, "fill");
	parent.set_attr(flood_valve_open, "flood_valve_open");
}



submarine::submarine(game& gm_, const xml_elem& parent)
	: ship(gm_, parent),
	  max_depth(0),
	  dive_to(0),
	  permanent_dive(false),
	  dive_state(dive_state_surfaced),
	  bow_depth_rudder(vector3(0,30,0 /*not used yet*/), 1, 30, 4/*area*/, 10),//read consts from spec file, fixme
	  stern_depth_rudder(vector3(0,-30,0 /*not used yet*/), 1, 30, 4/*area*/, 10),//read consts from spec file, fixme
	  mass_flooded_tanks(0),
	  ballast_tank_capacity(0), // read from spec file later
	  scope_raise_level(0.0f),
	  scope_raise_to_level(0.0f),
	  electric_engine(false),
	  hassnorkel(false),
	  snorkel_depth(8.0),
	  alarm_depth(150.0),
	  snorkelup(false),
	  battery_level(0)
{
	xml_elem sm = parent.child("motion").child("submerged");
	max_submerged_speed = kts2ms(sm.attrf("maxspeed"));
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
	if (parent.has_child("torpedomanage")) { // fixme: later all subs should have it!!
	xml_elem tm = parent.child("torpedomanage");
	torpedomanage_sidetopimg = tm.attr("image");
	}

	// set all common damageable parts to "no damage", fixme move to ship?, replace by damage editor data reading
	//parts.resize(nr_of_parts);
	//for (unsigned i = 0; i < unsigned(outer_stern_tubes); ++i)
	//parts[i] = part(0, 0);

	// set hearing device
	date dt = gm.get_date();
	if (dt < date(1941, 6, 1))
		hearing_device = hearing_device_KDB;
	else if (dt < date(1944, 11, 1))
		hearing_device = hearing_device_GHG;
	else
		hearing_device = hearing_device_BG;

	// load the bridge data
	xml_elem br = parent.child("bridge");
	bridge_model_name = data_file().get_rel_path(specfilename) + br.attr("model");

	bridge_camera_pos = br.child("camera").attrv3();
	bridge_uzo_pos = br.child("uzo").attrv3();
	bridge_freeview_pos = br.child("freeview").attrv3();

	if (parent.has_child("tanks")) { // fixme: later all subs should have it!!
		xml_elem etanks = parent.child("tanks");
		for (xml_elem::iterator it = etanks.iterate("tank"); !it.end(); it.next()) {
			tanks.push_back(tank(it.elem()));
			ballast_tank_capacity += tanks.back().get_volume();
		}
	}

	diveplane_1_id = mymodel->get_object_id_by_name("diveplane_1");
	diveplane_2_id = mymodel->get_object_id_by_name("diveplane_2");
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
	max_depth = dv.attrf("max_depth");
	dive_to = dv.attrf("dive_to");
	permanent_dive = dv.attrb("permanent_dive");
	dive_state = dive_states(dv.attru("dive_state"));
	bow_depth_rudder.load(dv.child("bow_depth_rudder"));
	stern_depth_rudder.load(dv.child("stern_depth_rudder"));

	xml_elem tp = parent.child("stored_torpedoes");
	torpedoes.clear();
	torpedoes.reserve(tp.attru("nr"));
	for (xml_elem::iterator it = tp.iterate("stored_torpedo"); !it.end(); it.next()) {
		stored_torpedo stp;
		stp.load(gm, it.elem());
		torpedoes.push_back(stp);
	}

	xml_elem sst = parent.child("sub_state");
	scope_raise_level = scope_raise_to_level = sst.attrf("scopeup");
	electric_engine = sst.attru("electric_engine");
	snorkelup = sst.attru("snorkelup");
	battery_level = sst.attrf("battery_level");
    
	// fixme: later move to ship, or even sea_object!
#if 0 // disabled to avoid abort when loading savegames
	xml_elem dm = parent.child("parts");
	parts.clear();
	parts.reserve(dm.attru("nr"));
	for (xml_elem::iterator it = tp.iterate("part"); !it.end(); it.next()) {
		//parts.push_back(part(it.elem()));
	}
#endif

	xml_elem tk = parent.child("tanks");
	for (xml_elem::iterator it = tp.iterate("tank"); !it.end(); it.next()) {
		xml_elem e = it.elem();
		unsigned id = e.attru("nr");
		if (id >= tanks.size()) throw error(std::string("invalid tank nr in ") + e.doc_name());
		tanks[id].load(e);
	}

	TDC.load(parent);
	sonarman.load(parent);
}



void submarine::save(xml_elem& parent) const
{
	ship::save(parent);
	xml_elem dv = parent.add_child("diving");
	dv.set_attr(max_depth, "max_depth");
	dv.set_attr(dive_to, "dive_to");
	dv.set_attr(permanent_dive, "permanent_dive");
	dv.set_attr(dive_state, "dive_state");
	xml_elem ebdr = dv.add_child("bow_depth_rudder");
	bow_depth_rudder.save(ebdr);
	xml_elem esdr = dv.add_child("stern_depth_rudder");
	stern_depth_rudder.save(esdr);

	xml_elem tp = parent.add_child("stored_torpedoes");
	tp.set_attr(unsigned(torpedoes.size()), "nr");
	for (vector<stored_torpedo>::const_iterator it = torpedoes.begin(); it != torpedoes.end(); ++it) {
		//save a stored_torpedo node for each entry
		xml_elem stp = tp.add_child("stored_torpedo");
		it->save(stp);
	}

	xml_elem sst = parent.add_child("sub_state");
	sst.set_attr(scope_raise_level, "scopeup");
	sst.set_attr(electric_engine, "electric_engine");
	sst.set_attr(snorkelup, "snorkelup");
	sst.set_attr(battery_level, "battery_level");
    
	// fixme: later move to ship, or even sea_object!
	//xml_elem dm = parent.add_child("parts");
	//dm.set_attr(unsigned(parts.size()), "nr");
	//for (vector<sea_object::part>::const_iterator it = parts.begin(); it != parts.end(); ++it) {
		//save a part node for each entry
		//it->save(out);
	//}

	xml_elem tk = parent.add_child("tanks");
	for (unsigned i = 0; i < tanks.size(); ++i) {
		xml_elem e = tk.add_child("tank");
		e.set_attr(i, "nr");
		tanks[i].save(e);
	}

	TDC.save(parent);
	sonarman.save(parent);
}



void submarine::transfer_torpedo(unsigned from, unsigned to)
{
	// fixme: it once crashed here... check from/to for limits?
	// not a fix, only ugly workaround!!!
	if (from >= torpedoes.size()) {
		log_warning("from="<<from<<" max="<<torpedoes.size());
		throw error("BUG! transfer_torpedo, from invalid");
	}
	if (to >= torpedoes.size()) {
		log_warning("to="<<from<<" max="<<torpedoes.size());
		throw error("BUG! transfer_torpedo, to invalid");
	}
	if (torpedoes[from].status == stored_torpedo::st_loaded &&
			torpedoes[to].status == stored_torpedo::st_empty) {
		if (torpedoes[to].torp != 0)
			throw error("destination tube not empty, internal error");
		torpedoes[to].torp = torpedoes[from].torp;
		torpedoes[from].torp = 0;
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
	// diveplane animation
	if (diveplane_1_id >= 0) mymodel->set_object_angle(diveplane_1_id, bow_depth_rudder.angle);
	if (diveplane_2_id >= 0) mymodel->set_object_angle(diveplane_2_id, stern_depth_rudder.angle);

	// simulate all tanks (flooding) and recompute mass_flooded_tanks here
	mass_flooded_tanks = 0;
	for (std::vector<tank>::iterator it = tanks.begin(); it != tanks.end(); ++it) {
		it->simulate(delta_time);
		mass_flooded_tanks += it->get_fill() * 1000.0 /* water density */;
	}

	// now simulate the submarine as usual, but modify mass to handle
	// the extra mass of the ballast (and trim) tanks.
	// Do not manipulate linear_momentum or angular momentum here,
	// which means velocity is lower when tanks have more mass,
	// because velocity = linear_momentum / mass and mass is greater
	// Thus flooding tanks will make sub slower, which is in fact realistic.
	double mass_orig = mass;
	mass += mass_flooded_tanks;
	mass_inv = 1.0/mass;
	ship::simulate(delta_time);
	mass = mass_orig;
	mass_inv = 1.0/mass;

	if (!permanent_dive) {
		depth_steering_logic();
	}
	
	ballast_tank_control_logic(delta_time);

	bow_depth_rudder.simulate(delta_time);
	stern_depth_rudder.simulate(delta_time);

	if (-position.z > max_depth)
		kill();

	// ------------- simulate state changes ----------------------------------
	//log_debug("dive_state="<<unsigned(dive_state));
	switch (dive_state) {
	case dive_state_surfaced:
		break;
	case dive_state_preparing_for_dive:
	case dive_state_preparing_for_crashdive:
		if (electric_engine)
			electric_engine = false;
		if (!is_gun_manned()) {
			if (dive_state == dive_state_preparing_for_dive)
				dive_state = dive_state_diving;
			else
				dive_state = dive_state_crashdive;
		}
		break;
	case dive_state_diving:
		if (dive_to > -1.0 && position.z > -2.0) {
			electric_engine = false;
			dive_state = dive_state_surfaced;
		}
		break;
	case dive_state_crashdive:
		if (position.z < -alarm_depth * 0.8) {
			dive_state = dive_state_diving;
		}
		break;
	default:
		break;
	}

	// ------------- simulate periscope movement -----------------------------
	const double scope_move_speed = 2.0/6.0; // m/sec / total raise height
	if (scope_raise_level < scope_raise_to_level) {
		scope_raise_level = std::min(scope_raise_to_level, scope_raise_level + float(delta_time * scope_move_speed));
	} else if (scope_raise_level > scope_raise_to_level) {
		scope_raise_level = std::max(scope_raise_to_level, scope_raise_level - float(delta_time * scope_move_speed));
	}

	// ------------- simulate the TDC ------------------------------------------
	TDC.update_heading(get_heading());
	TDC.simulate(delta_time);
	if (target) {
		// fixme: limit update of bearing to each 5-30 secs or so,
		// quality depends on duration of observance and quality of crew!
		if (TDC.auto_mode_enabled()) {
			TDC.set_bearing(angle(target->get_pos().xy() - get_pos().xy()));
		}
	}
		
	// ------------- torpedo transfer -----------------------------------
	for (unsigned i = 0; i < torpedoes.size(); ++i) {
		stored_torpedo& st = torpedoes[i];
		if (st.status == stored_torpedo::st_reloading ||
			st.status == stored_torpedo::st_unloading) { // reloading/unloading
			st.remaining_time -= delta_time;
			if (st.remaining_time <= 0) {
				if (st.status == stored_torpedo::st_reloading) {	// reloading
					st.status = stored_torpedo::st_loaded;	// loading
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

	// simulate the sonar man
	sonarman.simulate(gm, delta_time);

	// hack for test of hearing devices, change by date
	date dt = gm.get_date();
	if (dt < date(1941, 6, 1))
		hearing_device = hearing_device_KDB;
	else if (dt < date(1944, 11, 1))
		hearing_device = hearing_device_GHG;
	else
		hearing_device = hearing_device_BG;
}



void submarine::set_target(sea_object* s)
{
	sea_object::set_target(s);
	if (!target) return;

	TDC.set_torpedo_data(kts2ms(30), 7500);	// fixme!!!!
	// the values below should be modified by quality of guessed target data
	// well trained crews guess better and/or faster
	if (TDC.auto_mode_enabled()) {
		TDC.set_target_speed(target->get_speed());
		TDC.set_target_distance(target->get_pos().xy().distance(get_pos().xy()));
		TDC.set_bearing(angle(target->get_pos().xy() - get_pos().xy()));
		TDC.set_target_course(target->get_heading());
	}
	// this value is always fetched automatically to the TDC
	TDC.set_heading(get_heading());
}



void submarine::init_fill_torpedo_tubes(const date& d)
{
	// get date from game, fixme
	// we ignore T2, T1FAT, T4 here, fixme

	// standard1, special1 2/3, standard2, special2 1/3, 1/2 standard, 1/2 special
	// hence random 0-5, 0,1:std1, 2:std2, 3,4:spc1, 5:spc2
	string standard1, standard2, special1, special2, stern, deck;
	// the following if's and definitions are coded after the data from the torpedo xml's.
	// we could also read all xml's, parse them, and create torpedoes accordingly,
	// but it was faster to do it hardcoded, and that data is historic and thus does not
	// change.
	if (d < date(1940, 6, 1)) {
		// available until that date: TI/TII/TIII
		standard1 = "TI";
		standard2 = stern = "TIII";
		special1 = "TII";
		special2 = "TII";
		deck = "TI";
	} else if (d < date(1942, 12, 1)) {
		// available until that date: TI/TII/TIII, only use TI/TIII here
		standard1 = "TIII";
		standard2 = stern = "TIII";
		special1 = "TI";
		special2 = "TIII";
		deck = "TI";
	} else if (d < date(1943, 3, 1)) {
		// available until that date: TI_FaTI, TIII_FaTII, TIIIa_FaTII
		standard1 = stern = "TIII";
		standard2 = "TI";
		special1 = "TIIIa_FaTII";
		special2 = "TIII_FaTII";
		deck = "TI_FaTI";
	} else if (d < date(1943, 9, 1)) {
		// available until that date: TIV
		standard1 = stern = "TIII";
		standard2 = "TI";
		special1 = "TIIIa_FaTII";
		special2 = "TIV";
		deck = "TI_FaTI";
	} else if (d < date(1944, 1, 1)) {
		// available until that date: TV
		standard1 = "TIII";
		standard2 = "TIIIa_FaTII";
		special1 = "TI_FaTI";
		special2 = "TV";
		stern = "TV";
		deck = "TI_FaTI";
	} else if (d < date(1944, 6, 1)) {
		// available until that date: TVb
		standard1 = "TIII";
		standard2 = "TIIIa_FaTII";
		special1 = "TIIIa_LuTI";
		special2 = "TVb";
		stern = "TV";
		deck = "TI_LuTI";
	} else if (d < date(1945, 4, 1)) {
		// available until that date: TIIIa_LuTI, TIIIa_LuTII, TI_LuTI, TI_LuTII, TVI_LuTI
		standard1 = "TIII";
		standard2 = "TIIIa_FaTII";
		special1 = "TIIIa_LuTI";
		special2 = "TV";
		stern = "TV";
		deck = "TI_LuTI";
	} else {
		// available since april 1945: TXI
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



pair<unsigned, unsigned> submarine::get_bow_tube_indices() const
{
	unsigned off = 0;
	return make_pair(off, off+get_nr_of_bow_tubes());
}



pair<unsigned, unsigned> submarine::get_stern_tube_indices() const
{
	unsigned off = get_nr_of_bow_tubes();
	return make_pair(off, off+get_nr_of_stern_tubes());
}



pair<unsigned, unsigned> submarine::get_bow_reserve_indices() const
{
	unsigned off = get_nr_of_bow_tubes()+get_nr_of_stern_tubes();
	return make_pair(off, off+get_nr_of_bow_reserve());
}



pair<unsigned, unsigned> submarine::get_stern_reserve_indices() const
{
	unsigned off = get_nr_of_bow_tubes()+get_nr_of_stern_tubes()+get_nr_of_bow_reserve();
	return make_pair(off, off+get_nr_of_stern_reserve());
}



pair<unsigned, unsigned> submarine::get_bow_deckreserve_indices() const
{
	unsigned off = get_nr_of_bow_tubes()+get_nr_of_stern_tubes()+get_nr_of_bow_reserve()+get_nr_of_stern_reserve();
	return make_pair(off, off+get_nr_of_bow_deckreserve());
}



pair<unsigned, unsigned> submarine::get_stern_deckreserve_indices() const
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



double submarine::get_max_speed() const
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
	else if ( (depth > 2.0f ) && ( depth < 10.0f ) )
	{
		// Submarine becomes visible for active sonar system while
		// diving process.
		diveFactor = 0.125f * (depth - 2.0f);
	}

	diveFactor *= 1.0/700.0 * get_cross_section ( watcher );

	return diveFactor;
}



void submarine::scope_to_level(float f)
{
	scope_raise_to_level = myclamp(f, 0.0f, 1.0f);
}



void submarine::set_planes_to(double amount)
{
	amount = myclamp(amount, -1.0, 1.0);
	if (amount < 0) {
		if (dive_state == dive_state_surfaced) {
			gm.add_event(new event_preparing_to_dive());
			if (has_deck_gun() && is_gun_manned()) {
				unman_guns(); // rather to state simulation?! fixme
				gm.add_event(new event_unmanning_gun());
			}
			// don't set planes yet
			return;
		}
	} else if (fabs(amount) < 0.001) {
		// planes at midships, stop depth change
		permanent_dive = false;
		dive_to = position.z;
	}		
	
	if (dive_state == dive_state_crashdive) {
		dive_state = dive_state_diving;
	}

	bow_depth_rudder.set_to(amount);
	stern_depth_rudder.set_to(amount);
	permanent_dive = true;
}



void submarine::dive_to_depth(unsigned meters)
{
	// fixme misplaced here...
	if (dive_state == dive_state_surfaced) {
		if (has_deck_gun() && is_gun_manned()) {
			gm.add_event(new event_preparing_to_dive());
			unman_guns();
			gm.add_event(new event_unmanning_gun());
		}
		dive_state = dive_state_preparing_for_dive;
	} else if (dive_state == dive_state_crashdive) {
		dive_state = dive_state_diving;
	}
	dive_to = -int(meters);
	permanent_dive = false;
}



void submarine::crash_dive()
{
	// fixme misplaced here...
	if (dive_state == dive_state_surfaced) {
		if (has_deck_gun() && is_gun_manned()) {
			gm.add_event(new event_preparing_to_dive());
			unman_guns();
			gm.add_event(new event_unmanning_gun());
		}
		dive_state = dive_state_preparing_for_crashdive;
	}
	dive_to = -alarm_depth;
	permanent_dive = false;
}



//#include "global_data.h"
// later useful for torpedoes too...
void submarine::depth_steering_logic()
{
	// --------------- dynamic depth control (rudders) ------------------------------------------

	/* New helmsman simulation.
	   We have the formula
	   error = a * x + b * y + c * z
	   where x = depth difference between dive_to and position.z
	         y = vertical velocity (with sign)
		 z = dive_rudder_pos
	   and a, b, c are some control factors (constants).
	   c should be much smaller than a and b, normally a > b > c.
	   the error has a sign, according to sign and magnitude of it
	   the rudder_to is set.
	   This system should find the correct course, it only needs
	   tuning of a, b, c. Their values depend on maximum turn speed.
	   The following (experimentally gained) formulas give good results.
	*/
	double target_depth = dive_to;
	switch (dive_state) {
	case dive_state_surfaced:
	case dive_state_preparing_for_dive:
		target_depth = 0;
		break;
	default:
		break;
	}
	double depthdiff = position.z - target_depth;
	double error0 = depthdiff;
	double error1 = (bow_depth_rudder.max_angle/bow_depth_rudder.max_turn_speed +
			 stern_depth_rudder.max_angle/stern_depth_rudder.max_turn_speed) * local_velocity.z * 1.0;
	double error2 = 0;//-rudder_pos/max_rudder_turn_speed * turn_velocity;
	double error = error0 + error1 + error2;
	//DBGOUT7(anglediff, turn_velocity, rudder_pos, error0, error1, error2, error);
	double rd = myclamp(error, -5.0, 5.0);
	bow_depth_rudder.to_angle = bow_depth_rudder.max_angle * rd / 5.0;
	stern_depth_rudder.to_angle = stern_depth_rudder.max_angle * rd / 5.0;
	// when error below a certain limit, set head_to_fixed=false, rudder_to=ruddermidships
	if (fabs(depthdiff) <= 0.25 && fabs(bow_depth_rudder.angle) < 1.0) {
		permanent_dive = true;
		bow_depth_rudder.midships();
		stern_depth_rudder.midships();
		//std::cout << "reached course, diff=" << anglediff << " tv=" << turn_velocity << "\n";
	}
}



void submarine::ballast_tank_control_logic(double delta_time)
{
	// --------------- static depth control (ballast tanks) ---------------------------------------
	//const double kg_per_sec = 1000; // 100 liters per second can be flooded/blowed out
	// neutral code: flood as much water into the tanks so the sub floats
	// should be done only when not surfaced, though...
	double s1 = mass + mass_flooded_tanks - mymodel->get_base_mesh().volume * 1000.0/*water_density*/; //, s2 = 0;
//	double s1 = position.z - dive_to;
//	double s2 = linear_momentum.z * mass_inv;
//	double s3 = (200000 - mass_flooded_tanks) * 0.01;
//	double err = mysgn(s1)*s1*s1 + mysgn(s2)*s2*s2*20.0 + mysgn(s3)*s3*s3*0.03;
	double err = s1;//(s1 + s2*20.0)*1000.0;
//	if (err < 1e-3) {
	// force emptying of ballast tanks when going surface
	// quick & dirty solution
	if (dive_state == dive_state_surfaced || dive_state == dive_state_preparing_for_dive
	    || (!permanent_dive && dive_to > -1.0)) {
		err = 1;
		//log_debug("blowing tanks to get to surface");
	} else if (dive_state == dive_state_crashdive) {
		err = -1;
		//log_debug("flooding tanks because of crashdive");
	}
	if (err > 0) {
		// we need to go up, so empty ballast tanks
		/*double rest = */ push_air_to_ballast_tanks(1 /*m^3 of air*/ * delta_time);
		//fixme: handle compressed air storage/management
	} else if (err < 0) {
		flood_ballast_tanks();
	}
	//log_debug("volume="<<mymodel->get_base_mesh().volume<<" mass="<<mass<<" flooded tanks mass="<<mass_flooded_tanks<<" err="<<err);
	//DBGOUT8(position.z,dive_to,mass_flooded_tanks,mass,s1,s2,s3,err);
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
		
	log_debug("depth charge explosion distance to sub: " << relpos.length() << "m.");
	vector3 sdist(relpos.x, relpos.y, relpos.z /* *2.0 */);
	double sdlen = sdist.length();

	// is submarine killed immidiatly?
	if (sdlen <= deadly_radius) {
		log_info("depth charge hit!");
		kill();

	} else if (sdlen <= damage_radius) {	// handle damages
		// add damage
		vector3f bb = mymodel->get_boundbox_size();
	
		// project relative position to circle on 2d plane (y,z) parallel to sub.
		// circle's radius is proportional to strength.
		// all parts within this circle are affected to damage relative to their distance to the
		// circles center.
	
		for (unsigned i = 0; i < parts.size() /*nr_of_parts*/; ++i) {
			if (parts[i].status < 0) continue;	// avoid non existent parts.

			vector3f tmp = (damage_schemes[i].p1 + damage_schemes[i].p2) * 0.5;
				if (tmp.square_length() == 0) continue;//hack to avoid yet not exsisting data fixme
			vector3 part_center = get_pos() + vector3(
				(tmp.x - 0.5) * bb.x,
				(tmp.y - 0.5) * bb.y,
				(tmp.z - 0.5) * bb.z );
			vector3 relpos = part_center - dc.get_pos();
			// depth differences change destructive power etc. see above
			double sdlen = relpos.length();
			double strength = exp((deadly_radius - sdlen) * expfac);
			
			if (strength > 0 && strength <= 1) {
				add_saturated(parts[i].status, strength, 1.0);
			}

			log_debug("DC caused damage! relpos " << relpos.x << "," << relpos.y << "," << relpos.z << " dmg " << strength);
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



bool submarine::launch_torpedo(int tubenr, sea_object* target)
{
	if (target == 0) return false;	// maybe assert this?
	if (target == this) return false;
	
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
	if (torpedoes[tubenr].torp == 0)
		return false;

	//cout << "sol valid? " << TDC.solution_valid() << "\n";
	//fixme
	if (TDC.solution_valid()) {
		angle fired_at_angle = usebowtubes ? heading : heading + angle(180);
		angle torp_head_to = TDC.get_lead_angle() + TDC.get_parallax_angle();
		//cout << "fired at " << fired_at_angle.value() << ", head to " << torp_head_to.value() << ", is cw nearer " << torp_head_to.is_cw_nearer(fired_at_angle) << "\n";
		torpedoes[tubenr].torp->head_to_ang(torp_head_to, torp_head_to.is_cw_nearer(fired_at_angle));
		// just hand the torpedo object over to class game. tube is empty after that...
		vector3 torppos = position + (fired_at_angle.direction() * (get_length()/2 + 5 /*5m extra*/)).xy0();
		torpedoes[tubenr].torp->launch(torppos, fired_at_angle);
		gm.spawn_torpedo(torpedoes[tubenr].torp);
		torpedoes[tubenr].torp = 0;
		torpedoes[tubenr].status = stored_torpedo::st_empty;
		return true;
	} else {
		return false;
	}
}



void submarine::gun_manning_changed(bool is_gun_manned)
{
	//fixme: these events occur endlessly once active!
	log_debug("gun_manning_changed is_gun_manned="<<is_gun_manned);
	if (is_gun_manned)
		gm.add_event(new event_gun_manned());
	else
		gm.add_event(new event_gun_unmanned());
}



void submarine::compute_force_and_torque(vector3& F, vector3& T) const
{
	// static buoyancy from ballast tanks is inherently handled
	// because if the tanks are empty, they do not account to
	// total mass of submarine.
	ship::compute_force_and_torque(F, T);

	// drag by stern dive rudder
	const double water_density = 1000.0;

	vector3 Fdr, Tdr;
	bow_depth_rudder.compute_force_and_torque(Fdr, Tdr, get_local_velocity(), water_density);
	double flowforce = get_throttle_accel() * mass * rudder.deflect_factor();
	double finalflowforce = stern_depth_rudder.compute_force_and_torque(Fdr, Tdr, get_local_velocity(), water_density, flowforce);
	// when stern rudder is not at angle 0, some force points orthogonal to the
	// rudder (stern_depth_rudder.deflect_factor), so less force is available for
	// forward movement of submarine. So subtract from forward force what does not
	// bypass the rudder.
	//log_debug("Fdr=" << Fdr << " Tdr=" << Tdr);
	Fdr.y += finalflowforce - flowforce;

	F += orientation.rotate(Fdr);
	T += orientation.rotate(Tdr);

	// add torque caused from tanks here, force is computed by modifying mass
	// in simulate()
	for (std::vector<tank>::const_iterator it = tanks.begin(); it != tanks.end(); ++it) {
		double grav_force = it->get_fill() * 1000.0 /* water_density */ * -GRAVITY;
		T += orientation.rotate(it->get_pos().cross(vector3(0, 0, grav_force)));
	}
}



void submarine::flood_ballast_tanks()
{
	for (std::vector<tank>::iterator it = tanks.begin(); it != tanks.end(); ++it) {
		if (it->get_type() == tank::ballast) {
			it->set_flood_valve(true);
		}
	}
}



double submarine::push_air_to_ballast_tanks(double amount_cbm)
{
	// leverage over all ballast tanks? would be better...
	for (std::vector<tank>::iterator it = tanks.begin(); it != tanks.end(); ++it) {
		if (it->get_type() == tank::ballast) {
			amount_cbm = it->push_air_inside(amount_cbm);
			it->set_flood_valve(false); // close valves
		}
	}
	return amount_cbm;
}
