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
	write_u8(out, Uint8(alive_stat));

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
	// calculate sinking
	if (is_defunct()) {
		return;
	} else if (is_dead()) {
		alive_stat = defunct;
		return;
	} else if (is_sinking()) {
		position.z -= delta_time * SINK_SPEED;
		if (position.z < -50)	// used for ships.
			kill();
		throttle = stop;
		rudder_midships();
		return;
	}

	// calculate directional speed
	vector2 dir_speed_2d = heading.direction() * speed;
	vector3 dir_speed(dir_speed_2d.x, dir_speed_2d.y, 0);
	
	// calculate new position
	position += dir_speed * delta_time;
	
	// calculate speed change
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
	sink();
	return true;
}



unsigned sea_object::calc_damage(void) const
{
	return alive_stat == sinking ? 100 : 0;
}



void sea_object::sink(void)
{
	alive_stat = sinking;
}



void sea_object::kill(void)
{
	alive_stat = dead;
}



void sea_object::head_to_ang(const angle& a, bool left_or_right)	// true == left
{
	head_to = a;
	head_chg = (left_or_right) ? -1 : 1;
	permanent_turn = false;
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
	return 1.0/750.0 * get_cross_section(watcher);
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
