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
#include "ai.h"



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



vector3 sea_object::get_acceleration(void) const
{
	return vector3(0, 0, -GRAVITY);
}



double sea_object::get_turn_acceleration(void) const
{
	return 0.0;
}



// some heirs need this empty c'tor
sea_object::sea_object() :
	turn_velocity(0), heading(0), alive_stat(alive), myai(0)
{
	sensors.resize ( last_sensor_system );
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
		angle diff = angle(r) - get_heading();
		return mdl->get_cross_section(diff.value());
	}
	return 0.0;


}



sea_object::sea_object(TiXmlDocument* specfile, const char* topnodename) :
	turn_velocity(0), heading(0), alive_stat(alive), myai(0)
{
	TiXmlHandle hspec(specfile);
	TiXmlHandle hdftdobj = hspec.FirstChild(topnodename);
	TiXmlElement* eclassification = hdftdobj.FirstChildElement("classification").Element();
	system::sys().myassert(eclassification != 0, string("sea_object: classification node missing in ")+specfile->Value());
	specfilename = XmlAttrib(eclassification, "identifier");
	modelname = XmlAttrib(eclassification, "modelname");
	model* mdl = modelcache.ref(modelname);
	size3d = vector3f(mdl->get_width(), mdl->get_length(), mdl->get_height());
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
	modelcache.unref(modelname);
	delete myai;
	for (unsigned i = 0; i < sensors.size(); i++)
		delete sensors[i];
}



void sea_object::load(istream& in, class game& g)
{
	specfilename = read_string(in);
	position = read_vector3(in);
	velocity = read_vector3(in);
	orientation = read_quaternion(in);
	turn_velocity = read_double(in);
	heading = angle(read_double(in));
	
/*	
	heading = angle(read_double(in));
	speed = read_double(in);
	throttle = read_i8(in);
	permanent_turn = read_bool(in);
	head_chg = read_double(in);
	rudder = read_i8(in);
	head_to = angle(read_double(in));
*/
	alive_stat = alive_status(read_u8(in));

/*
	previous_positions.clear();
	for (unsigned s = read_u8(in); s > 0; --s) {
		double x = read_double(in);
		double y = read_double(in);
		previous_positions.push_back(vector2(x, y));
	}
*/
}



void sea_object::save(ostream& out, const class game& g) const
{
	write_string(out, specfilename);
	write_vector3(out, position);
	write_vector3(out, velocity);
	write_quaternion(out, orientation);
	write_double(out, turn_velocity);
	write_double(out, heading.value());
	
/*
	write_double(out, position.z);
	write_double(out, heading.value());
	write_double(out, speed);
	write_i8(out, throttle);
	write_bool(out, permanent_turn);
	write_double(out, head_chg);
	write_i8(out, rudder);
	write_double(out, head_to.value());
*/	
	write_u8(out, alive_stat);

/*
	write_u8(out, previous_positions.size());
	for (list<vector2>::const_iterator it = previous_positions.begin(); it != previous_positions.end(); ++it) {
		write_double(out, it->x);
		write_double(out, it->y);
	}
*/
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
			velocity.y = kts2ms(tmp);
	}
}



string sea_object::get_description(unsigned detail) const
{
	if (detail == 0) return descr_far;
	else if (detail == 1) return descr_medium;
	else return descr_near;
}



void sea_object::simulate(game& gm, double delta_time)
{
	// check and change states
        if (is_defunct()) {
                return;
        } else if (is_dead()) {
                alive_stat = defunct;
                return;
        }

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

	// drag: drag force is -b * v (low speeds), -b * v^2 (high speeds)
	// b is proportional to cross section area of body.

	// more difficult: change acceleration to match a certain position (or angle)
	// s = s0 + v0 * t + a/2 * t^2, v = v0 + a * t
	// compute a over time so that s_final = s_desired and v_final = 0, with arbitrary s0,v0.
	// a can be 0<=a<=max_accel. three phases (in time) speed grows until max_speed,
	// constant max speed, speed shrinks to zero (sometimes only phases 1 and 3 needed).
	// determine phase, in phase 1 and 2 give max. acceleration, in phase 3 give zero.

	// Screw force splits in forward force and sideward force (dependend on rudder position)
	// so compute side drag from turn_rate
	// compute turn_rate to maximum angular velocity, limited by ship's draught and shape.

	// we store velocity/acceleration, not momentum/force/torque
	// in reality applied force is transformed by inertia tensor, which is rotated according to
	// orientation. we don't use mass here, so apply rotation to stored velocity/acceleration.
	quaternion horientation = quaternion::rot(-heading.value(), 0, 0, 1);

	vector3 acceleration = get_acceleration();
	vector3 global_acceleration = horientation.rotate(acceleration);
	global_velocity = horientation.rotate(velocity);
	double t2_2 = 0.5 * delta_time * delta_time;

//cout << "object " << this << " simulate.\npos: " << position << "\nvelo: " << velocity << "\naccel: " << acceleration << "\n";

	position += global_velocity * delta_time + global_acceleration * t2_2;
	velocity += acceleration * delta_time;

//	cout << "NEWpos: " << position << "\nNEWvelo: " << velocity << "\n";
//	cout << "(delta t = " << delta_time << ")\n";
	
	double turnaccel = get_turn_acceleration();
	double add_turn_angle = turn_velocity * delta_time + turnaccel * t2_2;
	orientation = quaternion::rot(add_turn_angle, 0, 0, 1) * orientation;
	heading += angle(add_turn_angle);
	turn_velocity += turnaccel * delta_time;

//cout << "object " << this << " orientat: " << orientation << " rot_velo: " << turn_velocity << " turn_accel " << turnaccel << "\n";

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



void sea_object::set_inactive(void)
{
	alive_stat = inactive;
}



void sea_object::kill(void)
{
	alive_stat = dead;
}



void sea_object::destroy(void)
{
	alive_stat = defunct;
}



float sea_object::surface_visibility(const vector2& watcher) const
{
	return get_cross_section(watcher);
}



//fixme: should move to ship or maybe return pos. airplanes have engines, but not
//dc's/shells.
vector2 sea_object::get_engine_noise_source () const
{
	return get_pos ().xy () - get_heading().direction () * 0.3f * get_length();
}



void sea_object::display(void) const
{
	model* mdl = modelcache.find(modelname);

	if ( mdl )
		mdl->display ();
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
