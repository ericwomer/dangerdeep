// ships
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "ship.h"
#include "model.h"
#include "game.h"
#include "ship_mediummerchant.h"
#include "ship_mediumtroopship.h"
#include "ship_destroyertribal.h"
#include "ship_battleshipmalaya.h"
#include "tokencodes.h"

ship::ship() : sea_object()
{
}

bool ship::parse_attribute(parser& p)
{
	return sea_object::parse_attribute(p);
}

ship* ship::create(ship::types type_)
{
	switch (type_) {
		case mediummerchant: return new ship_mediummerchant();
		case mediumtroopship: return new ship_mediumtroopship();
		case destroyertribal: return new ship_destroyertribal();
		case battleshipmalaya: return new ship_battleshipmalaya();
	}
	return 0;
}

ship* ship::create(parser& p)
{
	p.parse(TKN_SHIP);
	int t = p.type();
	p.consume();
	switch (t) {
		case TKN_MEDIUMMERCHANT: return new ship_mediummerchant(p);
		case TKN_MEDIUMTROOPSHIP: return new ship_mediumtroopship(p);
		case TKN_DESTROYERTRIBAL: return new ship_destroyertribal(p);
		case TKN_BATTLESHIPMALAYA: return new ship_battleshipmalaya(p);
	}
	return 0;
}

void ship::simulate(game& gm, double delta_time)
{
	sea_object::simulate(gm, delta_time);
	myai->act(gm, delta_time);
}

void ship::fire_shell_at(const vector2& pos)
{
}

void ship::damage(const vector3& fromwhere, unsigned strength)
{
	sink();//fixme
	damage_status& where = midship_damage;//fixme
	int dmg = int(where) + strength;
	if (dmg > wrecked) where = wrecked; else where = damage_status(dmg);
}

unsigned ship::calc_damage(void) const
{
	if (bow_damage == wrecked || midship_damage == wrecked || stern_damage == wrecked)
		return 100;
	unsigned dmg = unsigned(round(15*(bow_damage + midship_damage + stern_damage)));
	return dmg > 100 ? 100 : dmg;
}

void ship::sink(void)
{
	stern_damage = midship_damage = bow_damage = wrecked;
//	game.ship_sunk(get_tonnage());
	sea_object::sink();
}

void ship::kill(void)
{
	stern_damage = midship_damage = bow_damage = wrecked;
	sea_object::kill();
}
