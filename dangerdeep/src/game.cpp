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

// game
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#ifdef USE_NATIVE_GL
#include <gl.h>
#else
#include "oglext/OglExt.h"
#endif
#include <SDL.h>

#include "system.h"
#include <sstream>
#include <float.h>

#include "game.h"
#include "ship.h"
#include "submarine.h"
#include "airplane.h"
#include "torpedo.h"
#include "depth_charge.h"
#include "gun_shell.h"
#include "water_splash.h"
#include "model.h"
#include "global_data.h"
#include "user_interface.h"
#include "submarine_interface.h"
#include "airplane_interface.h"
#include "ship_interface.h"
#include "texts.h"
#include "convoy.h"
#include "particle.h"
#include "sensors.h"
#include "sonar.h"
#include "network.h"
#include "matrix4.h"
#include "quaternion.h"
#include "water.h"
#include "log.h"
using std::ostringstream;
using std::pair;
using std::make_pair;
using std::list;
using std::vector;
using std::string;

const unsigned SAVEVERSION = 1;
const unsigned GAMETYPE = 0;//fixme, 0-mission , 1-patrol etc.

#define ENEMYCONTACTLOST 50000.0	// meters


const double game::TRAIL_TIME = 1.0;

/***************************************************************************/



game::ping::ping(const xml_elem& parent)
{
	pos.x = parent.attrf("posx");
	pos.y = parent.attrf("posy");
	dir = angle(parent.attrf("dir"));
	time = parent.attrf("time");
	range = parent.attrf("range");
	ping_angle = angle(parent.attrf("ping_angle"));
}



void game::ping::save(xml_elem& parent) const
{
	parent.set_attr(pos.x, "posx");
	parent.set_attr(pos.y, "posy");
	parent.set_attr(dir.value(), "dir");
	parent.set_attr(time, "time");
	parent.set_attr(range, "range");
	parent.set_attr(ping_angle.value(), "ping_angle");
}



game::sink_record::sink_record(const xml_elem& parent)
{
	dat.load(parent);
	descr = parent.attr("descr");
	mdlname = parent.attr("mdlname");
	tons = parent.attru("tons");
	specfilename = parent.attr("specfilename");
	layoutname = parent.attr("layoutname");
}



void game::sink_record::save(xml_elem& parent) const
{
	dat.save(parent);
	parent.set_attr(descr, "descr");
	parent.set_attr(mdlname, "mdlname");
	parent.set_attr(tons, "tons");
	parent.set_attr(specfilename, "specfilename");
	parent.set_attr(layoutname, "layoutname");
}



game::game()
{
	// empty, so that heirs can construct a game object. Needed for editor
	freezetime = 0;
	freezetime_start = 0;

	mywater.reset(new water(0.0));
}



game::game(const string& subtype, unsigned cvsize, unsigned cvesc, unsigned timeofday,
	unsigned timeperiod, unsigned nr_of_players)
{
/****************************************************************
	custom mission generation:
	As first find a random date and time, using time of day (tod).
	Whe have to calculate time of sunrise and sunfall for that, with some time
	until this time of day expires (5:59am is not really "night" when sunrise is at
	6:00am).
	Also weather computation is neccessary.
	Then we calculate size and structure of the convoy (to allow calculation of its
	map area).
	Then we have to calculate maximum viewing distance to know the distance of the
	sub relative to the convoy. We have to find a probable convoy position in the atlantic
	(convoy routes, enough space for convoy and sub).
	Then we place the convoy with probable course and path there.
	To do this we need a simulation of convoys in the atlantic.
	Then we place the sub somewhere randomly around the convoy with maximum viewing distance.
	Multiplayer: place several subs around the convoy with a minimum distance between each.
	Sub placement: compute a random angle. Place the sub on a line given by that angle
	around the convoy's center. Line is (0,0) + t * (dx, dy).
	Compute value t for each convoy ship so that the ship
	can be seen from the point t*(dx,dy), with maximum t (e.g. with binary subdivision
	approximation).
	The maximum t over all ships is choosen for the position.
	To do that we create and use a lookout sensor.
	This technique ignores the fact that convoys could be heared earlier than seen
	(below surface, passive sonar) or even detected by their smell (smoke)!
***********************************************************************/	
	networktype = 0;
	servercon = 0;

	// fixme: show some info like in Silent Service II? sun/moon pos,time,visibility?

	date datebegin, dateend;
	switch (timeperiod) {
		case 0: datebegin = date(1939, 9, 1); dateend = date(1940, 5, 31); break;
		case 1: datebegin = date(1940, 6, 1); dateend = date(1941, 3, 31); break;
		case 2: datebegin = date(1941, 4, 1); dateend = date(1941, 12, 31); break;
		case 3: datebegin = date(1942, 1, 1); dateend = date(1942, 6, 30); break;
		case 4: datebegin = date(1942, 7, 1); dateend = date(1942, 12, 31); break;
		case 5: datebegin = date(1943, 1, 1); dateend = date(1943, 5, 31); break;
		case 6: datebegin = date(1943, 6, 1); dateend = date(1944, 6, 30); break;
		case 7: datebegin = date(1944, 7, 1); dateend = date(1945, 5, 8); break;
	}

	double tpr = rnd();	
	time = datebegin.get_time() * (1.0-tpr) + dateend.get_time() * tpr;
	time = floor(time/86400)*86400;		// set to begin of day
	
	// all code from here on is fixme and experimental.
	// fixme: we need exact sunrise and fall times for a day. (also moon state is needed
	// later) The compute_sun_pos func is not enough
	switch (timeofday) {
		case 0: time += myfmod(20+10*rnd(), 24)*3600; break;	// night
		case 1: time += ( 6+ 2*rnd())*3600; break;		// dawn
		case 2: time += ( 8+10*rnd())*3600; break;		// day
		case 3: time += (18+ 2*rnd())*3600; break;		// dusk
	}
	
	date currentdate((unsigned)time);
	equipment_date = currentdate;	// fixme: another crude guess or hack

	mywater.reset(new water(time));

	// Convoy-constructor creates all the objects and spawns them in this game object.
	// fixme: creation of convoys should be rather moved to this class, so object creation
	// and logic is centralized.
	convoy* cv = new convoy(*this, (convoy::types)(cvsize), (convoy::esctypes)(cvesc));
	spawn_convoy(cv);

	lookout_sensor tmpsensor;
	vector<angle> subangles;
	submarine* psub = 0;
	for (unsigned i = 0; i < nr_of_players; ++i) {
		xml_doc doc(data_file().get_filename(subtype));
		doc.load();
		submarine* sub = new submarine(*this, doc.first_child());
		sub->set_skin_layout(model::default_layout);
		sub->init_fill_torpedo_tubes(currentdate);
		if (i == 0) {
			psub = sub;
			player = psub;
			compute_max_view_dist();
		}
		
		// distribute subs randomly around convoy.
		angle tmpa;
		double anglediff = 90.0;
		bool angleok = false;
		unsigned angletries = 0;
		do {
			angleok = true;
			tmpa = (rnd()*360.0);
			for (unsigned j = 0; j < subangles.size(); ++j) {
				if (tmpa.diff(subangles[j]) < anglediff) {
					angleok = false;
					break;
				}
			}
			if (!angleok) {
				++angletries;
				if (angletries >= nr_of_players) {
					angletries = 0;
					anglediff /= 2.0;
				}
			}
		} while (!angleok);
		
		// now tmpa holds the angle of the sub's position around the convoy.
		double maxt = 0;
		for (unsigned k = 0; k < ships.size(); ++k) {
			double maxt1 = 0, maxt2 = get_max_view_distance()/2, maxt3 = get_max_view_distance();
			sub->manipulate_position((maxt2 * tmpa.direction()).xy0());
			// find maximum distance t along line (0,0)+t*tmpa.dir() for this ship
			while (maxt3 - maxt1 > 50.0) {
				if (tmpsensor.is_detected(this, sub, ships[k])) {
					maxt1 = maxt2;
				} else {
					maxt3 = maxt2;
				}
				maxt2 = (maxt1 + maxt3)/2;
				sub->manipulate_position((maxt2 * tmpa.direction()).xy0());
			}
			if (maxt2 > maxt) maxt = maxt2;
		}
		vector3 subpos = (maxt * tmpa.direction()).xy0();
		subpos.z = (timeofday == 2) ? 0 : -12; // fixme maybe always surfaced, except late in war
		sub->manipulate_position(subpos);
		// heading should be facing to the convoy (+-90deg), as it is unrealistic
		// to detect a convoy while moving away from it
		sub->manipulate_heading(angle(rnd()*180.0 + 90.0) + tmpa);
	
		spawn_submarine(sub);
	}
	player = psub;

	my_run_state = running;
	last_trail_time = time - TRAIL_TIME;

	freezetime = 0;
	freezetime_start = 0;
}



// --------------------------------------------------------------------------------
//                        LOAD GAME (SAVEGAME OR MISSION)
// --------------------------------------------------------------------------------
game::game(const string& filename)
	: my_run_state(running), player(0),
	  time(0), last_trail_time(0), max_view_dist(0), networktype(0), servercon(0),
	  freezetime(0), freezetime_start(0)
{
	xml_doc doc(filename);
	doc.load();
	// could be savegame or mission, maybe check...
	// has_child("dftd-savegame") or has_child("dftd-mission");
	xml_elem sg = doc.first_child();
	// fixme: check for savegames.
//	unsigned v = sg.attr(version);
//	if (v != SAVEVERSION)
//		throw error("invalid game version");


	// load state first, because time is stored there and we need time/date for checks
	// while loading the rest.
	xml_elem gst = sg.child("state");
	time = gst.attrf("time");
	last_trail_time = gst.attrf("last_trail_time");
	equipment_date.load(gst.child("equipment_date"));
	max_view_dist = gst.attrf("max_view_dist");

	// fixme: save original water creation time and random seed with that water was generated.
	// set the same seed here again, so water is exactly like it was at game start.
	mywater.reset(new water(time));

	// create empty objects so references can be filled.
	// there must be ships in a mission...
	xml_elem sh = sg.child("ships");
	for (xml_elem::iterator it = sh.iterate("ship"); !it.end(); it.next()) {
		xml_doc spec(data_file().get_filename(it.elem().attr("type")));
		spec.load();
		ships.push_back(new ship(*this, spec.first_child()));
	}

	// there must be submarines in a mission...
	xml_elem su = sg.child("submarines");
	for (xml_elem::iterator it = su.iterate("submarine"); !it.end(); it.next()) {
		xml_doc spec(data_file().get_filename(it.elem().attr("type")));
		spec.load();
		submarines.push_back(new submarine(*this, spec.first_child()));
	}

	if (sg.has_child("airplanes")) {
		xml_elem ap = sg.child("airplanes");
		for (xml_elem::iterator it = ap.iterate("airplane"); !it.end(); it.next()) {
			xml_doc spec(data_file().get_filename(it.elem().attr("type")));
			spec.load();
			airplanes.push_back(new airplane(*this, spec.first_child()));
		}
	}

	if (sg.has_child("torpedoes")) {
		xml_elem tp = sg.child("torpedoes");
		for (xml_elem::iterator it = tp.iterate("torpedo"); !it.end(); it.next()) {
			xml_doc spec(data_file().get_filename(it.elem().attr("type")));
			spec.load();
			torpedoes.push_back(new torpedo(*this, spec.first_child()));
		}
	}

	if (sg.has_child("depth_charges")) {
		xml_elem dc = sg.child("depth_charges");
		for (xml_elem::iterator it = dc.iterate("depth_charge"); !it.end(); it.next()) {
			//xml_doc spec(get_depth_charge_dir() + it.elem().attr("type") + ".xml");
			//spec.load();
			depth_charges.push_back(new depth_charge(*this/*, spec.first_child()*/));
		}
	}

	if (sg.has_child("gun_shells")) {
		xml_elem gs = sg.child("gun_shells");
		for (xml_elem::iterator it = gs.iterate("gun_shell"); !it.end(); it.next()) {
			//xml_doc spec(get_gun_shell_dir() + it.elem().attr("type") + ".xml");
			//spec.load();
			gun_shells.push_back(new gun_shell(*this/*, spec.first_child()*/));
		}
	}

	if (sg.has_child("convoys")) {
		xml_elem cv = sg.child("convoys");
		for (xml_elem::iterator it = cv.iterate("convoy"); !it.end(); it.next()) {
			//xml_doc spec(get_convoy_dir() + it.elem().attr("type") + ".xml");
			//spec.load();
			convoys.push_back(new convoy(*this/*, spec.first_child()*/));
		}
	}

	// fixme: handle water splashes too.

#if 0
	if (sg.has_child("particles")) {
		xml_elem pt = sg.child("particles");
		for (xml_elem::iterator it = pt.iterate("particle"); !it.end(); it.next()) {
			//xml_doc spec(get_particle_dir() + it.elem().attr("type") + ".xml");
			//spec.load();
			particles.push_back(new particle(*this/*, spec.first_child()*/));
		}
	}
#endif

	// now all objects exist and must get loaded
	unsigned k = 0;
	for (xml_elem::iterator it = sh.iterate("ship"); !it.end(); it.next(), ++k) {
		ships[k]->load(it.elem());
	}

	k = 0;
	for (xml_elem::iterator it = su.iterate("submarine"); !it.end(); it.next(), ++k) {
		submarines[k]->load(it.elem());
	}

	if (airplanes.size() > 0) {
		k = 0;
		for (xml_elem::iterator it = sg.child("airplanes").iterate("airplane"); !it.end(); it.next(), ++k) {
			airplanes[k]->load(it.elem());
		}
	}

	if (torpedoes.size() > 0) {
		k = 0;
		for (xml_elem::iterator it = sg.child("torpedoes").iterate("torpedo"); !it.end(); it.next(), ++k) {
			torpedoes[k]->load(it.elem());
		}
	}

	if (depth_charges.size() > 0) {
		k = 0;
		for (xml_elem::iterator it = sg.child("depth_charges").iterate("depth_charge"); !it.end(); it.next(), ++k) {
			depth_charges[k]->load(it.elem());
		}
	}

	if (gun_shells.size() > 0) {
		k = 0;
		for (xml_elem::iterator it = sg.child("gun_shells").iterate("gun_shell"); !it.end(); it.next(), ++k) {
			gun_shells[k]->load(it.elem());
		}
	}

	if (convoys.size() > 0) {
		k = 0;
		for (xml_elem::iterator it = sg.child("convoys").iterate("convoy"); !it.end(); it.next(), ++k) {
			convoys[k]->load(it.elem());
		}
	}

	// fixme: handle water splashes too

#if 0
	if (particles.size() > 0) {
		k = 0;
		for (xml_elem::iterator it = sg.child("particles").iterate("particle"); !it.end(); it.next(), ++k) {
			particles[k]->load(it.elem());
		}
	}
#endif

	// create jobs fixme - at the moment the job interface is not used.
	// use it for regularly updating weather/sky/waves etc. etc.

	// load player
	xml_elem pl = sg.child("player");
	player = load_ptr(pl.attru("ref"));
	// fixme: maybe check if type matches!

	// ui is created from client of game!

	xml_elem sks = sg.child("sunken_ships");
	for (xml_elem::iterator it = sks.iterate("sink_record"); !it.end(); it.next()) {
		sunken_ships.push_back(sink_record(it.elem()));
	}

	//fixme save and load logbook

	xml_elem pgs = sg.child("pings");
	for (xml_elem::iterator it = pgs.iterate("ping"); !it.end(); it.next()) {
		pings.push_back(ping(it.elem()));
	}
}




game::~game()
{
	for (list<pair<double, job*> >::iterator it = jobs.begin(); it != jobs.end(); ++it)
		delete it->second;
}



// --------------------------------------------------------------------------------
//                        SAVE GAME
// --------------------------------------------------------------------------------

void game::save(const string& savefilename, const string& description) const
{
	xml_doc doc(savefilename);
	xml_elem sg = doc.add_child("dftd-savegame");
	sg.set_attr(description, "description");
	sg.set_attr(SAVEVERSION, "version");
	sg.set_attr(GAMETYPE, "type");

	xml_elem sh = sg.add_child("ships");
	sh.set_attr(ships.size(), "nr");
	for (unsigned k = 0; k < ships.size(); ++k) {
		xml_elem e = sh.add_child("ship");
		e.set_attr(ships[k]->get_specfilename(), "type");
		ships[k]->save(e);
	}

	xml_elem su = sg.add_child("submarines");
	su.set_attr(submarines.size(), "nr");
	for (unsigned k = 0; k < submarines.size(); ++k) {
		xml_elem e = su.add_child("submarine");
		e.set_attr(submarines[k]->get_specfilename(), "type");
		submarines[k]->save(e);
	}

	xml_elem ap = sg.add_child("airplanes");
	ap.set_attr(airplanes.size(), "nr");
	for (unsigned k = 0; k < airplanes.size(); ++k) {
		xml_elem e = ap.add_child("airplane");
		e.set_attr(airplanes[k]->get_specfilename(), "type");
		airplanes[k]->save(e);
	}

	xml_elem tp = sg.add_child("torpedoes");
	tp.set_attr(torpedoes.size(), "nr");
	for (unsigned k = 0; k < torpedoes.size(); ++k) {
		xml_elem e = tp.add_child("torpedo");
		e.set_attr(torpedoes[k]->get_specfilename(), "type");
		torpedoes[k]->save(e);
	}

	xml_elem dc = sg.add_child("depth_charges");
	dc.set_attr(depth_charges.size(), "nr");
	for (unsigned k = 0; k < depth_charges.size(); ++k) {
		xml_elem e = dc.add_child("depth_charge");
		//e.set_attr(depth_charges[k]->get_specfilename(), "type");//no specfilename for DCs
		depth_charges[k]->save(e);
	}

	xml_elem gs = sg.add_child("gun_shells");
	gs.set_attr(gun_shells.size(), "nr");
	for (unsigned k = 0; k < gun_shells.size(); ++k) {
		xml_elem e = gs.add_child("gun_shell");
		//e.set_attr(gun_shells[k]->get_specfilename(), "type");//no specfilename for shells
		gun_shells[k]->save(e);
	}

	xml_elem cv = sg.add_child("convoys");
	cv.set_attr(convoys.size(), "nr");
	for (unsigned k = 0; k < convoys.size(); ++k) {
		xml_elem e = cv.add_child("convoy");
		//e.set_attr(convoys[k]->get_specfilename(), "type");//no specfilename for convoys
		convoys[k]->save(e);
	}

#if 0	// fixme later!!!
	xml_elem pt = sg.add_child("particles");
	pt.set_attr(particles.size(), "nr");
	for (unsigned k = 0; k < particles.size(); ++k) {
		xml_elem e = pt.add_child("particle");
		//e.set_attr(particles[k]->get_specfilename(), "type");//no specfilename for particles
		particles[k]->save(e);
	}
#endif

	// my_run_state doesn't need to be saved
	
	// jobs are generated by dftd itself

	// save player
	submarine* tmpsub = dynamic_cast<submarine*>(player);
	string pltype;
	if (tmpsub) {
		pltype = "submarine";
	} else {
		ship* tmpship = dynamic_cast<ship*>(player);
		if (tmpship) {
			pltype = "ship";
		} else {
			airplane* tmpairpl = dynamic_cast<airplane*>(player);
			if (tmpairpl) {
				pltype = "airplane";
			} else {
				throw error("internal error: player is no sub, ship or airplane");
			}
		}
	}
	xml_elem pl = sg.add_child("player");
	pl.set_attr(save_ptr(player), "ref");
	pl.set_attr(pltype, "type");
	
	// user interface is generated according to player object by dftd

	xml_elem sks = sg.add_child("sunken_ships");
	sks.set_attr(unsigned(sunken_ships.size()), "nr");
	for (list<sink_record>::const_iterator it = sunken_ships.begin(); it != sunken_ships.end(); ++it) {
		it->save(sks);
	}

	//fixme save and load logbook

	xml_elem gst = sg.add_child("state");
	gst.set_attr(time, "time");
	// save current date as reference for human readers.
	date(unsigned(time)).save(gst);
	gst.set_attr(last_trail_time, "last_trail_time");
	xml_elem equ = gst.add_child("equipment_date");
	equipment_date.save(equ);
	gst.set_attr(max_view_dist, "max_view_dist");
	
	xml_elem pgs = sg.add_child("pings");
	pgs.set_attr(unsigned(pings.size()), "nr");
	for (list<ping>::const_iterator it = pings.begin(); it != pings.end(); ++it) {
		it->save(pgs);
	}

	// finally save file
	doc.save();
}



string game::read_description_of_savegame(const string& filename)
{
	// causes 90mb mem leak fixme
	xml_doc doc(filename);
	doc.load();
	xml_elem sg = doc.child("dftd-savegame");
	unsigned v = sg.attru("version");
	if (v != SAVEVERSION)
		return "<ERROR> Invalid version";
	string d = sg.attr("description");
	if (d.length() == 0)
		return "<ERROR> Empty description";
	return d;
}



void game::compute_max_view_dist()
{
	// a bit unprecise here, since the viewpos is not always the same as the playerpos
	// this must depend also on weather, fog, rain etc.
	vector3 sundir;
	max_view_dist = 5000.0 + compute_light_brightness(player->get_pos(), sundir) * 25000;
}



template<class T> void cleanup(ptrset<T>& s)
{
	for (unsigned i = 0; i < s.size(); ++i) {
		if (s[i] && s[i]->is_defunct()) {
			s.reset(i);
		}
	}
	s.compact();
}



void game::simulate(double delta_t)
{
	if (my_run_state != running) return;

	// protect physics simulation from bad values, simulation step must not
	// be less than 20fps.
	const double max_dt_rate = 1.0/20.0;
	if (delta_t > max_dt_rate) {
		// do some intermediate steps. All larger than max_dt_rate, so add a small amount.
		unsigned steps = unsigned(ceil(delta_t / max_dt_rate + 0.001));
		double ddt = delta_t / steps;
		log_debug("Large delta_t (" << delta_t << "), using " << steps << " steps in between.");
		for (unsigned s = 1; s < steps; ++s) {
			simulate(ddt);
			delta_t -= ddt;
		}
		simulate(delta_t);
		return;
	}

	// kill events left over from last run
	events.clear();

	// check if jobs are to be run
	for (list<pair<double, job*> >::iterator it = jobs.begin(); it != jobs.end(); ++it) {
		it->first += delta_t;
		if (it->first >= it->second->get_period()) {
			it->first -= it->second->get_period();
			it->second->run();
		}
	}

	// this could be done in jobs, fixme
	if (!player->is_alive()) {
		log_info("player killed!");//testing fixme
		my_run_state = player_killed;
		return;
	}
	
	if (/* submarines.size() == 0 && */ ships.size() == 0 && torpedoes.size() == 0 && depth_charges.size() == 0 &&
			airplanes.size() == 0 && gun_shells.size() == 0) {
		log_info("no objects except player left!");//testing fixme
		my_run_state = mission_complete; // or also contact lost?
		return;
	}

	compute_max_view_dist();
	
	bool record = false;
	if (get_time() >= last_trail_time + TRAIL_TIME) {
		last_trail_time = get_time();
		record = true;
	}
	
	//fixme 2003/07/11: time compression trashes trail recording.

	double nearest_contact = 1e10;

	// simulation for each object
	// Note! Simulation order does not matter, because every killed
	// object is kept for two rounds (state change to dead2, then
	// defunct) because state change happens only in
	// sea_object::simulate.
	// Even if e.g. three objects A, B, C are simulated in that order.
	// A has reference to C and B kills C when B is simulated. Then
	// C's state is dead, and when C is simulated afterwards, its
	// state changes to dead2. On the next call to game::simulate
	// the object C is NOT deleted, because it is not defunct
	// yet, only one round later.
	// This example shows how it works, and also that we need the
	// extra dead state "dead2". Otherwise C would have been deleted
	// before A is simulated again, when A still has a reference to
	// C which would give a SIGSEGV.
	// And because every reference to a sea_object must be checked for
	// validity at least every round, we can avoid to reference
	// deleted objects.

	// step 1: check for invalidity of every object and remove
	// defunct objects. do NOT mix simulate() calls with real
	// calls to delete an object.
	cleanup(ships);
	cleanup(submarines);
	cleanup(airplanes);
	cleanup(torpedoes);
	cleanup(depth_charges);
	cleanup(gun_shells);
	cleanup(water_splashes);

	// step 2: simulate all objects, possibly setting state to dead/defunct.
	// ------------------------------ ships ------------------------------
	for (unsigned i = 0; i < ships.size(); ++i) {
		if (ships[i] != player) {
			double dist = ships[i]->get_pos().distance(player->get_pos());
			if (dist < nearest_contact) nearest_contact = dist;
		}
		try {
			ships[i]->simulate(delta_t);
			if (record) ships[i]->remember_position(get_time());
		}
		catch (sea_object::is_dead_exception& ) {
			// nothing to do
		}
	}

	// ------------------------------ submarines ------------------------------
	for (unsigned i = 0; i < submarines.size(); ++i) {
		if (submarines[i] != player) {
			double dist = submarines[i]->get_pos().distance(player->get_pos());
			if (dist < nearest_contact) nearest_contact = dist;
		}
		try {
			submarines[i]->simulate(delta_t);
			if (record) submarines[i]->remember_position(get_time());
		}
		catch (sea_object::is_dead_exception& ) {
			// nothing to do
		}
	}

	// ------------------------------ airplanes ------------------------------
	for (unsigned i = 0; i < airplanes.size(); ++i) {
		if (airplanes[i] != player) {
			double dist = airplanes[i]->get_pos().distance(player->get_pos());
			if (dist < nearest_contact) nearest_contact = dist;
		}
		try {
			airplanes[i]->simulate(delta_t);
		}
		catch (sea_object::is_dead_exception& ) {
			// nothing to do
		}
	}

	// ------------------------------ torpedoes ------------------------------
	for (unsigned i = 0; i < torpedoes.size(); ++i) {
		try {
			torpedoes[i]->simulate(delta_t);
			if (record) torpedoes[i]->remember_position(get_time());
		}
		catch (sea_object::is_dead_exception& ) {
			// nothing to do
		}
	}

	// ------------------------------ depth_charges ------------------------------
	for (unsigned i = 0; i < depth_charges.size(); ++i) {
		try {
			depth_charges[i]->simulate(delta_t);
		}
		catch (sea_object::is_dead_exception& ) {
			// nothing to do
		}
	}

	// ------------------------------ gun_shells ------------------------------
	for (unsigned i = 0; i < gun_shells.size(); ++i) {
		try {
			gun_shells[i]->simulate(delta_t);
		}
		catch (sea_object::is_dead_exception& ) {
			// nothing to do
		}
	}

	// ------------------------------ water_splashes ------------------------------
	for (unsigned i = 0; i < water_splashes.size(); ++i) {
		try {
			water_splashes[i]->simulate(delta_t);
		}
		catch (sea_object::is_dead_exception& ) {
			// nothing to do
		}
	}

	// for convoys/particles it doesn't hurt to mix simulate() with compact().
	// ------------------------------ convoys ------------------------------
	for (unsigned i = 0; i < convoys.size(); ++i) {
		if (!convoys[i]) continue;
		convoys[i]->simulate(delta_t);	// fixme: handle erasing of empty convoys!
	}
	convoys.compact();

	// ------------------------------ particles ------------------------------
	for (unsigned i = 0; i < particles.size(); ++i) {
		if (!particles[i]) continue;
		if (particles[i]->is_defunct()) {
			particles.reset(i);
		} else {
			particles[i]->simulate(*this, delta_t);
		}
	}
	particles.compact();

	time += delta_t;

	// remove old pings
	for (list<ping>::iterator it = pings.begin(); it != pings.end(); ) {
		list<ping>::iterator it2 = it++;
		if (time - it2->time > PINGREMAINTIME)
			pings.erase(it2);
	}
	
	if (nearest_contact > ENEMYCONTACTLOST) {
		log_info("player lost contact to enemy!");//testing fixme
		my_run_state = contact_lost;
	}
}



void game::add_logbook_entry(const string& s)
{
	// fixme: format of date is fix in logbook then, this is not optimal.
	// when player changes language, format is not changed on display...
	players_logbook.add_entry(texts::numeric_from_daytime(date(unsigned(get_time()))) + " : " + s);
}



/******************************************************************************************
	Visibility computation
	----------------------

	Visibility is determined by two factors:
	1) overall visibility
		- time of day (sun position -> brightness)
		- weather
		- moon phase and position during night
	2) specific visibility
		- object type
		- surfaced: course, speed, engine type etc.
		- submerged: speed, scope height etc.
		- relative position to sun/moon
	The visibility computation gives a distance within that the object can be seen
	by other objects. This distance depends on relative distance and courses of
	both objects (factor2) and overall visibility (factor1).
	Maybe some randomization should be added (quality of crew, experience, overall
	visibility +- some meters)
	
	Visibility of ships can be determined by area that is visible and this depends
	on relative course between watcher and visible object.
	For ships this shouldn't make much difference (their shape is much higher than
	that of submarines), but for subs the visibility is like an ellipse given
	by an implicit function, rotated by course, roughly proportional to length and with.
	So we can compute visibiltiy by just multiplying relative coordinates (x, y, 1) with
	a 3x3 matrix from left and right and evaluate the result.
	
******************************************************************************************/

template <class T> inline vector<T*> visible_obj(const game* gm, const ptrset<T>& v, const sea_object* o)
{
	vector<T*> result;
	const sensor* s = o->get_sensor(o->lookout_system);
	if (!s) return result;
	const lookout_sensor* ls = dynamic_cast<const lookout_sensor*>(s);
	if (!ls) return result;
	result.reserve(v.size());
	for (unsigned i = 0; i < v.size(); ++i) {
		// do not handle dead or defunct objects!
		if (v[i] && v[i]->is_reference_ok()) {
			if (ls->is_detected(gm, o, v[i]))
				result.push_back(v[i]);
		}
	}
	return result;
}


vector<ship*> game::visible_ships(const sea_object* o) const
{
	return visible_obj<ship>(this, ships, o);
}

vector<submarine*> game::visible_submarines(const sea_object* o) const
{
	return visible_obj<submarine>(this, submarines, o);
}

vector<airplane*> game::visible_airplanes(const sea_object* o) const
{
	return visible_obj<airplane>(this, airplanes, o);
}

vector<torpedo*> game::visible_torpedoes(const sea_object* o) const
{
//testing: draw all torpedoes
	vector<torpedo*> result;
	result.reserve(torpedoes.size());
	// Note!!! all entries of "torpedoes" should be valid pointers here, not
	// null pointers, but it seems sometimes some are null, leading to
	// segfault on torpedo impact when not checking for null pointers here!
	// this is bad, but a check here is cheap...
	// torpedoes.compress() should remove any null pointers before display
	// is rendered, but it crashes when drawing trails because of null
	// pointers...
	for (unsigned k = 0; k < torpedoes.size(); ++k) {
		if (torpedoes[k] && torpedoes[k]->is_reference_ok()) {
			result.push_back(torpedoes[k]);
		}
	}
	return result;
//	return visible_obj<torpedo>(this, torpedoes, o);
}

vector<depth_charge*> game::visible_depth_charges(const sea_object* o) const
{
	return visible_obj<depth_charge>(this, depth_charges, o);
}

vector<gun_shell*> game::visible_gun_shells(const sea_object* o) const
{
	return visible_obj<gun_shell>(this, gun_shells, o);
}

vector<water_splash*> game::visible_water_splashes(const sea_object* o) const
{
//testing: draw all 
	vector<water_splash*> result(water_splashes.size());
	for (unsigned k = 0; k < water_splashes.size(); ++k)
		result[k] = water_splashes[k];
	return result;
//	return visible_obj<water_splash>(this, water_splashes, o);
}

vector<particle*> game::visible_particles(const sea_object* o ) const
{
	//fixme: this is called for every particle. VERY costly!!!
	vector<particle*> result;
	const sensor* s = o->get_sensor(o->lookout_system);
	if (!s) return result;
	const lookout_sensor* ls = dynamic_cast<const lookout_sensor*>(s);
	if (!ls) return result;
	result.reserve(particles.size());
	for (unsigned i = 0; i < particles.size(); ++i) {
		if (particles[i] == 0) // obsolete test? should be so...
			throw error("particles[i] is 0!");
		if (ls->is_detected(this, o, particles[i]))
			result.push_back(particles[i]);
	}
	return result;
}

vector<sonar_contact> game::sonar_ships (const sea_object* o ) const
{
	vector<sonar_contact> result;
	const sensor* s = o->get_sensor ( o->passive_sonar_system );
	if (!s) return result;
	const passive_sonar_sensor* pss = dynamic_cast<const passive_sonar_sensor*> ( s );
	if (!pss) return result;

	result.reserve(ships.size());

	// collect the nearest contacts, limited to some value!
	vector<pair<double, ship*> > contacts ( MAX_ACUSTIC_CONTACTS, make_pair ( 1e30, (ship*) 0 ) );
	for (unsigned k = 0; k < ships.size(); ++k) {
		// do not handle dead/defunct objects
		if (!ships[k]->is_reference_ok()) continue;

		// When the detecting unit is a ship it should not detect itself.
		if ( o == ships[k] )
			continue;

		double d = ships[k]->get_pos ().xy ().square_distance ( o->get_pos ().xy () );
		unsigned i = 0;
		for ( ; i < contacts.size (); ++i ) {
			if ( contacts[i].first > d )
				break;
		}

		if ( i < contacts.size () ) {
			for ( unsigned j = contacts.size ()-1; j > i; --j )
				contacts[j] = contacts[j-1];

			contacts[i] = make_pair ( d, ships[k] );
		}
	}

	unsigned size = contacts.size ();
	for (unsigned i = 0; i < size; i++ ) {
		ship* sh = contacts[i].second;
		if ( sh == 0 )
			break;

		if ( pss->is_detected ( this, o, sh ) )
			result.push_back(sonar_contact(sh->get_pos().xy(), sh->get_class()));
	}
	return result;
}

vector<sonar_contact> game::sonar_submarines (const sea_object* o ) const
{
	vector<sonar_contact> result;
	const sensor* s = o->get_sensor ( o->passive_sonar_system );
	if (!s) return result;
	const passive_sonar_sensor* pss = dynamic_cast<const passive_sonar_sensor*> ( s );
	if (!pss) return result;
	result.reserve(submarines.size());
	for (unsigned k = 0; k < submarines.size(); ++k) {
		// do not handle dead/defunct objects
		if (!submarines[k]->is_reference_ok()) continue;

		// When the detecting unit is a submarine it should not
		// detect itself.
		if ( o == submarines[k] )
			continue;

		if ( pss->is_detected ( this, o, submarines[k] ) )
			result.push_back(sonar_contact(submarines[k]->get_pos().xy(), submarines[k]->get_class()));
	}
	return result;
}

vector<sonar_contact> game::sonar_sea_objects(const sea_object* o) const
{
	vector<sonar_contact> sships = sonar_ships(o);
	vector<sonar_contact> ssubmarines = sonar_submarines(o);
	sships.reserve(sships.size() + ssubmarines.size());
	for (unsigned i = 0; i < ssubmarines.size(); ++i)
		sships.push_back(ssubmarines[i]);
	return sships;
}

vector<submarine*> game::radar_submarines(const sea_object* o) const
{
	vector<submarine*> result;
	const sensor* s = o->get_sensor ( o->radar_system );
	if (!s) return result;
	const radar_sensor* ls = dynamic_cast<const radar_sensor*> ( s );
	if (!ls) return result;
	result.reserve(submarines.size());
	for (unsigned k = 0; k < submarines.size(); ++k) {
		if ( ls->is_detected ( this, o, submarines[k] ) )
			result.push_back (submarines[k]);
	}
	return result;
}

vector<ship*> game::radar_ships(const sea_object* o) const
{
	vector<ship*> result;
	const sensor* s = o->get_sensor ( o->radar_system );
	if (!s) return result;
	const radar_sensor* ls = dynamic_cast<const radar_sensor*> ( s );
	if (!ls) return result;
	result.reserve(ships.size());
	for (unsigned k = 0; k < ships.size(); ++k) {
		if ( ls->is_detected ( this, o, ships[k] ) )
			result.push_back (ships[k]);
	}
	return result;
}

vector<sea_object*> game::radar_sea_objects(const sea_object* o) const
{
	vector<ship*> rships = radar_ships(o);
	vector<submarine*> rsubmarines = radar_submarines(o);
	vector<sea_object*> result;
	result.reserve(rships.size() + rsubmarines.size());
	append_vec(result, rships);
	append_vec(result, rsubmarines);
	return result;
}

vector<vector2> game::convoy_positions() const
{
	vector<vector2> result;
	result.reserve(convoys.size());
	for (unsigned k = 0; k < convoys.size(); ++k) {
		result.push_back(convoys[k]->get_pos());
	}
	return result;
}



pair<double, noise> game::sonar_listen_ships(const ship* listener,
					angle rel_listening_dir) const
{
	// collect all ships for sound strength measurement
	vector<const ship*> tmpships;
	tmpships.reserve(ships.size() + submarines.size() /* + torpedoes.size() */ - 1);
	for (unsigned i = 0; i < ships.size(); ++i)
		if (ships[i] != listener)
			tmpships.push_back(ships[i]);
	for (unsigned i = 0; i < submarines.size(); ++i)
		if (dynamic_cast<const ship*>(submarines[i]) != listener)
			tmpships.push_back(submarines[i]);
	// fixme: add torpedoes here as well... later...

#if 0
	// fixme, test, only detect one ship
	tmpships.resize(1);
#endif

	// fixme: the lower part of this function is sonar dependent and should go to a sonar class...

	// compute noise strengths for all ships for all frequency bands, real strengths, not dB!
	noise n;
#if 1
	// as first, add background noise
	n += noise::compute_ambient_noise_strength(0.2 /* sea state, fixme make dynamic later */);
#endif

	// next, add noise from receiver vessel
	// if we do that, weaker noises are wiped out...
	// fixme: GHG/BG have blind spots at aft, so the receiver caused noise is reduced much more.
	// we should handle receiver vessel as additional noise source with distance 50, direction 180° relative!
	n += listener->get_noise_signature().compute_signal_strength(50 /* distance */,
								     listener->get_speed(),
								     false /*cavitation=off for listener*/);

	angle hdg = listener->get_heading();
	bool listen_to_starboard = (rel_listening_dir.value_pm180() >= 0);

	// detection formula:
	// compute noise of target = L_t
	// compute ambient noise = L_a
	// compute sensor background noise (noise from own vessel, receiver) = L_r
	// store sensor sensitivity = S
	// The signal is detected when L_t - (L_a + L_r) > S
	// which is equivalent to L_t > S + L_a + L_r
	// This means loud background noise (e.g. rough seas) or high noise from own
	// vessel shadows target noise. We need to quantize the received noise somehow
	// or we could always find loudest source if background and own noise is constant,
	// because target noise would be the only varying factor.
	// The current way of detecting sounds is thus not realistic, as the highest
	// noise signal is always detected, no matter how big the difference to the back-
	// ground signals is...
	// By using the formulas above, noise strengths are multiplied (logarithmic scale!)
	// instead of copied. Maybe this models shadowing better...
	// weak signal of 1 dB and strong signal of 50 dB.
	// Adding them and computing dB of addition gives 50.000055 dB, total shadowed.
	// Adding them in dB scale gives 51 dB, which is not right.
	// What about subtraction? 50 dB - 1 dB = 49 dB, in real scale 49.99995 dB, nearly 50.
	// So adding dB values is bad, as weaker signals get accounted much stronger
	// than they are.
	// Solution maybe: quantize the target's noise, so weaker signals have the same
	// quantum as the background noise and vanish.

	// fixme: ghost images appear with higher frequencies!!! seems to be a ghg "feature"

	// add noise of vessels
	vector2 lp = listener->get_pos().xy();
	for (unsigned i = 0; i < tmpships.size(); ++i) {
		const ship* s = tmpships[i];
		vector2 relpos = s->get_pos().xy() - lp;
		double distance = relpos.length();
		double speed = s->get_speed();	// s->get_throttle_speed();
		bool cavit = s->screw_cavitation();
		angle direction_to_noise(relpos);
		angle rel_dir_to_noise = direction_to_noise - hdg;
		bool noise_is_starboard = (rel_dir_to_noise.value_pm180() >= 0);
		// check if noise is on active side of phones
		if (listen_to_starboard == noise_is_starboard) {
			noise nsig = s->get_noise_signature().compute_signal_strength(distance, speed, cavit);
			// compute strengths for all bands
			for (unsigned b = 0; b < noise::NR_OF_FREQUENCY_BANDS; ++b) {
				double signalstrength = compute_signal_strength_GHG(rel_dir_to_noise,
										    noise::typical_frequency[b],
										    rel_listening_dir);
				//printf("signalstrength is = %f\n", signalstrength);
				nsig.frequencies[b] *= signalstrength;
			}
			n += nsig;
		}
	}
	// now compute back to dB, quantize to integer dB values, to
	// simulate shadowing of weak signals by background noise
	// divide by receiver sensitivity before doing so, to avoid cutting off weak signals.
	const double GHG_receiver_sensitivity_dB = -3;	// weakest signal strength to be detectable
	double abs_strength =
		floor(std::max(n.compute_total_noise_strength_dB() - GHG_receiver_sensitivity_dB, 0.0))
		+ GHG_receiver_sensitivity_dB;

	// fixme: depending on listener angle, use only port or starboard phones to listen to signals!
	//        (which set to use must be given as parameter) <OK>
	// fixme: add sensitivity of receiver (see harpoon docs...)  TO BE DONE NEXT
	// fixme: add noise produced by receiver/own ship            TO BE DONE NEXT
	// fixme: discretize strengths!!! (by angle and frequency!) <OK, MAYBE A BIT CRUDE>
	//        this could be done also by quantizing the strength (in dB) of the signal,
	//        or both. To be tested...
	// fixme: identify type of noise (by sonarman). compute similarity to known
	//        noise signatures (minimum sim of squares of distances between measured
	//        values and known reference values). this should be done in another function...
	//        To do this store a list of typical noise signatures per ship
	//        category - that is also needed for creating the noise signals.
	//        <OK> BUT: this doesnt work well. To determine the signal type by distribution
	//        to just four frequency bands is not realistic. Signals are distuingished
	//        by their frequency mixture., CHANGE THIS LATER
	return make_pair(abs_strength, n.to_dB());
}



//
// create new objects
//
void game::spawn_ship(ship* s)
{
	ships.push_back(s);
}

void game::spawn_submarine(submarine* u)
{
	submarines.push_back(u);
}

void game::spawn_airplane(airplane* a)
{
	airplanes.push_back(a);
}

void game::spawn_torpedo(torpedo* t)
{
	torpedoes.push_back(t);
}

void game::spawn_gun_shell(gun_shell* s, const double &calibre)
{
	gun_shells.push_back(s);
	// vary the sound effect based on the gun size
	if (calibre <= 120.0)
		events.push_back(new event_gunfire_light(s));
	else if (calibre <= 200.0)
		events.push_back(new event_gunfire_medium(s));
	else
		events.push_back(new event_gunfire_heavy(s));
}

void game::spawn_water_splash(water_splash* s)
{
	water_splashes.push_back(s);
	// events.push_back(new event_splash(s));
}

void game::spawn_depth_charge(depth_charge* dc)
{
	depth_charges.push_back(dc);
	// evaluation of event should be only when player is near enough to hear it...
	events.push_back(new event_depth_charge_in_water(dc));
}

void game::spawn_convoy(convoy* cv)
{
	convoys.push_back(cv);
}

void game::spawn_particle(particle* pt)
{
	// fixme, maybe limit size of particles
	particles.push_back(pt);
}



void game::dc_explosion(const depth_charge& dc)
{
	// Create water splash.
	spawn_water_splash(new depth_charge_water_splash(*this, dc.get_pos().xy().xy0()));
	events.push_back(new event_depth_charge_exploding(&dc));

	// are subs affected?
	// fixme: ships can be damaged by DCs also...
	// fixme: ai should not be able to release dcs with a depth less than 30m or so, to
	// avoid suicide
	for (unsigned k = 0; k < submarines.size(); ++k) {
		submarines[k]->depth_charge_explosion(dc);
	}
}

bool game::gs_impact(const gun_shell *gs)	// fixme: vector2 would be enough
{
	vector3 pos = gs->get_pos();
	
//	return false;//fixme testing
	for (unsigned k = 0; k < ships.size(); ++k) {
		// fixme: we need a special collision detection, because
		// the shell is so fast that it can be collisionfree with *it
		// delta_time ago and now, but hit *it in between
		if (is_collision(ships[k], pos.xy())) {
			// 2006-11-30 doc1972 ships[]->damage() needs an unsigned int
			if (ships[k]->damage(ships[k]->get_pos() /*fixme*/, (unsigned int)gs->damage()))
			{
				ship_sunk(ships[k]);
			} else {
				ships[k]->ignite();
			}		

			events.push_back(new event_shell_explosion(gs));
			
			return true;	// only one hit possible
		}
	}
	for (unsigned k = 0; k < submarines.size(); ++k) {
//printf("sub %f %f %f\n",(*it)->get_pos().x,(*it)->get_pos().y,(*it)->get_pos().z);
//printf("gun %f %f %f\n",pos.x,pos.y,pos.z);
		if (is_collision(submarines[k], pos.xy())) {
//printf("sub damaged!!!\n");		
			// 2006-11-30 doc1972 submarines[]->damage() needs an unsigned int
			submarines[k]->damage(submarines[k]->get_pos() /*fixme*/, (unsigned int)gs->damage());
			
			events.push_back(new event_shell_explosion(gs));
			
			return true; // only one hit possible
		}
	}

	// no ship hit, so must have hit water surface
	if (pos.z <= 0) {
		// Create water splash.
		spawn_water_splash(new gun_shell_water_splash(*this, pos.xy().xy0()));
		events.push_back(new event_shell_splash(gs));
	}
	
	// No impact.
	return false;
}

void game::torp_explode(const torpedo *t)
{
	// each torpedo seems to explode twice, if it's only drawn twice or adds twice the damage is unknown.
	// fixme!
	spawn_water_splash(new torpedo_water_splash(*this, t->get_pos().xy().xy0()));
	events.push_back(new event_torpedo_explosion(t));
}

void game::ship_sunk(const ship* s)
{
	events.push_back(new event_ship_sunk(/*s*/));
	ostringstream oss;
	oss << texts::get(83) << " " << s->get_description ( 2 );
	date d((unsigned)time);
	sunken_ships.push_back(sink_record(d, s->get_description(2), s->get_modelname(), s->get_specfilename(), s->get_skin_layout(), s->get_tonnage()));
}


/*
	fixme: does this function make sense in this place?
	it does:
	- move sensor (could be done in sensor's parent simulate function)
	- stores ping (could be done in a spawn_ping function)
	- detects objects (could be done in a get_asdic_detected_objects(thisping) )
	This function is yet the only "action" function. This concept doesn't seem to match
	with class game or the rest of the simulation.
	maybe ged rid of this (for simplicity of network game this would be useful)
*/
void game::ping_ASDIC ( list<vector3>& contacts, sea_object* d,
	const bool& move_sensor, const angle& dir )
{
	sensor* s = d->get_sensor ( d->active_sonar_system );
	active_sonar_sensor* ass = 0;
	if ( s )
		ass = dynamic_cast<active_sonar_sensor*> ( s );

	if ( ass )
	{
		if ( !move_sensor )
			ass->set_bearing( dir - d->get_heading () );

		// remember ping (for drawing)
		//fixme: seems redundant with event list...!
		pings.push_back ( ping ( d->get_pos ().xy (),
			ass->get_bearing () + d->get_heading (), time,
			ass->get_range (), ass->get_detection_cone () ) );
		events.push_back(new event_ping(d));

		// fixme: noise from ships can disturb ASDIC or may generate more contacs.
		// ocean floor echoes ASDIC etc...
		for (unsigned k = 0; k < submarines.size(); ++k) {
			if ( ass->is_detected ( this, d, submarines[k] ) ) {
				contacts.push_back(submarines[k]->get_pos () +
					vector3 ( rnd ( 40 ) - 20.0f, rnd ( 40 ) - 20.0f,
					rnd ( 40 ) - 20.0f ) );
			}
		}

		if ( move_sensor )
		{
			sensor::sensor_move_mode mode = sensor::sweep;
			// Ships cannot rotate the active sonar sensor because of
			// their screws. A submarine can do so when it is submerged
			// and running on electric engines.
			submarine* sub = dynamic_cast<submarine*> ( d );
			if ( sub && sub->is_submerged() && sub->is_electric_engine() )
				mode = sensor::rotate;
			ass->auto_move_bearing ( mode );
		}
	}
}

void game::register_job(job* j)
{
	jobs.push_back(make_pair(0.0, j));
}

void game::unregister_job(job* j)
{
	for (list<pair<double, job*> >::iterator it = jobs.begin(); it != jobs.end(); ++it) {
		if (it->second == j) {
			delete it->second;
			jobs.erase(it);
			return;
		}
	}
	throw error("[game::unregister_job] job not found in list");
}

template<class C>
ship* game::check_units ( torpedo* t, const ptrset<C>& units )
{
	for (unsigned k = 0; k < units.size(); ++k) {
		if ( is_collision ( t, units[k] ) )
			return units[k];
	}

	return 0;
}

bool game::check_torpedo_hit(torpedo* t, bool runlengthfailure, bool failure)
{
	if (failure) {
		events.push_back(new event_torpedo_dud());
		return true;
	}

	ship* s = check_units ( t, ships );

	if ( !s )
		s = check_units ( t, submarines );

	if ( s ) {
		if (runlengthfailure) {
			events.push_back(new event_torpedo_dud_shortrange());
		} else {
			// Only ships that are alive can be sunk. Already sinking
			// or destroyed ships cannot be destroyed again.
			if (s->is_alive()) {
				if (s->damage(t->get_pos(), t->get_hit_points())) {
					ship_sunk(s);
				} else {
					s->ignite();
				}
			}
			
			//test: explosion:
			spawn_particle(new explosion_particle(s->get_pos() + vector3(0, 0, 5)));
			torp_explode ( t );
		}
		return true;
	}

	return false;
}

sea_object* game::contact_in_direction(const sea_object* o, const angle& direction)
{
	sea_object* result = 0;

	// Try ship first.
	result = ship_in_direction_from_pos ( o, direction );

	// Now submarines.
	if ( !result )
		result = sub_in_direction_from_pos ( o, direction );

	return result;
}

ship* game::ship_in_direction_from_pos(const sea_object* o, const angle& direction)
{
	const sensor* s = o->get_sensor( o->lookout_system );
	const lookout_sensor* ls = 0;
	ship* result = 0;

	if ( s )
		ls = dynamic_cast<const lookout_sensor*> ( s );

	if ( ls )
	{
		double angle_diff = 30;	// fixme: use range also, use ship width's etc.
		for (unsigned k = 0; k < ships.size(); ++k) {
			// Only a visible and intact submarine can be selected.
			if ( ls->is_detected ( this, o, ships[k] ) &&
				( ships[k]->is_alive () ) )
			{
				vector2 df = ships[k]->get_pos().xy () - o->get_pos().xy ();
				double new_ang_diff = (angle(df)).diff(direction);
				if (new_ang_diff < angle_diff)
				{
					angle_diff = new_ang_diff;
					result = ships[k];
				}
			}
		}
	}
	return result;
}

submarine* game::sub_in_direction_from_pos(const sea_object* o, const angle& direction)
{
	const sensor* s = o->get_sensor( o->lookout_system );
	const lookout_sensor* ls = 0;
	submarine* result = 0;

	if ( s )
		ls = dynamic_cast<const lookout_sensor*> ( s );

	if ( ls )
	{
		double angle_diff = 30;	// fixme: use range also, use ship width's etc.
		for (unsigned k = 0; k < submarines.size(); ++k) {
			// Only a visible and intact submarine can be selected.
			if ( ls->is_detected ( this, o, submarines[k] ) &&
				( submarines[k]->is_alive () ) ) {
				vector2 df = submarines[k]->get_pos ().xy () - o->get_pos(). xy();
				double new_ang_diff = (angle(df)).diff(direction);
				if (new_ang_diff < angle_diff)
				{
					angle_diff = new_ang_diff;
					result = submarines[k];
				}
			}
		}
	}
	return result;
}



const torpedo* game::get_torpedo_for_camera_track(unsigned nr) const
{
	if (nr < torpedoes.size() && torpedoes[nr]->is_reference_ok())
		return torpedoes[nr];
	return 0;
}



bool game::is_collision(const sea_object* s1, const sea_object* s2) const
{
	// bounding volume collision test
	float br1 = s1->get_bounding_radius();
	float br2 = s2->get_bounding_radius();
	float br = br1 + br2;
	vector2 p1 = s1->get_pos().xy();
	vector2 p2 = s2->get_pos().xy();
	if (p1.square_distance(p2) > br*br) return false;
	
	// exact collision test
	// compute direction and their normals of both objects
	vector2 d1 = s1->get_heading().direction();
	vector2 n1 = d1.orthogonal();
	vector2 d2 = s2->get_heading().direction();
	vector2 n2 = d2.orthogonal();
	double l1 = s1->get_length(), l2 = s2->get_length();
	double w1 = s1->get_width(), w2 = s2->get_width();

	// base points
	vector2 pb1 = p1 - d1 * (l1/2) - n1 * (w1/2);
	vector2 pb2 = p2 - d2 * (l2/2) - n2 * (w2/2);

	// check if any of obj2 corners is inside obj1
	vector2 pd2[4] = {d2*l2, n2*w2, -d2*l2, -n2*w2};
	vector2 pdiff = pb2 - pb1;
	for (int i = 0; i < 4; ++i) {
		double s = pdiff.x * d1.x + pdiff.y * d1.y;
		if (0 <= s && s <= l1) {
			double t = pdiff.y * d1.x - pdiff.x * d1.y;
			if (0 <= t && t <= w1) {
				return true;
			}
		}
		pdiff += pd2[i];
	}

	// check if any of obj1 corners is inside obj2
	vector2 pd1[4] = {d1*l1, n1*w1, -d1*l1, -n1*w1};
	pdiff = pb1 - pb2;
	for (int i = 0; i < 4; ++i) {
		double s = pdiff.x * d2.x + pdiff.y * d2.y;
		if (0 <= s && s <= l2) {
			double t = pdiff.y * d2.x - pdiff.x * d2.y;
			if (0 <= t && t <= w2) {
				return true;
			}
		}
		pdiff += pd1[i];
	}
	return false;
}

bool game::is_collision(const sea_object* s, const vector2& pos) const
{
	// bounding volume collision test
	float br = s->get_bounding_radius();
	vector2 p = s->get_pos().xy();
	if (p.square_distance(pos) > br*br) return false;
	
	// exact collision test
	// compute direction and their normals
	vector2 d = s->get_heading().direction();
	vector2 n = d.orthogonal();
	double l = s->get_length(), w = s->get_width();

	vector2 pb = p - d * (l/2) - n * (w/2);
	vector2 pdiff = pos - pb;
	double r = pdiff.x * d.x + pdiff.y * d.y;
	if (0 <= r && r <= l) {
		double t = pdiff.y * d.x - pdiff.x * d.y;
		if (0 <= t && t <= w) {
			return true;
		}
	}
	return false;
}

double game::get_depth_factor ( const vector3& sub ) const
{
	return ( 1.0f - 0.5f * sub.z / 400.0f );
}

vector<sea_object*> game::visible_surface_objects(const sea_object* o) const
{
	vector<ship*> vships = visible_ships(o);
	vector<submarine*> vsubmarines = visible_submarines(o);
	vector<airplane*> vairplanes = visible_airplanes(o);

	// fixme: adding RADAR-detected ships to a VISIBLE-objects function is a bit weird...
	// this leads to wrong results if radar detected objects are handled differently,
	// like different display on map, or drawing (not visible!), or for AI!
	vector<ship*> rships = radar_ships(o);
	vector<submarine*> rsubmarines = radar_submarines(o);
	
	vector<sea_object*> result;
	result.reserve(vships.size() + vsubmarines.size() + vairplanes.size() +
		       rships.size() + rsubmarines.size());
	append_vec(result, vships);
	append_vec(result, vsubmarines);
	append_vec(result, vairplanes);
	append_vec(result, rships);
	append_vec(result, rsubmarines);
	return result;
}

vector<sea_object*> game::visible_sea_objects(const sea_object* o) const
{
	vector<ship*> vships = visible_ships(o);
	vector<submarine*> vsubmarines = visible_submarines(o);
	vector<airplane*> vairplanes = visible_airplanes(o);
	vector<torpedo*> vtorpedoes = visible_torpedoes(o);
	vector<sea_object*> result;
	result.reserve(vships.size() + vsubmarines.size() + vairplanes.size() + vtorpedoes.size());
	append_vec(result, vships);
	append_vec(result, vsubmarines);
	append_vec(result, vairplanes);
	append_vec(result, vtorpedoes);
	return result;
}

ship* game::sonar_acoustical_torpedo_target ( const torpedo* o ) const
{
	ship* loudest_object = 0;
	double loudest_object_sf = 0.0f;
	const sensor* s = o->get_sensor ( o->passive_sonar_system );
	const passive_sonar_sensor* pss = 0;

	if ( s )
		pss = dynamic_cast<const passive_sonar_sensor*> ( s );

	if ( pss ) {
		for (unsigned k = 0; k < ships.size(); ++k) {
			double sf = 0.0f;
			if ( pss->is_detected ( sf, this, o, ships[k] ) ) {
				if ( sf > loudest_object_sf ) {
					loudest_object_sf = sf;
					loudest_object = ships[k];
				}
			}
		}

		for (unsigned k = 0; k < submarines.size(); ++k) {
			double sf = 0.0f;
			if ( pss->is_detected ( sf, this, o, submarines[k] ) ) {
				if ( sf > loudest_object_sf ) {
					loudest_object_sf = sf;
					loudest_object = submarines[k];
				}
			}
		}
	}

	return loudest_object;
}



bool game::is_day_mode () const
{
	vector3 sundir;
	double br = compute_light_brightness(player->get_pos(), sundir);
	return (br > 0.3); // fixme: a bit crude. brightness has 0.2 ambient...
}



/*
void game::receive_commands()
{
	// only used for multiplayer games!
	if (networktype > 0) {
		if (servercon) {	// i am client, receive commands from server
			string msg = servercon->receive_message();
			while (msg.length() > 0) {
				if (msg.substr(MSG_length) == MSG_command) {
					string cmd = msg.substr(MSG_length);
					istringstream iss(cmd);
					command* c = command::create(iss, *this);
					c->exec(*this);
					delete c;
				}
				msg = servercon->receive_message();
			}
		} else {		// i am server, receive commands from all clients
			for (vector<network_connection*>::iterator it = clientcons.begin(); it != clientcons.end(); ++it) {
				string msg = (*it)->receive_message();
				while (msg.length() > 0) {
					if (msg.substr(MSG_length) == MSG_command) {
						// fetch it to other clients
						for (vector<network_connection*>::iterator it2 = clientcons.begin(); it2 != clientcons.end(); ++it2) {
							if (it != it2) {
								(*it2)->send_message(msg);
							}
						}
						// execute it locally
						string cmd = msg.substr(MSG_length);
						istringstream iss(cmd);
						command* c = command::create(iss, *this);
						c->exec(*this);
						delete c;
					}
					msg = (*it)->receive_message();
				}
			}
		}
	}
}

void game::send(command* cmd)
{
	// multiplayer?
	if (networktype > 0) {
		// send it over next
		ostringstream osscmd;
		cmd->save(osscmd, *this);
		string msg = string(MSG_command) + osscmd.str();
	
		if (servercon) {	// i am client, send command to the server
			servercon->send_message(msg);
		} else {		// i am server, send command to all clients
			for (vector<network_connection*>::iterator it = clientcons.begin(); it != clientcons.end(); ++it) {
				(*it)->send_message(msg);
			}
		}
	}

	// and execute it locally
	cmd->exec(*this);

	// finally, delete it
	delete cmd;
}
*/


double game::compute_light_brightness(const vector3& viewpos, vector3& sundir) const
{
	// fixme: if sun is blocked by clouds, light must be darker...
	sundir = compute_sun_pos(viewpos).normal();
	// in reality the brightness is equal to sundir.z, but the sun is so bright that
	// we stretch and clamp this value
	double lightbrightness = sundir.z * 2.0;
	if (lightbrightness > 1.0) lightbrightness = 1.0;
	if (lightbrightness < 0.0) lightbrightness = 0.0;
	//fixme add moon light at night
	return lightbrightness * 0.8 + 0.2;	// some ambient value
}



colorf game::compute_light_color(const vector3& viewpos) const
{
	// fixme: sun color can be yellow/orange at dusk/dawn
	// attempt at having some warm variation at light color, previously it was
	// uniform, so we'll try a function of elevation (sundir.z to be precise)
	// Ratios of R, G, B channels are meant to remain in the orange area
	vector3 sundir;
	double lbrit = compute_light_brightness(viewpos, sundir);
	double color_elevation = sundir.z;
	// check for clamping here...
	double lr = lbrit * (1 - pow( cos(color_elevation+.47), 25));
	double lg = lbrit * (1 - pow( cos(color_elevation+.39), 20));
	double lb = lbrit * (1 - pow( cos(color_elevation+.22), 15));
   
	return colorf(lr, lg, lb);
}



/*	************** sun and moon *********************
	The model:
	Sun, moon and earth have an local space, moon and earth rotate around their y-axis.
	y-axes are all up, that means earth's y-axis points to the north pole.
	The moon rotates counter clockwise around the earth in 27 1/3 days (one sidereal month).
	The earth rotates counter clockwise around the sun in 365d 5h 48m 46.5s.
	The earth rotates around itself in 23h 56m 4.1s (one sidereal day).
	Earths rotational axis is rotated by 23.45 degrees.
	Moon orbits in a plane that is 5,15 degress rotated to the xz-plane (plane that
	earth rotates in, sun orbit). The moon is at its southmost position when it is a full moon
	Due to the earth rotation around the sun, the days/months appear longer (the earth
	rotation must compensate the movement).
	So the experienced lengths are 24h for a day and 29.5306 days for a full moon cycle.
	Earth rotational axis points towards the sun at top of summer on the northern hemisphere
	(around 21st. of June).
	On top of summer (northern hemisphere) the earth orbit pos is 0.
	On midnight at longitude 0, the earth rotation is 0.
	At a full moon the moon rotation/orbit position is 0.
	As result the earth takes ~ 366 rotations per year (365d 5h 48m 46.5s / 23h 56m 4.09s = 366.2422)
	We need the exact values/configuration on 1.1.1939, 0:0am.
	And we need the configuration of the moon rotational plane at this date and time.
*/	

const double EARTH_RADIUS = 6.378e6;			// 6378km
const double SUN_RADIUS = 696e6;			// 696.000km
const double MOON_RADIUS = 1.738e6;			// 1738km
const double EARTH_SUN_DISTANCE = 149600e6;		// 149.6 million km.
const double MOON_EARTH_DISTANCE = 384.4e6;		// 384.000km
const double EARTH_ROT_AXIS_ANGLE = 23.45;		// degrees.
const double MOON_ORBIT_TIME_SIDEREAL = 27.3333333 * 86400.0;	// sidereal month is 27 1/3 days
const double MOON_ORBIT_TIME_SYNODIC = 29.5306 * 86400.0;	// synodic month is 29.5306 days
//more precise values:
//29.53058867
//new moon was on 18/11/1998 9:36:00 pm
const double MOON_ORBIT_AXIS_ANGLE = 5.15;		// degrees
const double EARTH_ROTATION_TIME = 86164.09;		// 23h56m4.09s, one sidereal day!
const double EARTH_PERIMETER = 2.0 * M_PI * EARTH_RADIUS;
const double EARTH_ORBIT_TIME = 31556926.5;		// in seconds. 365 days, 5 hours, 48 minutes, 46.5 seconds

const double MOON_POS_ADJUST = 300.0;	// in degrees. Moon pos in its orbit on 1.1.1939 fixme: research the value

/*
what has to be fixed for sun/earth/moon simulation:
get exact distances and diameters (done)
get exact rotation times (sidereal day, not solar day) for earth and moon (done)
get exact orbit times for earth and moon around sun / earth (done)
get angle of rotational axes for earth and moon (fixme, 23.45 and 5.15) (done)
get direction of rotation for earth and moon relative to each other (done)
get position of objects and axis states for a fix date (optimum 1.1.1939) (!only moon needed, fixme!)
compute formulas for determining the positions for the following years (fixme)
write code that computes sun/moon pos relative to earth and relative to local coordinates (fixme)
draw moon with phases (fixme)
*/

vector3 game::compute_sun_pos(const vector3& viewpos) const
{
	double yearang = 360.0*myfrac((time+10*86400)/EARTH_ORBIT_TIME);
	double dayang = 360.0*(viewpos.x/EARTH_PERIMETER + myfrac(time/86400.0));
	double longang = 360.0*viewpos.y/EARTH_PERIMETER;
	matrix4 sun2earth =
		matrix4::rot_y(-90.0) *
		matrix4::rot_z(-longang) *
		matrix4::rot_y(-(yearang + dayang)) *
		matrix4::rot_z(EARTH_ROT_AXIS_ANGLE) *
		matrix4::rot_y(yearang) *
		matrix4::trans(-EARTH_SUN_DISTANCE, 0, 0) *
		matrix4::rot_y(-yearang);
	return sun2earth.column3(3);
}



vector3 game::compute_moon_pos(const vector3& viewpos) const
{
	double yearang = 360.0*myfrac((time+10*86400)/EARTH_ORBIT_TIME);
	double dayang = 360.0*(viewpos.x/EARTH_PERIMETER + myfrac(time/86400.0));
	double longang = 360.0*viewpos.y/EARTH_PERIMETER;
	double monthang = 360.0*myfrac(time/MOON_ORBIT_TIME_SYNODIC) + MOON_POS_ADJUST;

	matrix4 moon2earth =
		matrix4::rot_y(-90.0) * 
		matrix4::rot_z(-longang) *
		matrix4::rot_y(-(yearang + dayang)) *
		matrix4::rot_z(EARTH_ROT_AXIS_ANGLE) *
		matrix4::rot_y(yearang) *
		matrix4::rot_z(-MOON_ORBIT_AXIS_ANGLE) *
		matrix4::rot_y(monthang + MOON_POS_ADJUST) *
		matrix4::trans(MOON_EARTH_DISTANCE, 0, 0);

	return moon2earth.column3(3);
}



double game::compute_water_height(const vector2& pos) const
{
	return mywater->get_height(pos);
}



sea_object* game::load_ptr(unsigned nr) const
{
	if (nr == 0)
		return 0;
	if (nr <= submarines.size()) {
		return submarines[nr-1];
	} else if (nr <= submarines.size() + ships.size()) {
		return ships[nr-1 - submarines.size()];
	} else if (nr <= submarines.size() + ships.size() + airplanes.size()) {
		return airplanes[nr-1 - submarines.size() - ships.size()];
	} else {
		throw error("could not translate nr to submarine, ship or airplane ptr");
	}
}



ship* game::load_ship_ptr(unsigned nr) const
{
	if (nr > submarines.size() && nr <= submarines.size() + ships.size()) {
		return ships[nr-1 - submarines.size()];
	}
	throw error("could not translate nr to ship ptr");
}



convoy* game::load_convoy_ptr(unsigned nr) const
{
	if (nr == 0)
		return 0;
	if (nr > convoys.size())
		throw error("could not translate nr to convoy ptr");
	return convoys[nr-1];
}



unsigned game::save_ptr(const sea_object* s) const
{
	if (!s)
		return 0;
	// we can save only airplanes, ships, submarines. Because other sea_objects can't be referenced!
	const submarine* su = dynamic_cast<const submarine*>(s);
	if (su) {
		for (unsigned k = 0; k < submarines.size(); ++k) {
			if (submarines[k] == su)
				return k+1;
		}
		throw error("could not translate ptr to submarine nr");
	}
	const ship* sh = dynamic_cast<const ship*>(s);
	if (sh) {
		for (unsigned k = 0; k < ships.size(); ++k) {
			if (ships[k] == sh)
				return submarines.size() + k+1;
		}
		throw error("could not translate ptr to ship nr");
	}
	const airplane* ap = dynamic_cast<const airplane*>(s);
	if (ap) {
		for (unsigned k = 0; k < airplanes.size(); ++k) {
			if (airplanes[k] == ap)
				return submarines.size() + ships.size() + k+1;
		}
		throw error("could not translate ptr to airplane nr");
	}
	throw error("could not translate ptr to submarine, ship or airplane");
}



unsigned game::save_ptr(const convoy* c) const
{
	if (!c)
		return 0;
	for (unsigned k = 0; k < convoys.size(); ++k) {
		if (convoys[k] == c)
			return k+1;
	}
	throw error("could not translate convoy ptr to nr");
}



// function is not used yet.
// give relative position, length*vis, width*vis and course
bool is_in_ellipse(const vector2& p, double xl, double yl, angle& head)
{
	vector2 hd = head.direction();
	double t1 = (p.x*hd.x + p.y*hd.y);
	double t2 = (p.y*hd.x - p.x*hd.y);
	return ((t1*t1)/(xl*xl) + (t2*t2)/(yl*yl)) < 1;
}



void game::freeze_time()
{
	if (freezetime_start > 0)
		throw error("freeze_time() called twice!");
	freezetime_start = sys().millisec();
//	printf("time frozen at: %u\n", freezetime_start);
}



void game::unfreeze_time()
{
	unsigned freezetime_end = sys().millisec();
//	printf("time UNfrozen at: %u (%u)\n", freezetime_end, freezetime_end - freezetime_start);
	freezetime += freezetime_end - freezetime_start;
	freezetime_start = 0;
}
