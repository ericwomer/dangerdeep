// ships
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "ship.h"
#include "model.h"
#include "game.h"
#include "date.h"
#include "ship_largemerchant.h"
#include "ship_mediummerchant.h"
#include "ship_smallmerchant.h"
#include "ship_mediumtroopship.h"
#include "ship_destroyertribal.h"
#include "ship_battleshipmalaya.h"
#include "ship_carrierbogue.h"
#include "ship_corvette.h"
#include "ship_largefreighter.h"
#include "ship_mediumfreighter.h"
#include "ship_smalltanker.h"
#include "tokencodes.h"
#include "sensors.h"
#include "ai.h"
#include "smoke_stream.h"

ship::ship() : sea_object(), myai ( 0 ), fuel_level ( 1.0 ),
	fuel_value_a ( 0.0 ), fuel_value_t ( 1.0 ), mysmoke(0)
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

ship* ship::create(ship::types type_)
{
	switch (type_) {
		case largemerchant: return new ship_largemerchant();
		case mediummerchant: return new ship_mediummerchant();
		case smallmerchant: return new ship_smallmerchant();
		case mediumtroopship: return new ship_mediumtroopship();
		case destroyertribal: return new ship_destroyertribal();
		case battleshipmalaya: return new ship_battleshipmalaya();
		case carrierbogue: return new ship_carrierbogue();
		case corvette: return new ship_corvette();
		case largefreighter: return new ship_largefreighter();
		case mediumfreighter: return new ship_mediumfreighter();
		case smalltanker: return new ship_smalltanker();
	}
	return 0;
}

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
