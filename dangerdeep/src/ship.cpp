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



// xml helper functions
void firstchildfailed(TiXmlNode* parent, const string& tagchain)
{
	string tagchain2 = tagchain;
	TiXmlNode* m = parent;
	while (m != 0) {
		tagchain2 = m->Value() + string(", ") + tagchain2;
		m = m->Parent();
	}
	system::sys().myassert(false, string("xml tag not found: ") + tagchain2);
}
TiXmlNode* firstchild(TiXmlNode* parent)
{
	TiXmlNode* n = parent->FirstChild();
	if (!n)
		firstchildfailed(parent, "FirstChild()");
	return n;
}
TiXmlNode* firstchildn(TiXmlNode* parent, const string& childname)
{
	TiXmlNode* n = parent->FirstChild(childname);
	if (!n)
		firstchildfailed(parent, childname);
	return n;
}
// end of helper functions
ship::ship(const string& specfilename_) : sea_object()
{
	specfilename = specfilename_;
	fuel_level = 1.0;
	fuel_value_a = 0.0;//fixme: move to xml
	fuel_value_t = 1.0;//fixme: move to xml
	TiXmlDocument doc(get_ship_dir() + specfilename + ".xml");
	doc.LoadFile();
	TiXmlNode* xdftdship = firstchildn(&doc, "dftd-ship");
	TiXmlNode* xmodelname = firstchildn(xdftdship, "modelname");
	modelname = firstchild(xmodelname)->Value();
	modelcache.ref(modelname);
	TiXmlNode* xdescription = firstchildn(xdftdship, "description");
	//fixme
	TiXmlNode* xtype = firstchildn(xdftdship, "type");
	string ts = firstchild(xtype)->Value();
	if (ts == "warship") shipclass = WARSHIP;
	else if (ts == "escort") shipclass = ESCORT;
	else if (ts == "merchant") shipclass = MERCHANT;
	else system::sys().myassert(false, string("illegal ship type in ") + specfilename_);
	TiXmlNode* xcountry = firstchildn(xdftdship, "country");
	//country = firstchild(xcountry)->Value();
	TiXmlNode* xmaxspeed = firstchildn(xdftdship, "maxspeed");
	max_speed = atof(firstchild(xmaxspeed)->Value());
	TiXmlNode* xmaxrevspeed = firstchildn(xdftdship, "maxrevspeed");
	max_rev_speed = atof(firstchild(xmaxrevspeed)->Value());
	TiXmlNode* xacceleration = firstchildn(xdftdship, "acceleration");
	acceleration = atof(firstchild(xacceleration)->Value());
	TiXmlNode* xturnrate = firstchildn(xdftdship, "turnrate");
	turn_rate = atof(firstchild(xturnrate)->Value());
	TiXmlNode* xtonnage = firstchildn(xdftdship, "tonnage");
	TiXmlNode* xminton = firstchildn(xtonnage, "min");
	unsigned minton = atoi(firstchild(xminton)->Value());
	TiXmlNode* xmaxton = firstchildn(xtonnage, "max");
	unsigned maxton = atoi(firstchild(xmaxton)->Value());
	tonnage = minton + rnd(maxton - minton + 1);
	TiXmlNode* xaitype = firstchildn(xdftdship, "aitype");
	string aitype = firstchild(xaitype)->Value();
	if (aitype == "dumb") myai = new ai(this, ai::dumb);
	else if (aitype == "escort") myai = new ai(this, ai::escort);
	else system::sys().myassert(false, string("illegal AI type in ") + specfilename_);
	// fixme smoke
	mysmoke = 0;
	// fixme description
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
	
	write_u32(out, tonnage);
	write_u8(out, stern_damage);
	write_u8(out, midship_damage);
	write_u8(out, bow_damage);
	write_double(out, fuel_level);
	write_double(out, fuel_value_a);
	write_double(out, fuel_value_t);
}

/*
ship* ship::create(parser& p)
{
	p.parse(TKN_SHIP);
	int t = p.type();
//string s = p.text();
	p.consume();
	switch (t) {
		case TKN_LARGEMERCHANT: return new ship_largemerchant(p);
		case TKN_MEDIUMMERCHANT: return new ship_mediummerchant(p);
		case TKN_SMALLMERCHANT: return new ship_smallmerchant(p);
		case TKN_MEDIUMTROOPSHIP: return new ship_mediumtroopship(p);
		case TKN_DESTROYERTRIBAL: return new ship_destroyertribal(p);
		case TKN_BATTLESHIPMALAYA: return new ship_battleshipmalaya(p);
		case TKN_CARRIERBOGUE: return new ship_carrierbogue(p);
		case TKN_CORVETTE: return new ship_corvette(p);
		case TKN_LARGEFREIGHTER: return new ship_largefreighter(p);
		case TKN_MEDIUMFREIGHTER: return new ship_mediumfreighter(p);
		case TKN_SMALLTANKER: return new ship_smalltanker(p);
	}
//cerr << "token " << s << " unknown.\n";	
	return 0;
}
*/





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
