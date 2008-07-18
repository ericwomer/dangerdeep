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

// sea objects
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "sea_object.h"
#include "vector2.h"
#include "sensors.h"
#include "model.h"
#include "global_data.h"
#include "system.h"
#include "texts.h"
#include "ai.h"
#include "game.h"
#include "datadirs.h"
#include "global_constants.h"
#include "log.h"
using std::string;
using std::list;



void sea_object::degrees2meters(bool west, unsigned degx, unsigned minx, bool south,
	unsigned degy, unsigned miny, double& x, double& y)
{
	x = (west ? -1.0 : 1.0)*(double(degx)+double(minx)/60.0)*EARTH_PERIMETER/360.0;
	y = (south ? -1.0 : 1.0)*(double(degy)+double(miny)/60.0)*EARTH_PERIMETER/360.0;
}



void sea_object::meters2degrees(double x, double y, bool& west, unsigned& degx, unsigned& minx, bool& south,
	unsigned& degy, unsigned& miny)
{
	double fracdegrx = fabs(x*360.0/EARTH_PERIMETER);
	double fracdegry = fabs(y*360.0/EARTH_PERIMETER);
	degx = unsigned(floor(fracdegrx));
	degy = unsigned(floor(fracdegry));
	minx = unsigned(60.0 * myfrac(fracdegrx) + 0.5);
	miny = unsigned(60.0 * myfrac(fracdegry) + 0.5);
	west = (x < 0.0);
	south = (y < 0.0);
}



void sea_object::compute_force_and_torque(vector3& F, vector3& T) const
{
	// force is in world space!
	/* general formulas:
	   Total force acting on a body is just the sum of all forces acting on it.
	   Total torque is the sum over all forces with index i, with summands
	   (p_i - x) cross F_i.
	   F_i is the force.
	   p_i is the point in 3-space where F_i acts.
	   x is the gravitational center of the body.
	   Hence: total torque = sum xr_i cross F_i  (xr : relative position).
	   Torque is a vector, that has a direction and a length.
	   The direction is the axis around that the torque/force acts,
	   and the length is proportional to the amount of the torque.
	   In our current model the length would be proportional to the turn acceleration.
	*/
	F.z = -GRAVITY * mass;
}



void sea_object::compute_helper_values()
{
	velocity = linear_momentum * mass_inv;
	local_velocity = orientation.conj().rotate(velocity);

	heading = angle(orientation.rotate(0.0, 1.0, 0.0).xy());
	// w is _old_ spin vector, but we need the new one...
	// does it make a large difference?
	// |w| is revolutions per time, thus 2*Pi/second for |w|=1.
	// we have to multiply it by 360/(2*Pi) to get angles per second.
	// hmmm, w2.length is speed, but sign of it depends on direction of w!!!!
	// if the ship turns right (clockwise), turn_velocity should be positive?
	// in that case, w is pointing downwards.
	//DBGOUT6(velocity,local_velocity,heading.value(),turn_velocity,orientation,orientation.length());

	// unit of |w| is revolutions per time, that is 2*Pi/second.
	// Note! here w is local. Get global w by rotating it with orientation.rotate(w)
	vector3 w = inertia_tensor_inv * orientation.conj().rotate(angular_momentum);
	// turn velocity around z-axis is projection of w to z-axis, that is
	// simply w.z. Transform to angles per second. same for x/y.
	turn_velocity = w.z * (180.0/M_PI);	// could also be named yaw_velocity.
	pitch_velocity = w.x * (180.0/M_PI);
	roll_velocity = w.y * (180.0/M_PI);
	//std::cout << "velocities(deg) turn=" << turn_velocity << " pitch=" << pitch_velocity << " roll=" << roll_velocity << "\n";
}



/*
  fixme: later switch to model that uses force and torque.
  we need an inertia tensor for that model.

  compute torque of sub/ship to simulate moving in the waves:

  compute force at bow/stern/mid/left side/right side or some other important points.
  like where the trim tanks of subs are.

  the torque is computed as

  M = (r1 - r0) x F1, where F1 is the force acting at point r1, where r0 is center of gravity
  do r1-r0 is the vector from the object's center to the point where F1 is acting.
  M is a vector definining axis of rotation (direction of M) and strength of rotation (length of M).
  To compute total torque, sum all M over i: M_total = Sum_i (r_i - r0) x F_i

  To compute translational forces, just sum up all forces F: F_total = Sum_i F_i.

  Problem: orientation is stored as quaternion, not as three angles.
  torque changes angular velocity over time, and that changes orientation over time.
  The axes of torque or angular velocity don't need to be identical!
  That is the problem.
  Given torque, angular velocity and orientation as quaternions q_t, q_v and q_r we have
  q_v' = q_v * (q_t * delta_t)
  q_r' = q_r * (q_v * delta_t)
  But computing q * z, where q is a quaternion for rotation (unit quaternion) and z is a number
  is not that easy. q represents a rotation around an axis x and and angle w.
  q * z would then be the rotation around the same axis, but with angle w*z.
  A rotation quaternion for axis x and angle w is given as:
  (cos(w/2), sin(w/2)*x)
  Thus q*z would be
  (cos(w*z/2), sin(w*z/2)*)
  how to compute that from q? we would need an acos function call, which is very costly...
  One could also see cos(w/2),sin(w/2) as complex number. It has an absolute value of 1.
  So multiplying the angle with n would be the same as taking the n'th power of that number.
  But this computation would also be very costly - it is the same problem as we have here.
  Alternativly: Represent angular velocity and torque around three fix axes.
  Store axis and angle for each part, like x/y/z-axis and the three angles.
  Changing angular velocity by torque is easy then, orientation could still be stored as quaternion.

  Computing orientation forced by the waves:
  Compute buoyancy on points around the ship. Draught gives weight of displaced water,
  the difference of the ships weight (or the weight of that part for which the buoyancy
  is computed) gives a force, that is applied to the ship.
  Force = mass * acceleration. Acceleration is g, mass is difference between displaced water
  and the ship's part (is that correct for computation of buoyancy?).


  is this comment up-to-date? rather not. check and remove, fixme...
*/



void sea_object::set_sensor ( sensor_system ss, sensor* s )
{
	if ( ss >= 0 && ss < last_sensor_system ){
		sensors.reset(size_t(ss), s);
	}
}



double sea_object::get_cross_section ( const vector2& d ) const
{
	if (mymodel) {
		vector2 r = get_pos().xy() - d;
		angle diff = angle(r) - get_heading();
		return mymodel->get_cross_section(diff.value());
	}
	return 0.0;


}



string sea_object::compute_skin_name() const
{
	for (list<skin_variant>::const_iterator it = skin_variants.begin();
	     it != skin_variants.end(); ++it) {
		// check date
		if (skin_date < it->from || skin_date > it->until) {
			continue;
		}
		// iterate over regioncodes
		if (it->regions.size() > 0) {
			// if any regions are given (otherwise all match)
			bool match = false;
			for (list<string>::const_iterator it2 = it->regions.begin();
			     it2 != it->regions.end(); ++it2) {
				if (skin_regioncode == *it2) {
					match = true;
					break;
				}
			}
			if (!match) {
				continue;
			}
		}
		// iterate over countrycodes
		if (it->countries.size() > 0) {
			// if any countries are given (otherwise all match)
			bool match = false;
			for (list<string>::const_iterator it2 = it->countries.begin();
			     it2 != it->countries.end(); ++it2) {
				if (*it2 == string(countrycodes[skin_country])) {
					match = true;
					break;
				}
			}
			if (!match) {
				continue;
			}
		}
		// we found a match!
		return it->name;
	}
	return model::default_layout;
}



void sea_object::set_random_skin_name(const date& d)
{
	unsigned nr_possible = 0;
	for (list<skin_variant>::const_iterator it = skin_variants.begin();
	     it != skin_variants.end(); ++it) {
		if (d >= it->from && d <= it->until)
			++nr_possible;
	}
	if (nr_possible == 0) {
		log_debug("Could not chose valid skin, using anyone");
		nr_possible = skin_variants.size();
		if (nr_possible == 0) {
			set_skin_layout(model::default_layout);
			return; // can't set anything, shouldn't happen
		}
	}
	unsigned chosen = rnd(nr_possible);
	for (list<skin_variant>::const_iterator it = skin_variants.begin();
	     it != skin_variants.end(); ++it) {
		if (d >= it->from && d <= it->until) {
			if (chosen == 0) {
				skin_date = d;
				skin_regioncode = "NN";
				skin_country = UNKNOWNCOUNTRY;
				log_debug("using skin name " << it->name << " as random skin");
				set_skin_layout(it->name);
				return;
			}
			--chosen;
		}
	}
	// set default layout if everything else fails
	set_skin_layout(model::default_layout);
}



void sea_object::set_skin_layout(const std::string& layout)
{
//	cout << "set skin layout = '" << layout << "'\n";
	if (layout != skin_name) {
		if (mymodel) {
			if (skin_name.length() > 0)
				mymodel->unregister_layout(skin_name);
			mymodel->register_layout(layout);
		}
		skin_name = layout;
	}
}



sea_object::sea_object(game& gm_, const string& modelname_)
	: gm(gm_),
	  modelname(modelname_),
	  mymodel(0),
	  skin_country(UNKNOWNCOUNTRY),
	  mass(1.0),//fixme
	  mass_inv(1.0/mass),
	  turn_velocity(0),
	  pitch_velocity(0),
	  roll_velocity(0),
	  alive_stat(alive),
	  sensors(last_sensor_system),
	  target(0),
	  invulnerable(false), country(UNKNOWNCOUNTRY), party(UNKNOWNPARTY),
	  redetect_time(0)
{
	// no specfile, so specfilename is empty, do not call get_rel_path with empty string!
	mymodel = modelcache().ref(/*data_file().get_rel_path(specfilename) + */ modelname);
	// this constructor is used only for simple models that have no skin support,
	// so register default layout.
	skin_name = model::default_layout;
	mymodel->register_layout(skin_name);
//  	cout << "base c'tor: registered layout " << skin_name << "\n";
	size3d = vector3f(mymodel->get_width(), mymodel->get_length(), mymodel->get_height());

	// this constructor is used for simple models like depth charges and grenades.
	// we use the displacement value as mass plus some extra part to make
	// them sink.
	// later we should give spec files for every object and then erase this
	// constructor.
	mass = mymodel->get_base_mesh().volume * 1100;
	mass_inv = 1.0/mass;
	inertia_tensor = mymodel->get_base_mesh().inertia_tensor * mass;
	inertia_tensor_inv = inertia_tensor.inverse();
}



sea_object::sea_object(game& gm_, const xml_elem& parent)
	: gm(gm_),
	  mymodel(0),
	  skin_country(UNKNOWNCOUNTRY),
	  mass(1.0),//fixme
	  mass_inv(1.0/mass),
	  turn_velocity(0),
	  pitch_velocity(0),
	  roll_velocity(0),
	  alive_stat(alive),
	  sensors(last_sensor_system),
	  target(0),
	  invulnerable(false), country(UNKNOWNCOUNTRY), party(UNKNOWNPARTY),
	  redetect_time(0)
{
	xml_elem cl = parent.child("classification");
	specfilename = cl.attr("identifier");
	modelname = cl.attr("modelname");

	// read skin data
	for (xml_elem::iterator it = cl.iterate("skin"); !it.end(); it.next()) {
		skin_variant sv;
		sv.name = it.elem().attr("name");
		if (it.elem().has_attr("regions")) {
			// empty list means all/any...
			sv.regions = string_split(it.elem().attr("regions"));
		}
		if (it.elem().has_attr("countries")) {
			// empty list means all/any...
			sv.countries = string_split(it.elem().attr("countries"));
		}
		if (it.elem().has_attr("from")) {
			sv.from = date(it.elem().attr("from"));
		} else {
			sv.from = date(1939, 1, 1);
		}
		if (it.elem().has_attr("until")) {
			sv.until = date(it.elem().attr("until"));
		} else {
			sv.until = date(1945, 12, 31);
		}
		skin_variants.push_back(sv);
// 		cout << "read skin variant: " << sv.name << " ctr=" << sv.countries
// 		     << " rgn=" << sv.regions << " from=" << sv.from << " until=" << sv.until
// 		     << "\n";
	}

	mymodel = modelcache().ref(data_file().get_rel_path(specfilename) + modelname);
	size3d = vector3f(mymodel->get_width(), mymodel->get_length(), mymodel->get_height());

	// read mass from spec file
	if (parent.has_child("mass")) {
	mass = parent.child("mass").attrf();
	} else {
		log_warning("no <mass> tag in file " << parent.doc_name());
		mass = mymodel->get_base_mesh().volume * 500; // assume 0,5tons per cubic meter as crude guess.
	}
	mass_inv = 1.0/mass;
	inertia_tensor = mymodel->get_base_mesh().inertia_tensor * mass;
	inertia_tensor_inv = inertia_tensor.inverse();

	string countrystr = cl.attr("country");
	country = UNKNOWNCOUNTRY;
	party = UNKNOWNPARTY;
	for (unsigned i = 0; i < NR_OF_COUNTRIES; ++i) {
		if (countrystr == countrycodes[i]) {
			country = countrycode(i);
			party = party_of_country(country, gm.get_date());
		}
	}
	xml_elem ds = parent.child("description");
	for (xml_elem::iterator it = ds.iterate("far"); !it.end(); it.next()) {
		if (it.elem().attr("lang") == texts::get_language_code()) {
			descr_far = it.elem().child_text();
		}
	}
	for (xml_elem::iterator it = ds.iterate("medium"); !it.end(); it.next()) {
		if (it.elem().attr("lang") == texts::get_language_code()) {
			descr_medium = it.elem().child_text();
		}
	}
	for (xml_elem::iterator it = ds.iterate("near"); !it.end(); it.next()) {
		if (it.elem().attr("lang") == texts::get_language_code()) {
			descr_near = it.elem().child_text();
		}
	}
	xml_elem sn = parent.child("sensors");
	for (xml_elem::iterator it = sn.iterate("sensor"); !it.end(); it.next()) {
		string typestr = it.elem().attr("type");
		if (typestr == "lookout") set_sensor(lookout_system, new lookout_sensor());
		else if (typestr == "passivesonar") set_sensor(passive_sonar_system, new passive_sonar_sensor());
		else if (typestr == "activesonar") set_sensor(active_sonar_system, new active_sonar_sensor());
		else if (typestr == "radar") {
			radar_sensor::radar_type type = radar_sensor::radar_type_default;
			string radar_model = it.elem().attr("model");
			if ("British Type 271" == radar_model)
				type = radar_sensor::radar_british_type_271;
			else if ("British Type 272" == radar_model)
				type = radar_sensor::radar_british_type_272;
			else if ("British Type 273" == radar_model)
				type = radar_sensor::radar_british_type_273;
			else if ("British Type 277" == radar_model)
				type = radar_sensor::radar_british_type_277;
			else if ("German FuMO 29" == radar_model)
				type  = radar_sensor::radar_german_fumo_29;
			else if ("German FuMO 30" == radar_model)
				type  = radar_sensor::radar_german_fumo_30;
			else if ("German FuMO 61" == radar_model)
				type  = radar_sensor::radar_german_fumo_61;
			else if ("German FuMO 64" == radar_model)
				type  = radar_sensor::radar_german_fumo_64;
			else if ("German FuMO 391" == radar_model)
				type  = radar_sensor::radar_german_fumo_391;
			else
				throw error("invalid radar type name");
					
			set_sensor(radar_system, new radar_sensor(type));
		}
		// ignore unknown sensors.
	}

	// ai is filled in by heirs.
}



sea_object::~sea_object()
{
// 	cout << "d'tor: unregistered layout " << skin_name << "\n";
	if (skin_name.length() > 0)
		mymodel->unregister_layout(skin_name);
	modelcache().unref(mymodel);
}



void sea_object::load(const xml_elem& parent)
{
	string specfilename2 = parent.attr("type");	// checks
	// this checks if the filename of the specfile matches the internal id string...
	if (specfilename != specfilename2)
		throw error(string("stored specfilename does not match, type=") + specfilename2
			    + string(", but read ") + specfilename + string(" from spec file"));
	xml_elem st = parent.child("state");
	position = st.child("position").attrv3();
	orientation = st.child("orientation").attrq();
	linear_momentum = st.child("linear_momentum").attrv3();
	angular_momentum = st.child("angular_momentum").attrv3();
	compute_helper_values();

	// read skin info
	if (parent.has_child("skin")) {
		// read attributes
		xml_elem sk = parent.child("skin");
		skin_regioncode = sk.attr("region");
		std::string sc = sk.attr("country");
		skin_country = UNKNOWNCOUNTRY;
		for (int i = UNKNOWNCOUNTRY; i < NR_OF_COUNTRIES; ++i) {
//			cout << "load cmp ctr '" << sc << "' '" << string(countrycodes[i]) << "'\n";
			if (sc == string(countrycodes[i])) {
				skin_country = countrycode(i);
				break;
			}
		}
		skin_date = date(sk.attr("date"));
	} else {
		// set default skin values
		skin_regioncode = "NA";	// north atlantic
		skin_country = UNKNOWNCOUNTRY;
		skin_date = date(1941, 1, 1);
	}
	skin_name = compute_skin_name();
	// register new skin name. Note! if skin_name was already set and registered,
	// the old one is not unregistered. But we don't use sea_object in that way.
	// So everything is ok.
	mymodel->register_layout(skin_name);
//  	cout << "load: registered layout " << skin_name << "\n";
	// load ai
	if (myai.get()) {
		myai->load(gm, parent.child("AI"));
	}
	// load target
	target = gm.load_ptr(parent.child("target").attru());
}



void sea_object::save(xml_elem& parent) const
{
	// specfilename is requested and stored by game or callers of this function
	xml_elem st = parent.add_child("state");
	st.add_child("position").set_attr(position);
	st.add_child("orientation").set_attr(orientation);
	st.add_child("linear_momentum").set_attr(linear_momentum);
	st.add_child("angular_momentum").set_attr(angular_momentum);
	parent.add_child("alive_stat").set_attr(unsigned(alive_stat));
	// write skin info
	xml_elem sk = parent.add_child("skin");
	sk.set_attr(skin_regioncode, "region");
	sk.set_attr(countrycodes[skin_country], "country");
	sk.set_attr(skin_date.to_str(), "date");
	// save ai
	if (myai.get()) {
		xml_elem ae = parent.add_child("AI");
		myai->save(gm, ae);
	}
	// save target
	parent.add_child("target").set_attr(gm.save_ptr(target));
}



string sea_object::get_description(unsigned detail) const
{
	if (detail == 0) return descr_far;
	else if (detail == 1) return descr_medium;
	else return descr_near;
}



void sea_object::simulate(double delta_time)
{
	// check and change states
	if (alive_stat == defunct) {
		// with that trick we do not need to handle this situation in every heir of sea_object.
		throw is_dead_exception();
	} else if (alive_stat == dead2) {
		// change state to defunct.
		alive_stat = defunct;
		throw is_dead_exception();
	} else if (alive_stat == dead) {
		// change state to dead2.
		alive_stat = dead2;
		throw is_dead_exception();
	}

	// check target. heirs should check for "out of range" condition too
	if (target && !target->is_alive())
		target = 0;

	// check if list of detected objects needs to be compressed.
	// needs to be called for every frame and object, because objects can become defunct every frame
	// and must then get removed from the list.
	compress(visible_objects);
	compress(radar_objects);

	// check for redection jobs and eventually (re)create list of detected objects
	if (detect_other_sea_objects()) {
		redetect_time -= delta_time;
		if (redetect_time <= 0) {
			//fixme: doing this for every object leads to N^2 sensor tests per second.
			//until now this was done for the player only leading to N*fps tests per second, which is
			//more for a small number of objects, but less than for greater N (> 30).
			//this can be a problem, but even with 120 ships we have 14400 sensor checks per second only
			//and this is not much. On the other hand we need to check for each ship if it can see
			//other ships to make the AI work correctly (like collision avoidance).
			//We can decrease the needed cpu usage easily by increasing the redection cycles to 5 seconds
			//or so, which is still realistic.
			visible_objects = gm.visible_sea_objects(this);
			radar_objects = gm.radar_sea_objects(this);
			sonar_objects = gm.sonar_sea_objects(this);
			redetect_time = 1.0;	// fixme: maybe make it variable, depending on the object type.
		}
	}

	// get force and torque for current time.
	vector3 force, torque;
	compute_force_and_torque(force, torque);
	//DBGOUT6(position, orientation, linear_momentum, angular_momentum, force, torque);

	// compute new position by integrating linear_momentum
	// M^-1 * P = v, linear_momentum is in world space!
	position += linear_momentum * mass_inv * delta_time;

	// compute new linear_momentum by integrating force
	//fixme: linear_momentum was object_local, so direction of linear_momentum
	//was changed automatically when orientation changed. This is no longer
	//the case. Normal objects still are simulated correctly, but this is
	//missing some physics laws? - a fast moving plane going into a curve
	//will change its linear momentum for example, because direction is
	//changed without new force (e.g. if motor is off, like sailing plane).
	//test show that this still works with subs: accelerate to flank speed,
	//then turn off engine, then turn rudder -> sub will turn!
	//the drag may slow the sub down, but why it is moving forward in new
	//direction?! drag should slow sub down and does so, but nothing
	//accelerates it!!
	linear_momentum += delta_time * force;

	// compute new orientation by integrating angular momentum
	// L = I * w = R * I_k * R^T * w =>
	// w = I^-1 * L = R * I_k^-1 * R^-1 * L
	// so we can compute w from I_k^-1 and L.
	// with some math we have:
	// w = R * (I_k^-1 * (R^-1 * L))
	//                   ^^^^^^^^^^
	//                      vector
	//         ^^^^^^^^^^^^^^^^^^^^
	//               vector
	// Thus we don't need the matrix R but can use
	// the orientation quaternion directly for rotation.
	// This is much more efficient.
	// with w we can update orientation.
	// w codes the axis/angle, which needs to get multiplied by delta_t,
	// so we can just compute w' = w * delta_t and then compute a
	// rotation quaternion from w' and set new orientation as
	// produkt of old orientation and w'.
// 	std::cout << "torque=" << torque << " angular momentum=" << angular_momentum << "\n";
// 	std::cout << "compute w, angular_momentum=" << angular_momentum << " inertiainv:\n";
	vector3 w = orientation.rotate(inertia_tensor_inv * orientation.conj().rotate(angular_momentum));
	vector3 w2 = w * delta_time;
// 	std::cout << "update orientation, dt=" << delta_time << " w=" << w << " w2=" << w2 << "\n";
	// unit of |w| is revolutions per time, that is 2*Pi/second.
	double w2l = w2.length();
	if (w2l > 1e-8) {
		// avoid too small numbers
		quaternion q = quaternion::rot_rad(w2l, w2 * (1.0/w2l));
		// multiply orientation with q: combined rotation.
// 		std::cout << "q=" << q << " orientation old=" << orientation << " new=" << q * orientation << "\n";
		orientation = q * orientation;
		// we should renormalize orientation regularly, to avoid that
		// orientation isn't a valid rotation after many changes.
		if (fabs(orientation.square_length() - 1.0) > 1e-8) {
			orientation.normalize();
		}
	}

	// compute new angular momentum by integrating torque (both in world space)
	angular_momentum += delta_time * torque;

	// update helper variables
	compute_helper_values();

	// OLD COMMENT, BUT STILL HELPFUL:
	// this leads to another model for acceleration/max_speed/turning etc.
	// the object applies force to the screws etc. with Force = acceleration * mass.
	// there is some drag caused by air/water opposite to the force.
	// this drag damps the speed curve so that acceleration is zero at speed==max_speed.
	// drag depends on speed (v or v^2).
	// v = v0 + a * delta_t, v <= v_max, a = engine_force/mass - drag
	// now we have: drag(v) = max_accel = engine_force/mass.
	// and: v=v_max, hence: drag(v_max) = max_accel, max_accel is given, v_max too.
	// so choose a drag formula: factor*v or factor*v^2 and compute factor.
	// we have: factor*v_max^2 = max_accel => factor = max_accel/v_max^2
	// finally: v = v0 + delta_t * (max_accel - dragfactor * v0^2)
	// if v0 == 0 then we have maximum acceleration.
	// acceleration lowers quadratically until we have maximum velocity.
	// we also have side drag (limit turning speed!):

	// compute force for some velocity v: find accel so that accel - dragfactor * v^2 = 0
	// hence: accel = dragfactor * v^2, this means force is proportional to square
	// of speed -> fuel comsumption depends ~quadratically on speed.
	// To throttle to a given speed, apply max_accel until we have it then apply accel.
	// In reality engine throttle could translate directly to acceleration, this means
	// with 1/2 throttle we just use constant acceleration of 1/2*max_accel.
	// This leads to a maximum speed determined by drag, but time until that
	// speed is reached is much longer compared to using max_accel.

	// drag: drag force is -b * v (low speeds), -b * v^2 (high speeds)
	// b is proportional to cross section area of body.

	// more difficult: change acceleration to match a certain position (or angle)
	// s = s0 + v0 * t + a/2 * t^2, v = v0 + a * t
	// compute a over time so that s_final = s_desired and v_final = 0, with arbitrary s0,v0.
	// a can be 0<=a<=max_accel. three phases (in time) speed grows until max_speed,
	// constant max speed, speed shrinks to zero (sometimes only phases 1 and 3 needed).
	// determine phase, in phase 1 and 2 give max. acceleration, in phase 3 give zero.
	// or even give -max_accel is phase 3 (turn rudder to opposite with full throttle
	// to stop motion fastly)

	// Screw force splits in forward force and sideward force (dependend on rudder position)
	// so compute side drag from turn_rate
	// compute turn_rate to maximum angular velocity, limited by ship's draught and shape.

	// we store velocity/acceleration, not momentum/force/torque
	// in reality applied force is transformed by inertia tensor, which is rotated according to
	// orientation. we don't use mass here, so apply rotation to stored velocity/acceleration.
	//fixme: use orientation here and compute heading from it, not vice versa!
}



bool sea_object::damage(const vector3& fromwhere, unsigned strength)
{
	kill();	// fixme crude hack, replace by damage simulation
	return true;
}



unsigned sea_object::calc_damage() const
{
	return is_dead() ? 100 : 0;
}



void sea_object::set_inactive()
{
	if (alive_stat == defunct)
		throw error("illegal alive_stat switch (defunct to inactive)");
	if (alive_stat == dead)
		throw error("illegal alive_stat switch (dead to inactive)");
	alive_stat = inactive;
}



void sea_object::kill()
{
	if (alive_stat == defunct)
		throw error("illegal alive_stat switch (defunct to inactive)");
	alive_stat = dead;
}



float sea_object::surface_visibility(const vector2& watcher) const
{
	return get_cross_section(watcher);
}



void sea_object::manipulate_position(const vector3& newpos)
{
	position = newpos;
}



void sea_object::manipulate_speed(double localforwardspeed)
{
	local_velocity.y = localforwardspeed;
	linear_momentum = orientation.rotate(local_velocity * mass);
	compute_helper_values();
}



void sea_object::manipulate_heading(angle hdg)
{
	orientation = quaternion::rot(-hdg.value(), 0, 0, 1);
	linear_momentum = orientation.rotate(local_velocity) * mass;
	compute_helper_values();
}



//fixme: should move to ship or maybe return pos. airplanes have engines, but not
//dc's/shells.
vector2 sea_object::get_engine_noise_source () const
{
	return get_pos ().xy () - get_heading().direction () * 0.3f * get_length();
}



void sea_object::display(const texture *caustic_map) const
{
	if (mymodel) {
//		cout << "render with skin layout = " << skin_name << "\n";
		mymodel->set_layout(skin_name);
		mymodel->display(caustic_map);
	}
}



void sea_object::display_mirror_clip() const
{
	if (mymodel) {
//		cout << "renderMC with skin layout = " << skin_name << "\n";
		mymodel->set_layout(skin_name);
		mymodel->display_mirror_clip();
	}
}



sensor* sea_object::get_sensor ( sensor_system ss )
{
	if ( ss >= 0 && ss < last_sensor_system )
		return sensors[ss];

	return 0;
}



const sensor* sea_object::get_sensor ( sensor_system ss ) const
{
	if ( ss >= 0 && ss < last_sensor_system )
		return sensors[ss];

	return 0;
}



void sea_object::compress(std::vector<sea_object*>& vec)
{
	// this algorithm keeps the order of objects.
	unsigned j = 0;
	for (unsigned i = 0; i < vec.size(); ++i) {
		if (vec[i]->is_reference_ok()) {
			sea_object* tmp = vec[i];
			vec[i] = 0;
			vec[j] = tmp;
			++j;
		}
	}
	vec.resize(j);
}



void sea_object::compress(std::list<sea_object*>& lst)
{
	for (std::list<sea_object*>::iterator it = lst.begin(); it != lst.end(); ) {
		if ((*it)->is_reference_ok()) {
			++it;
		} else {
			it = lst.erase(it);
		}
	}
}



model& sea_object::get_model() const
{
	if (mymodel)
		return *mymodel;
	throw error("sea_object::get_model(), no model set");
}



std::vector<polygon> sea_object::get_bounding_box(bool inverse) const
{
	std::vector<polygon> bbox;
	bbox.reserve(6);
	vector3f pmin = get_model().get_min();
	vector3f pmax = get_model().get_max();
	// order:
	// bottom layer:
	// 2--3
	// |  |
	// 0--1  -> x
	// top layer is 4-7 accordingly
	vector3 p0 = orientation.rotate(vector3(pmin.x, pmin.y, pmin.z)) + position;
	vector3 p1 = orientation.rotate(vector3(pmax.x, pmin.y, pmin.z)) + position;
	vector3 p2 = orientation.rotate(vector3(pmin.x, pmax.y, pmin.z)) + position;
	vector3 p3 = orientation.rotate(vector3(pmax.x, pmax.y, pmin.z)) + position;
	vector3 p4 = orientation.rotate(vector3(pmin.x, pmin.y, pmax.z)) + position;
	vector3 p5 = orientation.rotate(vector3(pmax.x, pmin.y, pmax.z)) + position;
	vector3 p6 = orientation.rotate(vector3(pmin.x, pmax.y, pmax.z)) + position;
	vector3 p7 = orientation.rotate(vector3(pmax.x, pmax.y, pmax.z)) + position;
	if (inverse) {
		bbox.push_back(polygon(p0, p1, p3, p2));
		bbox.push_back(polygon(p0, p4, p5, p1));
		bbox.push_back(polygon(p1, p5, p7, p3));
		bbox.push_back(polygon(p3, p7, p6, p2));
		bbox.push_back(polygon(p2, p6, p4, p0));
		bbox.push_back(polygon(p4, p6, p7, p5));
	} else {
		bbox.push_back(polygon(p0, p2, p3, p1));
		bbox.push_back(polygon(p0, p1, p5, p4));
		bbox.push_back(polygon(p1, p3, p7, p5));
		bbox.push_back(polygon(p3, p2, p6, p7));
		bbox.push_back(polygon(p2, p0, p4, p6));
		bbox.push_back(polygon(p4, p5, p7, p6));
	}
	return bbox;
}



unsigned sea_object::get_min_max_voxel_index_for_polyset(const std::vector<polygon>& polys,
							 vector3i& vxmin, vector3i& vxmax) const
{
	quaternion cjq = orientation.conj();
	matrix4f obj2voxel = get_model().get_base_mesh_transformation().inverse();
	const vector3i& vres = get_model().get_voxel_resolution();
	vector3i vidxmax = vres - vector3i(1, 1, 1);
	vector3f voxel_pos_trans = vector3f(vres) * 0.5f;
	vector3f voxel_size_rcp = get_model().get_voxel_size().rcp();
	vxmin = vres;
	vxmax = vector3i(-1, -1, -1);
	for (unsigned i = 0; i < polys.size(); ++i) {
		const polygon& p = polys[i];
		if (!p.empty()) {
			for (unsigned k = 0; k < p.points.size(); ++k) {
				// transform point to voxel space
				vector3f ptvx = obj2voxel * vector3f(cjq.rotate(p.points[k] - position));
				// transform to voxel coordinate
				vector3i v = vector3i(ptvx.coeff_mul(voxel_size_rcp) + voxel_pos_trans);
				// clip v to valid range
				v = v.max(vector3i(0,0,0)).min(vidxmax);
				// adjust min/max accordingly
				vxmin = vxmin.min(v);
				vxmax = vxmax.max(v);
			}
		}
	}
	return	std::max(0, vxmax.x + 1 - vxmin.x) *
		std::max(0, vxmax.y + 1 - vxmin.y) *
		std::max(0, vxmax.z + 1 - vxmin.z);
}



vector3 sea_object::compute_linear_velocity(const vector3& p) const
{
	// result is v(t) + w(t) x r(t)  (linear velocity + omega cross relative vector)
	vector3 w = orientation.rotate(inertia_tensor_inv * orientation.conj().rotate(angular_momentum));
	return velocity + w.cross(p - position);
}



double sea_object::compute_collision_response_value(const vector3& collision_pos, const vector3& N) const
{
	vector3 r = collision_pos - position;
	return mass_inv + N * orientation.rotate(inertia_tensor_inv * orientation.conj().rotate(r.cross(N))).cross(r);
}



void sea_object::apply_collision_impulse(const vector3& collision_pos, const vector3& J)
{
	vector3 r = collision_pos - position;
	linear_momentum += J;
	angular_momentum += r.cross(J);
	compute_helper_values();
}
