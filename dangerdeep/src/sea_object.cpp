// sea objects
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "sea_object.h"
#include "vector2.h"
#include "tokencodes.h"
#include "sensors.h"
#include "model.h"
#include "global_data.h"
#include "tinyxml/tinyxml.h"
#include "system.h"
#include "texts.h"


void sea_object::degrees2meters(bool west, unsigned degx, unsigned minx, bool south,
	unsigned degy, unsigned miny, double& x, double& y)
{
	x = (west ? -1.0 : 1.0)*(double(degx)+double(minx)/60.0)*20000000.0/180.0;
	y = (south ? -1.0 : 1.0)*(double(degy)+double(miny)/60.0)*20000000.0/180.0;
}



void sea_object::meters2degrees(double x, double y, bool& west, unsigned& degx, unsigned& minx, bool& south,
	unsigned& degy, unsigned& miny)
{
	double fracdegrx = fabs(x*180.0/20000000.0);
	double fracdegry = fabs(y*180.0/20000000.0);
	degx = unsigned(floor(fracdegrx)),
	degy = unsigned(floor(fracdegry)),
	minx = unsigned(60.0*myfrac(fracdegrx) + 0.5);
	miny = unsigned(60.0*myfrac(fracdegry) + 0.5);
	west = (x < 0.0);
	south = (y < 0.0);
}



// some heirs need this empty c'tor
sea_object::sea_object() : speed(0), max_speed(0), max_rev_speed(0), throttle(stop),
	acceleration(0), permanent_turn(false), head_chg(0), rudder(0), length(0), width(0),
	alive_stat(alive)
{
	sensors.resize ( last_sensor_system );
}



sea_object::sea_object(TiXmlDocument* specfile, const char* topnodename) :
	speed(0.0), throttle(stop), permanent_turn(false), head_chg(0.0), rudder(ruddermid),
	head_to(0.0), alive_stat(alive)
{
	TiXmlHandle hspec(specfile);
	TiXmlHandle hdftdobj = hspec.FirstChild(topnodename);
	TiXmlElement* eclassification = hdftdobj.FirstChildElement("classification").Element();
	system::sys().myassert(eclassification != 0, string("sea_object: classification node missing in ")+specfile->Value());
	specfilename = XmlAttrib(eclassification, "identifier");
	modelname = XmlAttrib(eclassification, "modelname");
	model* mdl = modelcache.ref(modelname);
	width = mdl->get_width();
	length = mdl->get_length();
	//country = XmlAttrib(eclassification, "country");
	TiXmlHandle hdescription = hdftdobj.FirstChild("description");
	TiXmlElement* edescr = hdescription.FirstChild("far").Element();
	for ( ; edescr != 0; edescr = edescr->NextSiblingElement("far")) {
		if (XmlAttrib(edescr, "lang") == texts::get_language_code()) {
			TiXmlNode* ntext = edescr->FirstChild();
			system::sys().myassert(ntext != 0, string("sea_object: far description text child node missing in ")+specfilename);
			descr_near = ntext->Value();
			break;
		}
	}
	edescr = hdescription.FirstChild("medium").Element();
	for ( ; edescr != 0; edescr = edescr->NextSiblingElement("medium")) {
		if (XmlAttrib(edescr, "lang") == texts::get_language_code()) {
			TiXmlNode* ntext = edescr->FirstChild();
			system::sys().myassert(ntext != 0, string("sea_object: medium description text child node missing in ")+specfilename);
			descr_near = ntext->Value();
			break;
		}
	}
	edescr = hdescription.FirstChild("near").Element();
	for ( ; edescr != 0; edescr = edescr->NextSiblingElement("near")) {
		if (XmlAttrib(edescr, "lang") == texts::get_language_code()) {
			TiXmlNode* ntext = edescr->FirstChild();
			system::sys().myassert(ntext != 0, string("sea_object: near description text child node missing in ")+specfilename);
			descr_near = ntext->Value();
			break;
		}
	}
	TiXmlElement* emotion = hdftdobj.FirstChildElement("motion").Element();
	system::sys().myassert(emotion != 0, string("motion node missing in ")+specfilename);
	double tmp = 0;
	if (emotion->Attribute("maxspeed", &tmp))
		max_speed = kts2ms(tmp);
	tmp = 0;
	if (emotion->Attribute("maxrevspeed", &tmp))
		max_rev_speed = kts2ms(tmp);
	emotion->Attribute("acceleration", &acceleration);
	tmp = 0;
	if (emotion->Attribute("turnrate", &tmp))
		turn_rate = angle(tmp);
	TiXmlHandle hsensors = hdftdobj.FirstChild("sensors");
	sensors.resize ( last_sensor_system );
	TiXmlElement* esensor = hsensors.FirstChild("sensor").Element();
	for ( ; esensor != 0; esensor = esensor->NextSiblingElement("sensor")) {
		string typestr = XmlAttrib(esensor, "type");
		if (typestr == "lookout") set_sensor(lookout_system, new lookout_sensor());
		else if (typestr == "passivesonar") set_sensor(passive_sonar_system, new passive_sonar_sensor());
		else if (typestr == "activesonar") set_sensor(active_sonar_system, new active_sonar_sensor());
		// ignore unknown sensors.
	}
}



sea_object::~sea_object()
{
	remove_all_references();
	modelcache.unref(modelname);
	for (unsigned i = 0; i < sensors.size(); i++)
		delete sensors[i];
}



void sea_object::load(istream& in, class game& g)
{
	specfilename = read_string(in);
	position.x = read_double(in);
	position.y = read_double(in);
	position.z = read_double(in);
	heading = angle(read_double(in));
	speed = read_double(in);
	throttle = read_i8(in);
	permanent_turn = read_bool(in);
	head_chg = read_double(in);
	rudder = read_i8(in);
	head_to = angle(read_double(in));
	alive_stat = alive_status(read_u8(in));

	previous_positions.clear();
	for (unsigned s = read_u8(in); s > 0; --s) {
		double x = read_double(in);
		double y = read_double(in);
		previous_positions.push_back(vector2(x, y));
	}
}



void sea_object::save(ostream& out, const class game& g) const
{
	write_string(out, specfilename);
	write_double(out, position.x);
	write_double(out, position.y);
	write_double(out, position.z);
	write_double(out, heading.value());
	write_double(out, speed);
	write_i8(out, throttle);
	write_bool(out, permanent_turn);
	write_double(out, head_chg);
	write_i8(out, rudder);
	write_double(out, head_to.value());
	write_u8(out, alive_stat);

	write_u8(out, previous_positions.size());
	for (list<vector2>::const_iterator it = previous_positions.begin(); it != previous_positions.end(); ++it) {
		write_double(out, it->x);
		write_double(out, it->y);
	}
}



void sea_object::parse_attributes(TiXmlElement* parent)
{
	TiXmlHandle hdftdobj(parent);
	TiXmlElement* eposition = hdftdobj.FirstChildElement("position").Element();
	if (eposition) {
		vector3 p;
		eposition->Attribute("x", &p.x);
		eposition->Attribute("y", &p.y);
		eposition->Attribute("z", &p.z);
		position = p;
	}
	TiXmlElement* emotion = hdftdobj.FirstChildElement("motion").Element();
	if (emotion) {
		double tmp = 0;
		if (emotion->Attribute("heading", &tmp))
			heading = angle(tmp);
		tmp = 0;
		if (emotion->Attribute("speed", &tmp))
			speed = kts2ms(tmp);
		string thr = XmlAttrib(emotion, "throttle");
		if (thr == "stop") throttle = stop;
		else if (thr == "reverse") throttle = reverse;
		else if (thr == "aheadlisten") throttle = aheadlisten;
		else if (thr == "aheadsonar") throttle = aheadsonar;
		else if (thr == "aheadslow") throttle = aheadslow;
		else if (thr == "aheadhalf") throttle = aheadhalf;
		else if (thr == "aheadfull") throttle = aheadfull;
		else if (thr == "aheadflank") throttle = aheadflank;
		else throttle = atoi(thr.c_str());
	}
	// fixme: parse permanent_turn,head_chg,head_to,rudder  maybe also alive_stat,previous_positions
}



string sea_object::get_description(unsigned detail) const
{
	if (detail == 0) return descr_far;
	else if (detail == 1) return descr_medium;
	else return descr_near;
}



void sea_object::simulate(game& gm, double delta_time)
{
	// fixme: 2004/06/18
	// this should be replaced by directional speed.
	// store spatial position and speed.
	// get acceleration (spatial) from virtual function.
	// solve an ODE with simple x'=x+v*t+a/2*t*t, v'=v+a*t
	// the sea_object interface should be a bit more general (rigid body)
	// even rotation/turning could/should be handled with forces.
	// that is rotational impulse and torque.
	// So store position and velocity for position/orientation.
	// get acceleration/torque for time t, change position/orientation accordingly.
	// attention: torque needs mass. we don't care about that, so use rotaccel.
	// changing position/orientation depends on inertia and mass of object...
	// for display, generate a rotation matrix from quaternion.
	// heading is the angle of the projection of the 2nd column of that matrix onto
	// the x-y plane.
	// acceleration is mostly along local axes, so compute it like this:
	// orientation rotates (0,accel,0) vector (acceleration along local y-axis).
	// or better: get_acceleration() returns LOCAL acceleration.
	// use vector3 acceleration = orientation.rotate(get_acceleration()); etc.
	// or maybe offer both functions (both virtual): get_accel and get_local_accel
	// the first is predefined with the rotation.

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

	// more difficult: change acceleration to match a certain position (or angle)
	// s = s0 + v0 * t + a/2 * t^2, v = v0 + a * t
	// compute a over time so that s_final = s_desired and v_final = 0, with arbitrary s0,v0.
	// a can be 0<=a<=max_accel. three phases (in time) speed grows until max_speed,
	// constant max speed, speed shrinks to zero (sometimes only phases 1 and 3 needed).
	// determine phase, in phase 1 and 2 give max. acceleration, in phase 3 give zero.

	// Screw force splits in forward force and sideward force (dependend on rudder position)
	// so compute side drag from turn_rate
	// compute turn_rate to maximum angular velocity, limited by ship's draught and shape.

	/* change header file:
	vector3 position, velocity;
	double acceleration -> double max_acceleration;
	quaternion orientation, rotvelocity;
	virtual vector3 get_acceleration(const double& t)  maybe t is not needed...
	virtual quaternion get_rotacceleration();
	
	code:
	vector3 acceleration = get_acceleration();
	position += velocity * delta_time + acceleration * (0.5 * delta_time * delta_time);
	velocity += acceleration * delta_time;
	quaternion rotacceleration = get_rotacceleration();
	add orientation and rotvelocity and rotacceleration...
	orientation *= quatrotvelo, quatrotvelo = rotvelo with angle = rotvelo.angle * t
	rotvelo.s = sin(alpha/2), quatrotvelo.s = sin(2*arcsin(rotvelo.s)*t/2)
	  = sin(arcsin(rotvelo.s)*t)
	add rotvelocity and rotacceleration...
	*/	

	// calculate directional speed
	vector2 dir_speed_2d = heading.direction() * speed;
	vector3 dir_speed(dir_speed_2d.x, dir_speed_2d.y, 0);
	
	// calculate new position
	position += dir_speed * delta_time;
	
	// calculate speed change
	// 2004/06/18 fixme: this is not part of physical simulation!
	// changing acceleration to match speed to throttle is more an AI simulation.
	// so it should move to another function or at least cleanly separated from physical
	// simulation.
	double t = get_throttle_speed() - speed;
	double s = acceleration * delta_time;
	if (fabs(t) > s) {
		speed += (t < 0) ? -s : s;
	} else {
		speed = get_throttle_speed();
	}

	/*
		correct turning model:
		object can only accelerate to left or right
		water decelerates the object proportional to its speed
		speed is limited
		acceleration is a non-linear function, we assume a(t) values of +a,0,-a only
		hence speed v(t) is integral over time of a(t)
		and position s(t) is integral over time of v(t) with v(t) <= vmax
		user wants to change depth/course etc. about s units.
		This takes tf seconds.
		Three phases: acceleration (tv seconds), constant speed (tc), deceleration (tv)
		Two modes: tf >= 2*tv or not.
		tv=vmax/a
		s(t) after acceleration is (s=a/2*t^2,t=tv=vmax/a) s'=(vmax^2/a)/2
		hence if s < 2*s' = vmax^2/a then mode 2.
		here tf=2*t with s/2=a/2*t^2 -> s/2=a/2*(tf/2)^2 -> s=a*tf^2/4 ->
		tf = sqrt(4*s/a) -> tf = 2*sqrt(s/a)
		Else mode 1: s'' = s-2*s', tc=s''/vmax and tf = 2*tv+tc = 2*vmax/a + (s-2*s')/vmax
		= 2*vmax/a + (s - vmax^2/a)/vmax
		= 2*vmax/a + s/vmax - vmax/a
		= vmax/a + s/vmax = tf
		How to steer:
		check if s < vmax^2/a then use mode 1: tf=2*sqrt(s/a), accelerate for tf/2 seconds, then decelerate.
		or else use mode 2: accelerate for tv seconds, hold tc, decelerate.
	*/

	// calculate heading change (fixme, this turning is not physically correct)
	angle maxturnangle = turn_rate * (head_chg * delta_time * speed);
	if (permanent_turn) {
		heading += maxturnangle;
	} else {
		double fac = (head_to - heading).value_pm180()
			/ maxturnangle.value_pm180();
		if (0 <= fac && fac <= 1) {
			rudder_midships();
			heading = head_to;
		} else {
			heading += maxturnangle;
		}
	}
}



bool sea_object::damage(const vector3& fromwhere, unsigned strength)
{
	kill();	// fixme crude hack, replace by damage simulation
	return true;
}



unsigned sea_object::calc_damage(void) const
{
	return is_dead() ? 100 : 0;
}



void sea_object::kill(void)
{
	alive_stat = dead;
	remove_all_references();
}



void sea_object::destroy(void)
{
	alive_stat = defunct;
	remove_all_references();
}



void sea_object::head_to_ang(const angle& a, bool left_or_right)	// true == left
{
	head_to = a;
	head_chg = (left_or_right) ? -1 : 1;
	permanent_turn = false;
}



void sea_object::register_ref(sea_object::ref& myref)
{
#if 1
	// check for double ref's
	for (list<ref*>::iterator it = references.begin(); it != references.end(); ++it) {
		if (*it == &myref) {
			system::sys().myassert(false, "sea_object::register_ref : reference is already stored!");
		}
	}
#endif
	references.push_front(&myref);
	myref.myobj = this;
}



void sea_object::unregister_ref(sea_object::ref& myref)
{
	for (list<ref*>::iterator it = references.begin(); it != references.end(); ++it) {
		if (*it == &myref) {
			myref.myobj = 0;
			references.erase(it);
			return;
		}
	}
	system::sys().myassert(false, "sea_object::unregister_ref : reference is not found!");
}



void sea_object::remove_all_references(void)
{
	for (list<ref*>::iterator it = references.begin(); it != references.end(); ++it) {
		(*it)->myobj = 0;
	}
	references.clear();
}



void sea_object::change_rudder (int dir)
{
    // Change rudder state first.
    if ( dir < 0 )
        rudder --;
    else if ( dir > 0 )
        rudder ++;

    // Limit rudder state.
    if ( rudder < rudderfullleft )
        rudder = rudderfullleft;
    else if ( rudder > rudderfullright )
        rudder = rudderfullright;

    // Set head_chg due to rudder state.
    switch ( rudder )
    {
        case rudderfullleft:
            head_chg = -1.0f;
            break;
        case rudderleft:
            head_chg = -0.5f;
            break;
        case rudderright:
            head_chg = 0.5f;
            break;
        case rudderfullright:
            head_chg = 1.0f;
            break;
        default:
            head_chg = 0.0f;
            break;
    }
    
    if ( rudder == ruddermid )
        permanent_turn = false;
    else
        permanent_turn = true;
}



void sea_object::rudder_left(void)
{
	change_rudder ( -1 );
}



void sea_object::rudder_right(void)
{
	change_rudder ( 1 );
}



void sea_object::rudder_hard_left(void)
{
	rudder = rudderfullleft;
	head_chg = -1.0f;
	permanent_turn = true;
}



void sea_object::rudder_hard_right(void)
{
	rudder = rudderfullright;
	head_chg = 1.0f;
	permanent_turn = true;
}



void sea_object::rudder_midships(void)
{
	rudder = ruddermid;
	head_chg = 0.0f;
	head_to = heading;
	permanent_turn = false;
}



void sea_object::set_throttle(throttle_status thr)
{
	throttle = thr;
}



void sea_object::remember_position(void)
{
	previous_positions.push_front(get_pos().xy());
	if (previous_positions.size() > MAXPREVPOS)
		previous_positions.pop_back();
}	



double sea_object::get_throttle_speed(void) const
{
	double ms = get_max_speed();
	if (throttle <= 0) {
		switch (throttle) {
			case reverse: return -ms*0.25f;     // 1/4
			case stop: return 0;
			case aheadlisten: return ms*0.25f;  // 1/4
			case aheadsonar: return ms*0.25f;   // 1/4
			case aheadslow: return ms*0.33333f; // 1/3
			case aheadhalf: return ms*0.5f;     // 1/2
			case aheadfull: return ms*0.75f;    // 3/4
			case aheadflank: return ms;
		}
	} else {
		double sp = kts2ms(throttle);
		if (sp > ms) sp = ms;
		return sp;
	}
	return 0;
}



pair<angle, double> sea_object::bearing_and_range_to(const sea_object* other) const
{
	vector2 diff = other->get_pos().xy() - position.xy();
	return make_pair(angle(diff), diff.length());
}



angle sea_object::estimate_angle_on_the_bow(angle target_bearing, angle target_heading) const
{
	return (angle(180) + target_bearing - target_heading).value_pm180();
}



float sea_object::surface_visibility(const vector2& watcher) const
{
	return get_cross_section(watcher);
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



void sea_object::set_sensor ( sensor_system ss, sensor* s )
{
	if ( ss >= 0 && ss < last_sensor_system ){
		sensors[ss] = s;
	}
}



double sea_object::get_cross_section ( const vector2& d ) const
{
	model* mdl = modelcache.find(modelname);
	if (mdl) {
		vector2 r = get_pos().xy() - d;
		angle diff = angle(r) - heading;
		return mdl->get_cross_section(diff.value());
	}
	return 0.0;


}



double sea_object::get_noise_factor () const
{
    return get_throttle_speed () / max_speed;
}



vector2 sea_object::get_engine_noise_source () const
{
	return get_pos ().xy () - get_heading ().direction () * 0.3f * length;
}



void sea_object::display(void) const
{
	model* mdl = modelcache.find(modelname);

	if ( mdl )
		mdl->display ();
}
