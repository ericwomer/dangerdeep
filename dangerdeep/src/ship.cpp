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



ship::ship()
{
}



ship::ship(const string& specfilename_) : sea_object()
{
	specfilename = specfilename_;

	TiXmlDocument doc(get_ship_dir() + specfilename + ".xml");
	doc.LoadFile();
	TiXmlHandle hdoc(&doc);
	TiXmlHandle hdftdship = hdoc.FirstChild("dftd-ship");
	TiXmlElement* eclassification = hdftdship.FirstChildElement("classification").Element();
	system::sys().myassert(eclassification != 0, string("classification node missing in ")+specfilename);
	modelname = eclassification->Attribute("modelname");
	modelcache.ref(modelname);
	string typestr = eclassification->Attribute("type");
	if (typestr == "warship") shipclass = WARSHIP;
	else if (typestr == "escort") shipclass = ESCORT;
	else if (typestr == "merchant") shipclass = MERCHANT;
	else if (typestr == "submarine") shipclass = SUBMARINE;
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
	else if (aitype == "none") myai = 0;
	else system::sys().myassert(false, string("illegal AI type in ") + specfilename);
	TiXmlElement* efuel = hdftdship.FirstChildElement("fuel").Element();
	system::sys().myassert(efuel != 0, string("fuel node missing in ")+specfilename);
	fuel_level = atof(efuel->Attribute("capacity"));
	fuel_value_a = atof(efuel->Attribute("consumption_a"));
	fuel_value_t = atof(efuel->Attribute("consumption_t"));

	// fixme smoke
	mysmoke = 0;
}



ship::ship(parser& p)
{
}



ship::~ship()
{
	delete myai;
	delete mysmoke;
}



void ship::sink(void)
{
	sea_object::sink();
	if (mysmoke) mysmoke->kill();
}



bool ship::parse_attribute(parser& p)
{
	if ( sea_object::parse_attribute(p) )
		return true;
	switch ( p.type () )
	{
		case TKN_FUEL:
			p.consume ();
			p.parse ( TKN_ASSIGN );
			fuel_level = p.parse_number () / 100.0f;
			p.parse ( TKN_SEMICOLON );
			break;
	}
	return false;
}

void ship::load(istream& in, game& g)
{
	sea_object::load(in, g);

//fixme: add values from xml read
	if (read_bool(in))
		myai = new ai(in, g);
	
	tonnage = read_u32(in);
	stern_damage = damage_status(read_u8(in));
	midship_damage = damage_status(read_u8(in));
	bow_damage = damage_status(read_u8(in));
	fuel_level = read_double(in);
	fuel_value_a = read_double(in);
	fuel_value_t = read_double(in);
}

void ship::save(ostream& out, const game& g) const
{
	sea_object::save(out, g);

	write_bool(out, (myai != 0));
	if (myai)
		myai->save(out, g);
	
//fixme: add values from xml read
	write_u32(out, tonnage);
	write_u8(out, stern_damage);
	write_u8(out, midship_damage);
	write_u8(out, bow_damage);
	write_double(out, fuel_level);
	write_double(out, fuel_value_a);
	write_double(out, fuel_value_t);
}



void ship::simulate(game& gm, double delta_time)
{
	sea_object::simulate(gm, delta_time);
	if ( myai )
		myai->act(gm, delta_time);

	// Adjust fuel_level.
	calculate_fuel_factor ( delta_time );
	
	if (mysmoke) {
		if (!is_sinking())
			mysmoke->set_source(position + vector3(0, 0, 10));//fixme add pos. relative to ship
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
