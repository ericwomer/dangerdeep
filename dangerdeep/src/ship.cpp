// ships
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "ship.h"
#include "model.h"
#include "game.h"
#include "date.h"
#include "tokencodes.h"
#include "sensors.h"
#include "ai.h"
#include "smoke_stream.h"
#include "system.h"
#include "tinyxml/tinyxml.h"



// empty c'tor is needed by heirs
ship::ship() : myai(0), mysmoke(0)
{
}



ship::ship(TiXmlDocument* specfile, const char* topnodename) : sea_object(specfile, topnodename)
{
	TiXmlHandle hspec(specfile);
	TiXmlHandle hdftdship = hspec.FirstChild(topnodename);
	TiXmlElement* eclassification = hdftdship.FirstChildElement("classification").Element();
	system::sys().myassert(eclassification != 0, string("ship: classification node missing in ")+specfilename);
	string typestr = XmlAttrib(eclassification, "type");
	if (typestr == "warship") shipclass = WARSHIP;
	else if (typestr == "escort") shipclass = ESCORT;
	else if (typestr == "merchant") shipclass = MERCHANT;
	else if (typestr == "submarine") shipclass = SUBMARINE;
	else system::sys().myassert(false, string("illegal ship type in ") + specfilename);
	TiXmlElement* etonnage = hdftdship.FirstChildElement("tonnage").Element();
	system::sys().myassert(etonnage != 0, string("tonnage node missing in ")+specfilename);
	unsigned minton = XmlAttribu(etonnage, "min");
	unsigned maxton = XmlAttribu(etonnage, "max");
	tonnage = minton + rnd(maxton - minton + 1);
	TiXmlElement* esmoke = hdftdship.FirstChildElement("smoke").Element();
	mysmoke = 0;
	if (esmoke) {
		int smtype = 0;
		esmoke->Attribute("type", &smtype);
		if (smtype > 0) {
			TiXmlElement* esmpos = esmoke->FirstChildElement("position");
			system::sys().myassert(esmpos != 0, string("no smoke position given in ")+specfilename);
			esmpos->Attribute("x", &smokerelpos.x);
			esmpos->Attribute("y", &smokerelpos.y);
			esmpos->Attribute("z", &smokerelpos.z);
			mysmoke = new smoke_stream(position+smokerelpos, 2);
		}
	}
	TiXmlElement* eai = hdftdship.FirstChildElement("ai").Element();
	system::sys().myassert(eai != 0, string("ai node missing in ")+specfilename);
	string aitype = XmlAttrib(eai, "type");
	if (aitype == "dumb") myai = new ai(this, ai::dumb);
	else if (aitype == "escort") myai = new ai(this, ai::escort);
	else if (aitype == "none") myai = 0;
	else system::sys().myassert(false, string("illegal AI type in ") + specfilename);
	TiXmlElement* efuel = hdftdship.FirstChildElement("fuel").Element();
	system::sys().myassert(efuel != 0, string("fuel node missing in ")+specfilename);
	fuel_capacity = XmlAttribu(efuel, "capacity");
	efuel->Attribute("consumption_a", &fuel_value_a);
	efuel->Attribute("consumption_t", &fuel_value_t);
}



ship::~ship()
{
	delete myai;
	delete mysmoke;
}



void ship::sink(void)
{
	sea_object::kill();
	if (mysmoke) mysmoke->kill();
}



void ship::parse_attributes(TiXmlElement* parent)
{
	sea_object::parse_attributes(parent);
	
	// parse tonnage, fuel level, damage status, fixme
}



void ship::load(istream& in, game& g)
{
	sea_object::load(in, g);

	if (read_bool(in))
		myai = new ai(in, g);
	
	tonnage = read_u32(in);
	stern_damage = damage_status(read_u8(in));
	midship_damage = damage_status(read_u8(in));
	bow_damage = damage_status(read_u8(in));
	fuel_level = read_double(in);
}

void ship::save(ostream& out, const game& g) const
{
	sea_object::save(out, g);

	write_bool(out, (myai != 0));
	if (myai)
		myai->save(out, g);
	
	write_u32(out, tonnage);
	write_u8(out, stern_damage);
	write_u8(out, midship_damage);
	write_u8(out, bow_damage);
	write_double(out, fuel_level);
}



void ship::simulate(game& gm, double delta_time)
{
	sea_object::simulate(gm, delta_time);
	if ( myai )
		myai->act(gm, delta_time);

	// calculate sinking
	if (is_dead()) {
		position.z -= delta_time * SINK_SPEED;
		if (position.z < -50)	// used for ships.
			destroy();
		throttle = stop;
		rudder_midships();
		return;
	}

	// Adjust fuel_level.
	calculate_fuel_factor ( delta_time );
	
	if (mysmoke) {
		if (is_alive())
			mysmoke->set_source(position + smokerelpos);
		mysmoke->simulate(gm, delta_time);
	}
}



void ship::fire_shell_at(const vector2& pos)
{
	// fixme!!!!!!
}



void ship::smoke_display(double degr) const
{
	if (mysmoke) mysmoke->display(degr);
}



bool ship::damage(const vector3& fromwhere, unsigned strength)
{
	damage_status& where = midship_damage;//fixme
	int dmg = int(where) + strength;
	if (dmg > wrecked) where = wrecked; else where = damage_status(dmg);
	// fixme:
	stern_damage = midship_damage = bow_damage = wrecked;
	sink();
	return true;
}



unsigned ship::calc_damage(void) const
{
	if (bow_damage == wrecked || midship_damage == wrecked || stern_damage == wrecked)
		return 100;
	unsigned dmg = unsigned(round(15*(bow_damage + midship_damage + stern_damage)));
	return dmg > 100 ? 100 : dmg;
}



double ship::get_roll_factor(void) const
{
	return 400.0 / (get_tonnage() + 6000.0);	// fixme: rather simple yet. should be overloaded by each ship
}



void ship::calculate_fuel_factor ( double delta_time )
{
	fuel_level -= delta_time * get_fuel_consumption_rate ();
}
