// game
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include "../config.h"
#endif

#include <SDL.h>
#include <GL/gl.h>

#include "system.h"
#include <sstream>

#include "game.h"
#include "ship.h"
#include "airplane.h"
#include "torpedo.h"
#include "depth_charge.h"
#include "gun_shell.h"
#include "model.h"
#include "global_data.h"
#include "parser.h"
#include "water_splash.h"
#include "user_interface.h"
#include "submarine_interface.h"
#include "airplane_interface.h"
#include "ship_interface.h"
#include "tokencodes.h"
#include "texts.h"
#include "sensors.h"
#include "menu.h"	// fixme why this? get rid of it
#include "command.h"
#include "network.h"
#include "matrix4.h"
#include "quaternion.h"

const int SAVEVERSION = 0;
const int GAMETYPE = 0;//fixme

#define TRAILTIME 10
#define ENEMYCONTACTLOST 100000.0	// meters

game::ping::ping(istream& in)
{
	pos.x = read_double(in);
	pos.y = read_double(in);
	dir = angle(read_double(in));
	time = read_double(in);
	range = read_double(in);
	ping_angle = angle(read_double(in));
}

void game::ping::save(ostream& out) const
{
	write_double(out, pos.x);
	write_double(out, pos.y);
	write_double(out, dir.value());
	write_double(out, time);
	write_double(out, range);
	write_double(out, ping_angle.value());
}

game::sink_record::sink_record(istream& in)
{
	dat.load(in);
	descr = read_string(in);
	tons = read_u32(in);
}

void game::sink_record::save(ostream& out) const
{
	dat.save(out);
	write_string(out, descr);
	write_u32(out, tons);
}

game::game(submarine::types subtype, unsigned cvsize, unsigned cvesc, unsigned timeofday, unsigned nr_of_players)
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
***********************************************************************/	
	networktype = 0;
	servercon = 0;
	ui = 0;

	time = 86400*500;	// fixme random dependent of period
	
	// all code from here on is fixme and experimental.
	switch (timeofday) {
		case 0: time += 20*3600+5*3600*rnd(); break;		
		case 1: time += 3*3600*rnd(); break;		
		case 2: time += 6*3600+1800*rnd(); break;		
		case 3: time += 18*3600+1800*rnd(); break;		
	};

	convoy* cv = new convoy(*this, (convoy::types)(cvsize), (convoy::esctypes)(cvesc));
	for (list<pair<ship*, vector2> >::iterator it = cv->merchants.begin(); it != cv->merchants.end(); ++it)
		spawn_ship(it->first);
	for (list<pair<ship*, vector2> >::iterator it = cv->warships.begin(); it != cv->warships.end(); ++it)
		spawn_ship(it->first);
	for (list<pair<ship*, vector2> >::iterator it = cv->escorts.begin(); it != cv->escorts.end(); ++it)
		spawn_ship(it->first);
	spawn_convoy(cv);

	vector<angle> subangles;
	submarine* psub = 0;
	for (unsigned i = 0; i < nr_of_players; ++i) {	
		submarine* sub = submarine::create(subtype);//fixme give time for init
		if (i == 0) {
			psub = sub;
			player = psub;
			compute_max_view_dist();
		}
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
		vector2 tmpp = tmpa.direction() * (get_max_view_distance()/2);
		sub->position = vector3(tmpp.x, tmpp.y, timeofday == 2 ? 0 : -12); // fixme maybe always surfaced, except late in war
		sub->heading = sub->head_to = angle(rnd()*360.0);
	
		spawn_submarine(sub);
	}
	player = psub;

	running = true;
	last_trail_time = time - TRAILTIME;
}

game::game(parser& p) : running(true), time(0)
{
	networktype = 0;
	servercon = 0;
	
	player = 0;
	ui = 0;
	while (!p.is_empty()) {
		bool nextisplayer = false;
		if (p.type() == TKN_PLAYER) {
			if (player != 0) {
				p.error("Player defined twice!");
			}
			p.consume();
			nextisplayer = true;
		}
		switch (p.type()) {
			case TKN_SUBMARINE: {
				submarine* sub = submarine::create(p);
				spawn_submarine(sub);
				if (nextisplayer)
					player = sub;
				break; }
			case TKN_SHIP: {
				ship* shp = ship::create(p);
				spawn_ship(shp);
				if (nextisplayer)
					player = shp;
				break; }
			case TKN_AIRPLANE: {
				airplane* apl = airplane::create(p);
				spawn_airplane(apl);
				if (nextisplayer)
					player = apl;
				break; }
			case TKN_CONVOY: {
				convoy* cv = new convoy(*this, p);
				spawn_convoy(cv);
				break; }
/*			
			case TKN_DESCRIPTION:
			case TKN_WEATHER:
*/			
			case TKN_TIME: {
				p.consume();
				unsigned year, month, day, hour, minute, second;
				year = p.parse_number();
				p.parse(TKN_COMMA);
				month = p.parse_number();
				p.parse(TKN_COMMA);
				day = p.parse_number();
				p.parse(TKN_COMMA);
				hour = p.parse_number();
				p.parse(TKN_COMMA);
				minute = p.parse_number();
				p.parse(TKN_COMMA);
				second = p.parse_number();
				p.parse(TKN_SEMICOLON);
				time = ::get_time(year, month, day) + 3600*hour + 60*minute + second;
				break;
				}
			default: p.error("game: Expected definition");
		}
	}
	last_trail_time = time - TRAILTIME;
	if (player == 0) p.error("No player defined!");
	
	compute_max_view_dist();
}

game::~game()
{
	for (list<ship*>::iterator it = ships.begin(); it != ships.end(); ++it)
		delete (*it);
	for (list<submarine*>::iterator it = submarines.begin(); it != submarines.end(); ++it)
		delete (*it);
	for (list<airplane*>::iterator it = airplanes.begin(); it != airplanes.end(); ++it)
		delete (*it);
	for (list<torpedo*>::iterator it = torpedoes.begin(); it != torpedoes.end(); ++it)
		delete (*it);
	for (list<depth_charge*>::iterator it = depth_charges.begin(); it != depth_charges.end(); ++it)
		delete (*it);
	for (list<gun_shell*>::iterator it = gun_shells.begin(); it != gun_shells.end(); ++it)
		delete (*it);
	for (list<convoy*>::iterator it = convoys.begin(); it != convoys.end(); ++it)
		delete (*it);
	for (list<pair<double, job*> >::iterator it = jobs.begin(); it != jobs.end(); ++it)
		delete it->second;
}

void game::save(const string& savefilename, const string& description) const
{
cout<<"saving game...\n";
	ofstream out(savefilename.c_str(), ios::out|ios::binary);
	write_string(out, "DANGER FROM THE DEEP game save file");
	write_string(out, description);
	
	save_to_stream(out);
}

game::game(const string& savefilename)
{
	networktype = 0;
	servercon = 0;	//fixme maybe move to load_from_stream? to allow loading network games?
	ui = 0;

cout<<"loading game...\n";
	ifstream in(savefilename.c_str(), ios::in|ios::binary);
	read_string(in);
	string description = read_string(in);
	
	load_from_stream(in);
}

game::game(istream& in)
{
	networktype = 0;
	servercon = 0;	//fixme maybe move to load_from_stream? to allow loading network games?
	ui = 0;
	load_from_stream(in);
}

string game::read_description_of_savegame(const string& filename)
{
	ifstream in(filename.c_str(), ios::in|ios::binary);
	read_string(in);
	string desc = read_string(in);
	int versionnr = read_i32(in);
	if (versionnr != SAVEVERSION) system::sys().myassert(false, "invalid save game version");
	return desc;
}

void game::save_to_stream(ostream& out) const
{
	write_i32(out, SAVEVERSION);

	write_i32(out, GAMETYPE);

	write_u32(out, ships.size());
	for (list<ship*>::const_iterator ip = ships.begin(); ip != ships.end(); ++ip)
		write_u16(out, (*ip)->get_type());
	write_u32(out, submarines.size());
	for (list<submarine*>::const_iterator ip = submarines.begin(); ip != submarines.end(); ++ip)
		write_u16(out, (*ip)->get_type());
	write_u32(out, airplanes.size());
	write_u32(out, torpedoes.size());
	write_u32(out, depth_charges.size());
	write_u32(out, gun_shells.size());
	write_u32(out, convoys.size());
	write_u32(out, water_splashs.size());

	for (list<ship*>::const_iterator ip = ships.begin(); ip != ships.end(); ++ip)
		(*ip)->save(out, *this);

	for (list<submarine*>::const_iterator ip = submarines.begin(); ip != submarines.end(); ++ip)
		(*ip)->save(out, *this);

	for (list<airplane*>::const_iterator ip = airplanes.begin(); ip != airplanes.end(); ++ip)
		(*ip)->save(out, *this);

	for (list<torpedo*>::const_iterator ip = torpedoes.begin(); ip != torpedoes.end(); ++ip)
		(*ip)->save(out, *this);

	for (list<depth_charge*>::const_iterator ip = depth_charges.begin(); ip != depth_charges.end(); ++ip)
		(*ip)->save(out, *this);

	for (list<gun_shell*>::const_iterator ip = gun_shells.begin(); ip != gun_shells.end(); ++ip)
		(*ip)->save(out, *this);

	for (list<convoy*>::const_iterator ip = convoys.begin(); ip != convoys.end(); ++ip)
		(*ip)->save(out, *this);

	for (list<water_splash*>::const_iterator ip = water_splashs.begin(); ip != water_splashs.end(); ++ip)
		(*ip)->save(out, *this);

	// running / stopexec doesn't need to be saved
	
	// jobs are generated by dftd itself

	submarine* tmpsub = dynamic_cast<submarine*>(player);
	if (tmpsub) {	
		write_i32(out, 1);      // player is submarine
		write(out, tmpsub);
	} else {
		ship* tmpship = dynamic_cast<ship*>(player);
		if (tmpship) {
			write_i32(out, 0);	// player is ship
			write(out, tmpship);
		} else {
			airplane* tmpairpl = dynamic_cast<airplane*>(player);
			if (tmpairpl) {
				write_i32(out, 2);
				write(out, tmpairpl);
			} else {
				system::sys().myassert(false, "internal error: player is no sub, ship or airplane");
			}
		}
	}
	
	// user interface is generated according to player object by dftd

	write_u8(out, sunken_ships.size());
	for (list<sink_record>::const_iterator it = sunken_ships.begin(); it != sunken_ships.end(); ++it) {
		it->save(out);
	}

	write_double(out, time);
	write_double(out, last_trail_time);
	
	write_double(out, max_view_dist);
	
	write_u8(out, pings.size());
	for (list<ping>::const_iterator it = pings.begin(); it != pings.end(); ++it) {
		it->save(out);
	}
}

void game::load_from_stream(istream& in)
{
	int versionnr = read_i32(in);
	if (versionnr != SAVEVERSION) system::sys().myassert(false, "invalid save game version");
	int gametype = read_i32(in);

	for (unsigned s = read_u32(in); s > 0; --s) {
		unsigned type = read_u16(in);
		ships.push_back(ship::create(in, type));
	}
	for (unsigned s = read_u32(in); s > 0; --s) {
		unsigned type = read_u16(in);
		submarines.push_back(submarine::create(in, type));
	}
	for (unsigned s = read_u32(in); s > 0; --s) airplanes.push_back(new airplane());
	for (unsigned s = read_u32(in); s > 0; --s) torpedoes.push_back(new torpedo());
	for (unsigned s = read_u32(in); s > 0; --s) depth_charges.push_back(new depth_charge());
	for (unsigned s = read_u32(in); s > 0; --s) gun_shells.push_back(new gun_shell());
	for (unsigned s = read_u32(in); s > 0; --s) convoys.push_back(new convoy());
	for (unsigned s = read_u32(in); s > 0; --s) water_splashs.push_back(new water_splash());

	for (list<ship*>::iterator ip = ships.begin(); ip != ships.end(); ++ip)
		(*ip)->load(in, *this);

	for (list<submarine*>::iterator ip = submarines.begin(); ip != submarines.end(); ++ip)
		(*ip)->load(in, *this);

	for (list<airplane*>::iterator ip = airplanes.begin(); ip != airplanes.end(); ++ip)
		(*ip)->load(in, *this);

	for (list<torpedo*>::iterator ip = torpedoes.begin(); ip != torpedoes.end(); ++ip)
		(*ip)->load(in, *this);

	for (list<depth_charge*>::iterator ip = depth_charges.begin(); ip != depth_charges.end(); ++ip)
		(*ip)->load(in, *this);

	for (list<gun_shell*>::iterator ip = gun_shells.begin(); ip != gun_shells.end(); ++ip)
		(*ip)->load(in, *this);

	for (list<convoy*>::iterator ip = convoys.begin(); ip != convoys.end(); ++ip)
		(*ip)->load(in, *this);

	for (list<water_splash*>::iterator ip = water_splashs.begin(); ip != water_splashs.end(); ++ip)
		(*ip)->load(in, *this);

	// running / stopexec doesn't need to be saved
	running = true;
	stopexec = false;
	
	// jobs are generated by dftd itself
	//fixme

	int playertype = read_i32(in);
	if (playertype == 0) {
		ship* s = read_ship(in);
		player = s;
	} else if (playertype == 1) {
		submarine* s = read_submarine(in);
		player = s;
	} else if (playertype == 2) {
		airplane* a = read_airplane(in);
		player = a;
	} else {
		system::sys().myassert(false, "savegame error: player is no sub or ship");
	}
	
	// user interface is generated according to player object by dftd
	//fixme

	sunken_ships.clear();
	for (unsigned s = read_u8(in); s > 0; --s)
		sunken_ships.push_back(sink_record(in));

	time = read_double(in);
	last_trail_time = read_double(in);
	
	max_view_dist = read_double(in);
	
	pings.clear();
	for (unsigned s = read_u8(in); s > 0; --s)
		pings.push_back(ping(in));
}

void game::compute_max_view_dist(void)
{
	// a bit unprecise here, since the viewpos is not always the same as the playerpos
	max_view_dist = 5000.0 + compute_light_brightness(player->get_pos()) * 25000;
}

void game::simulate(double delta_t)
{
	if (!running) return;

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
		system::sys().add_console("player killed!");//testing fixme
		running = false;
		return;
	}
	
	if (/* submarines.size() == 0 && */ ships.size() == 0 && torpedoes.size() == 0 && depth_charges.size() == 0 &&
			airplanes.size() == 0 && gun_shells.size() == 0) {
		system::sys().add_console("no objects except player left!");//testing fixme
		running = false;
		return;
	}

	compute_max_view_dist();
	
	bool record = false;
	if (get_time() >= last_trail_time + TRAILTIME) {
		last_trail_time += TRAILTIME;
		record = true;
	}
	
	//fixme 2003/07/11: time compression trashes trail recording.

	double nearest_contact = 1e10;
	for (list<ship*>::iterator it = ships.begin(); it != ships.end(); ) {
		list<ship*>::iterator it2 = it++;
		double dist = (*it2)->get_pos().distance(player->get_pos());
		if (dist < nearest_contact) nearest_contact = dist;
		if (!(*it2)->is_defunct()) {
			(*it2)->simulate(*this, delta_t);
			if (record) (*it2)->remember_position();
		} else {
			delete (*it2);
			ships.erase(it2);
		}
	}
	for (list<submarine*>::iterator it = submarines.begin(); it != submarines.end(); ) {
		list<submarine*>::iterator it2 = it++;
		if ((*it2) != player) {
			double dist = (*it2)->get_pos().distance(player->get_pos());
			if (dist < nearest_contact) nearest_contact = dist;
		}
		if (!(*it2)->is_defunct()) {
			(*it2)->simulate(*this, delta_t);
			if (record) (*it2)->remember_position();
		} else {
			delete (*it2);
			submarines.erase(it2);
		}
	}
	for (list<airplane*>::iterator it = airplanes.begin(); it != airplanes.end(); ) {
		list<airplane*>::iterator it2 = it++;
		double dist = (*it2)->get_pos().distance(player->get_pos());
		if (dist < nearest_contact) nearest_contact = dist;
		if (!(*it2)->is_defunct()) {
			(*it2)->simulate(*this, delta_t);
		} else {
			delete (*it2);
			airplanes.erase(it2);
		}
	}
	for (list<torpedo*>::iterator it = torpedoes.begin(); it != torpedoes.end(); ) {
		list<torpedo*>::iterator it2 = it++;
		if (!(*it2)->is_defunct()) {
			(*it2)->simulate(*this, delta_t);
			if (record) (*it2)->remember_position();
		} else {
			delete (*it2);
			torpedoes.erase(it2);
		}
	}
	for (list<depth_charge*>::iterator it = depth_charges.begin(); it != depth_charges.end(); ) {
		list<depth_charge*>::iterator it2 = it++;
		if (!(*it2)->is_defunct()) {
			(*it2)->simulate(*this, delta_t);
		} else {
			delete (*it2);
			depth_charges.erase(it2);
		}
	}
	for (list<gun_shell*>::iterator it = gun_shells.begin(); it != gun_shells.end(); ) {
		list<gun_shell*>::iterator it2 = it++;
		if (!(*it2)->is_dead()) {
			(*it2)->simulate(*this, delta_t);
		} else {
			delete (*it2);
			gun_shells.erase(it2);
		}
	}
	for (list<convoy*>::iterator it = convoys.begin(); it != convoys.end(); ) {
		list<convoy*>::iterator it2 = it++;
		if (!(*it2)->is_defunct()) {
			(*it2)->simulate(*this, delta_t);
		} else {
			delete (*it2);
			convoys.erase(it2);
		}
	}
	for (list<water_splash*>::iterator it = water_splashs.begin(); it != water_splashs.end (); )
	{
		list<water_splash*>::iterator it2 = it++;
		if ( !(*it2)->is_dead () ) {
			(*it2)->simulate ( *this, delta_t );
		}
		else {
			delete (*it2);
			water_splashs.erase ( it2 );
		}
	}
	time += delta_t;
	
	// remove old pings
	for (list<ping>::iterator it = pings.begin(); it != pings.end(); ) {
		list<ping>::iterator it2 = it++;
		if (time - it2->time > PINGREMAINTIME)
			pings.erase(it2);
	}
	
	if (nearest_contact > ENEMYCONTACTLOST) {
		system::sys().add_console("player lost contact to enemy!");//testing fixme
		running = false;
	}
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

void game::visible_ships(list<ship*>& result, const sea_object* o)
{
	const sensor* s = o->get_sensor ( o->lookout_system );
	const lookout_sensor* ls = 0;

	if ( s )
		ls = dynamic_cast<const lookout_sensor*> ( s );

	if ( ls )
	{
		for (list<ship*>::iterator it = ships.begin(); it != ships.end(); ++it)
		{
			if ( ls->is_detected ( this, o, *it ) )
				result.push_back (*it);
		}
	}
}

// give relative position, length*vis, width*vis and course
bool is_in_ellipse(const vector2& p, double xl, double yl, angle& head)
{
	vector2 hd = head.direction();
	double t1 = (p.x*hd.x + p.y*hd.y);
	double t2 = (p.y*hd.x - p.x*hd.y);
	return ((t1*t1)/(xl*xl) + (t2*t2)/(yl*yl)) < 1;
}

void game::visible_submarines(list<submarine*>& result, const sea_object* o)
{
	const sensor* s = o->get_sensor ( o->lookout_system );
	const lookout_sensor* ls = 0;

	if ( s )
		ls = dynamic_cast<const lookout_sensor*> ( s );

	if ( ls )
	{
		const lookout_sensor* ls = dynamic_cast<const lookout_sensor*> ( s );
		for (list<submarine*>::iterator it = submarines.begin();
			it != submarines.end(); ++it)
		{
			if ( ls->is_detected ( this, o, *it ) )
				result.push_back (*it);
		}
	}
}

void game::visible_airplanes(list<airplane*>& result, const sea_object* o)
{
	const sensor* s = o->get_sensor ( o->lookout_system );
	const lookout_sensor* ls = 0;

	if ( s )
		ls = dynamic_cast<const lookout_sensor*> ( s );

	if ( ls )
	{
		
		for (list<airplane*>::iterator it = airplanes.begin();
			it != airplanes.end(); ++it)
		{
			if ( ls->is_detected ( this, o, *it ) )
				result.push_back (*it);
		}
	}
}

void game::visible_torpedoes(list<torpedo*>& result, const sea_object* o)
{
	const sensor* s = o->get_sensor ( o->lookout_system );
	const lookout_sensor* ls = 0;
	
	if ( s )
		ls = dynamic_cast<const lookout_sensor*> ( s );

	if ( ls )
	{
		for (list<torpedo*>::iterator it = torpedoes.begin();
			it != torpedoes.end(); ++it)
		{
			if ( ls->is_detected ( this, o, *it ) )
				result.push_back (*it);
		}
	}
}

void game::visible_depth_charges(list<depth_charge*>& result, const sea_object* o)
{
	const sensor* s = o->get_sensor ( o->lookout_system );
	const lookout_sensor* ls = 0;

	if ( s )
		ls = dynamic_cast<const lookout_sensor*> ( s );

	if ( ls )
	{
		for (list<depth_charge*>::iterator it = depth_charges.begin();
			it != depth_charges.end(); ++it)
		{
			if ( ls->is_detected ( this, o, *it ) )
				result.push_back (*it);
		}
	}
}

void game::visible_gun_shells(list<gun_shell*>& result, const sea_object* o)
{
	const sensor* s = o->get_sensor ( o->lookout_system );
	const lookout_sensor* ls = 0;

	if ( s )
		ls = dynamic_cast<const lookout_sensor*> ( s );

	if ( ls )
	{
		for (list<gun_shell*>::iterator it = gun_shells.begin();
			it != gun_shells.end(); ++it)
		{
			if ( ls->is_detected ( this, o, *it ) )
				result.push_back (*it);
		}
	}
}

void game::sonar_ships ( list<ship*>& result, const sea_object* o )
{
	const sensor* s = o->get_sensor ( o->passive_sonar_system );
	const passive_sonar_sensor* pss = 0;

	if ( s )
		pss = dynamic_cast<const passive_sonar_sensor*> ( s );

	if ( pss )
	{
		// collect the nearest contacts, limited to some value!
		vector<pair<double, ship*> > contacts ( MAX_ACUSTIC_CONTACTS, make_pair ( 1e30, (ship*) 0 ) );

		for ( list<ship*>::iterator it = ships.begin (); it != ships.end (); ++it)
		{
			// When the detecting unit is a ship it should not detect itself.
			if ( o == (*it) )
				continue;

			double d = (*it)->get_pos ().xy ().square_distance ( o->get_pos ().xy () );
			unsigned i = 0;
			for ( ; i < contacts.size (); ++i )
			{
				if ( contacts[i].first > d )
					break;
			}

			if ( i < contacts.size () )
			{
				for ( unsigned j = contacts.size ()-1; j > i; --j )
					contacts[j] = contacts[j-1];

				contacts[i] = make_pair ( d, *it );
			}
		}

		unsigned size = contacts.size ();
		for (unsigned i = 0; i < size; i++ )
		{
			ship* sh = contacts[i].second;
			if ( sh == 0 )
				break;

			if ( pss->is_detected ( this, o, sh ) )
				result.push_back ( sh );
		}
    }
}

void game::sonar_submarines ( list<submarine*>& result, const sea_object* o )
{
	const sensor* s = o->get_sensor ( o->passive_sonar_system );
	const passive_sonar_sensor* pss = 0;

	if ( s )
		pss = dynamic_cast<const passive_sonar_sensor*> ( s );

	if ( pss )
	{
		for (list<submarine*>::iterator it = submarines.begin ();
			it != submarines.end (); it++ )
		{
			// When the detecting unit is a submarine it should not
			// detect itself.
			if ( o == (*it) )
				continue;

			if ( pss->is_detected ( this, o, *it ) )
				result.push_back (*it);
		}
	}
}

void game::convoy_positions(list<vector2>& result) const
{
	for (list<convoy*>::const_iterator it = convoys.begin(); it != convoys.end(); ++it) {
		result.push_back((*it)->get_pos().xy());
	}
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

void game::spawn_gun_shell(gun_shell* s)
{
	gun_shells.push_back(s);
}

void game::spawn_depth_charge(depth_charge* dc)
{
	if (ui) ui->add_message(texts::get(205));	// fixme: only if player is near enough
	depth_charges.push_back(dc);
}

void game::spawn_convoy(convoy* cv)
{
	convoys.push_back(cv);
}

void game::spawn_water_splash ( water_splash* ws )
{
	water_splashs.push_back ( ws );
}



void game::dc_explosion(const depth_charge& dc)
{
	// Create water splash.
	spawn_water_splash(new depth_charge_water_splash(dc.get_pos()));

	// are subs affected?
	// fixme: ships can be damaged by DCs also...
	// fixme: ai should not be able to release dcs with a depth less than 30m or so, to
	// avoid suicide
	for (list<submarine*>::iterator it = submarines.begin(); it != submarines.end(); ++it) {
		(*it)->depth_charge_explosion(dc);
		if (*it == player) {
			// play detonation sound, volume depends on distance
		}
	}
	if (ui) ui->add_message(texts::get(204));	// fixme: only when player is near enough
}

bool game::gs_impact(const vector3& pos)	// fixme: vector2 would be enough
{
	// Create water splash. // FIXME
	spawn_water_splash ( new gun_shell_water_splash ( pos ) );
//	return false;//fixme testing
	for (list<ship*>::iterator it = ships.begin(); it != ships.end(); ++it) {
		// fixme: we need a special collision detection, because
		// the shell is so fast that it can be collisionfree with *it
		// delta_time ago and now, but hit *it in between
		if (is_collision(*it, pos.xy())) {
			(*it)->damage((*it)->get_pos() /*fixme*/,GUN_SHELL_HITPOINTS);
			return true;	// only one hit possible
		}
	}
	for (list<submarine*>::iterator it = submarines.begin(); it != submarines.end(); ++it) {
//printf("sub %f %f %f\n",(*it)->get_pos().x,(*it)->get_pos().y,(*it)->get_pos().z);
//printf("gun %f %f %f\n",pos.x,pos.y,pos.z);
		if (is_collision(*it, pos.xy())) {
//printf("sub damaged!!!\n");		
			(*it)->damage((*it)->get_pos() /*fixme*/,GUN_SHELL_HITPOINTS);
			return true; // only one hit possible
		}
	}

	// No impact.
	spawn_water_splash ( new gun_shell_water_splash ( pos ) );
	return false;
}

void game::torp_explode(const vector3& pos)
{
	spawn_water_splash ( new torpedo_water_splash ( pos ) );
	if (ui) ui->play_sound_effect_distance ( ui->se_torpedo_detonation,
		player->get_pos ().distance ( pos ) );
}

void game::ship_sunk( const ship* s )
{
	if (ui) ui->add_message ( texts::get(83) );
	ostringstream oss;
	oss << texts::get(83) << " " << s->get_description ( 2 );
	if (ui) ui->add_captains_log_entry( *this, oss.str () );
	if (ui) ui->record_sunk_ship ( s );
	date d;
	::get_date(time, d);
	sunken_ships.push_back(sink_record(d, s->get_description(2),
		s->get_tonnage()));
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
		pings.push_back ( ping ( d->get_pos ().xy (),
			ass->get_bearing () + d->get_heading (), time,
			ass->get_range (), ass->get_detection_cone () ) );

		// fixme: noise from ships can disturb ASDIC or may generate more contacs.
		// ocean floor echoes ASDIC etc...
		for ( list<submarine*>::iterator it = submarines.begin ();
			it != submarines.end (); ++it )
		{
			if ( ass->is_detected ( this, d, *it ) )
			{
				contacts.push_back((*it)->get_pos () +
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
	system::sys().myassert(false, "[game::unregister_job] job not found in list");
}

template<class _C>
ship* game::check_unit_list ( torpedo* t, list<_C>& unit_list )
{
	for ( typename list<_C>::iterator it = unit_list.begin (); it != unit_list.end (); ++it )
	{
		if ( is_collision ( t, *it ) )
			return *it;
	}

	return 0;
}

bool game::check_torpedo_hit(torpedo* t, bool runlengthfailure, bool failure)
{
	if (failure) {
		if (ui) ui->add_message(texts::get(60));
		return true;
	}

	ship* s = check_unit_list ( t, ships );

	if ( !s )
		s = check_unit_list ( t, submarines );

	if ( s ) {
		if (runlengthfailure) {
			if (ui) ui->add_message(texts::get(59));
		} else {
		// Only ships that are alive or defunct can be sunk. Already sinking
		// or destroyed ships cannot be destroyed again.
			if ( ( s->is_alive () || s->is_defunct () ) &&
				s->damage ( t->get_pos (), t->get_hit_points () ) )
				ship_sunk ( s );
			torp_explode ( t->get_pos () );
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
		for (list<ship*>::iterator it = ships.begin(); it != ships.end(); ++it)
		{
			// Only a visible and intact submarine can be selected.
			if ( ls->is_detected ( this, o, (*it) ) &&
				( (*it)->is_alive () || (*it)->is_defunct () ) )
			{
				vector2 df = (*it)->get_pos().xy () - o->get_pos().xy ();
				double new_ang_diff = (angle(df)).diff(direction);
				if (new_ang_diff < angle_diff)
				{
					angle_diff = new_ang_diff;
					result = *it;
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
		for (list<submarine*>::iterator it = submarines.begin();
			it != submarines.end(); ++it)
		{
			// Only a visible and intact submarine can be selected.
			if ( ls->is_detected ( this, o, (*it) ) &&
				( (*it)->is_alive () || (*it)->is_defunct () ) )
			{
				vector2 df = (*it)->get_pos ().xy () - o->get_pos(). xy();
				double new_ang_diff = (angle(df)).diff(direction);
				if (new_ang_diff < angle_diff)
				{
					angle_diff = new_ang_diff;
					result = *it;
				}
			}
		}
	}
	return result;
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

void game::visible_water_splashes ( list<water_splash*>& result, const sea_object* o )
{
	const sensor* s = o->get_sensor ( o->lookout_system );
	const lookout_sensor* ls = 0;

	if ( s )
		ls = dynamic_cast<const lookout_sensor*> ( s );

	if ( ls )
	{
		for (list<water_splash*>::iterator it = water_splashs.begin();
			it != water_splashs.end(); ++it)
		{
			if ( ls->is_detected ( this, o, *it ) )
				result.push_back (*it);
		}
	}
}

ship* game::sonar_acoustical_torpedo_target ( const torpedo* o )
{
	ship* loudest_object = 0;
	double loudest_object_sf = 0.0f;
	const sensor* s = o->get_sensor ( o->passive_sonar_system );
	const passive_sonar_sensor* pss = 0;

	if ( s )
		pss = dynamic_cast<const passive_sonar_sensor*> ( s );

    if ( pss )
    {
		for ( list<ship*>::iterator it = ships.begin (); it != ships.end (); it++ )
		{
			double sf = 0.0f;
			if ( pss->is_detected ( sf, this, o, *it ) )
			{
				if ( sf > loudest_object_sf )
				{
					loudest_object_sf = sf;
					loudest_object = *it;
				}
			}
		}

		for ( list<submarine*>::iterator it = submarines.begin ();
			it != submarines.end (); it++ )
		{
			double sf = 0.0f;
			if ( pss->is_detected ( sf, this, o, *it ) )
			{
				if ( sf > loudest_object_sf )
				{
					loudest_object_sf = sf;
					loudest_object = *it;
				}
			}
		}
	}

	return loudest_object;
}

// main play loop
// fixme: a bit misplaced here, especially after ui was moved away from game
unsigned game::exec(void)
{
	class system& sys = system::sys();
	unsigned frames = 1;
	unsigned lasttime = sys.millisec();
	unsigned lastframes = 1;
	double fpstime = 0;
	double totaltime = 0;
	double measuretime = 5;	// seconds

	stopexec = false;
	
	// draw one initial frame
	system::sys().myassert(ui != 0, "game::exec() called for a game without an user_interface");
	ui->display(*this);
	
	while (running && !stopexec) {
		sys.poll_event_queue();

		// this time_scaling is bad. hits may get computed wrong when time
		// scaling is too high. fixme
		unsigned thistime = sys.millisec();
		double delta_time = (thistime - lasttime)/1000.0 * ui->time_scaling();
		totaltime += (thistime - lasttime)/1000.0;
		lasttime = thistime;
		
		// next simulation step
		if (!ui->paused()) {
			simulate(delta_time);
		}

		ui->display(*this);
		++frames;

		// record fps
		if (totaltime - fpstime >= measuretime) {
			fpstime = totaltime;
			ostringstream os;
			os << "$c0fffffps " << (frames - lastframes)/measuretime;
			lastframes = frames;
			sys.add_console(os.str());
		}
		
		sys.swap_buffers();
	}
	
	return (running ? 0 : 1);	// if player is killed, end game (1), else show menu (0)
}

bool game::is_day_mode () const
{
	double br = compute_light_brightness(player->get_pos());
	return (br > 0.0);
}

template <class T>
T* ith_elem(const list<T*>& lst, unsigned i)
{
	for (typename list<T*>::const_iterator it = lst.begin(); it != lst.end(); ++it, --i)
		if (i == 0)
			return *it;
	system::sys().myassert(false, "weird: could not access i'th element");
	return 0;
}

template <class T>
void write_ptr2u16(ostream& out, const T* p, const list<T*>& lst, unsigned offset, const string& ptrtype)
{
	if (!p) {
//cout<<"write #late "<<0<<"\n";
		write_u16(out, 0);
		return;
	}
	typename list<T*>::const_iterator it = lst.begin();
	for (unsigned i = 1; it != lst.end(); ++i, ++it) {
		if (*it == p) {
//cout<<"write #late "<<i + offset<<"\n";
			write_u16(out, i + offset);
			return;
		}
	}
	system::sys().myassert(false, string("could not translate ") + ptrtype + " pointer to number");
}

template <class T>
T* read_u162ptr(istream& in, const list<T*>& lst, unsigned n0, unsigned n1, const string& ptrtype)
{
	unsigned nr = read_u16(in);
//cout <<"read #late "<<nr<<"\n";
	if (nr == 0) return 0;
	if (nr > n0 && nr <= n1) {
		return ith_elem(lst, nr-n0-1);
	}
	system::sys().myassert(false, string("could not translate number to ") + ptrtype + " pointer");
	return 0;
}

unsigned game::listsizes(unsigned n) const
{
	unsigned s = 0;
	switch (n) {
		case 8: s += water_splashs.size();
		case 7: s += convoys.size();
		case 6: s += gun_shells.size();
		case 5: s += depth_charges.size();
		case 4: s += torpedoes.size();
		case 3: s += airplanes.size();
		case 2: s += submarines.size();
		case 1: s += ships.size();
		case 0: s += 0; break;
		default: system::sys().myassert(false, "game::listsizes  n too high");
	}
	return s;
}

void game::receive_commands(void)
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



double game::compute_light_brightness(const vector3& viewpos) const
{
	vector3 sundir = compute_sun_pos(viewpos).normal();
	// in reality the brightness is equal to sundir.z, but the sun is so bright that
	// we stretch and clamp this value
	double lightbrightness = sundir.z * 2.0;
	if (lightbrightness > 1.0) lightbrightness = 1.0;
	if (lightbrightness < 0.0) lightbrightness = 0.0;
	//fixme add moon light at night
	return lightbrightness * 0.8 + 0.2;	// some ambient value
}



color game::compute_light_color(const vector3& viewpos) const
{
	// fixme: sun color can be yellow/orange at dusk/dawn
	Uint8 lc = Uint8(255*compute_light_brightness(viewpos));
	return color(lc, lc, lc);
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
	return sun2earth.column(3);
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

	return moon2earth.column(3);
}



void game::write(ostream& out, const ship* s) const
{
	write_ptr2u16(out, s, ships, listsizes(0), "ship");
}

void game::write(ostream& out, const submarine* s) const
{
	write_ptr2u16(out, s, submarines, listsizes(1), "submarine");
}

void game::write(ostream& out, const airplane* a) const
{
	write_ptr2u16(out, a, airplanes, listsizes(2), "airplane");
}

void game::write(ostream& out, const torpedo* t) const
{
	write_ptr2u16(out, t, torpedoes, listsizes(3), "torpedo");
}

void game::write(ostream& out, const depth_charge* d) const
{
	write_ptr2u16(out, d, depth_charges, listsizes(4), "depth_charge");
}

void game::write(ostream& out, const gun_shell* q) const
{
	write_ptr2u16(out, q, gun_shells, listsizes(5), "gun_shell");
}

void game::write(ostream& out, const convoy* c) const
{
	write_ptr2u16(out, c, convoys, listsizes(6), "convoy");
}

void game::write(ostream& out, const water_splash* w) const
{
	write_ptr2u16(out, w, water_splashs, listsizes(7), "water_splash");
}

void game::write(ostream& out, const sea_object* s) const
{
	if (s == 0) { write_u16(out, 0); return; }
	// note! we have to test submarine first, because each submarine is also a ship, but
	// the write() functions expect the same class type, not a heir!
	// calling write(ostream&, ship* s) with submarine type s will fail!
	const submarine* su = dynamic_cast<const submarine*>(s); if (su) { write(out, su); return; }
	const ship* sh = dynamic_cast<const ship*>(s); if (sh) { write(out, sh); return; }
	const airplane* ap = dynamic_cast<const airplane*>(s); if (ap) { write(out, ap); return; }
	const torpedo* tp = dynamic_cast<const torpedo*>(s); if (tp) { write(out, tp); return; }
	const depth_charge* dc = dynamic_cast<const depth_charge*>(s); if (dc) { write(out, dc); return; }
	const gun_shell* gs = dynamic_cast<const gun_shell*>(s); if (gs) { write(out, gs); return; }
	const convoy* cv = dynamic_cast<const convoy*>(s); if (cv) { write(out, cv); return; }
	const water_splash* ws = dynamic_cast<const water_splash*>(s); if (ws) { write(out, ws); return; }
	system::sys().myassert(false, "internal error: ptr is not 0 and no heir of sea_object");
}

ship* game::read_ship(istream& in) const
{
	return read_u162ptr(in, ships, listsizes(0), listsizes(1), "ship");
}

submarine* game::read_submarine(istream& in) const
{
	return read_u162ptr(in, submarines, listsizes(1), listsizes(2), "submarine");
}

airplane* game::read_airplane(istream& in) const
{
	return read_u162ptr(in, airplanes, listsizes(2), listsizes(3), "airplane");
}

torpedo* game::read_torpedo(istream& in) const
{
	return read_u162ptr(in, torpedoes, listsizes(3), listsizes(4), "torpedo");
}

depth_charge* game::read_depth_charge(istream& in) const
{
	return read_u162ptr(in, depth_charges, listsizes(4), listsizes(5), "depth_charge");
}

gun_shell* game::read_gun_shell(istream& in) const
{
	return read_u162ptr(in, gun_shells, listsizes(5), listsizes(6), "gun_shell");
}

convoy* game::read_convoy(istream& in) const
{
	return read_u162ptr(in, convoys, listsizes(6), listsizes(7), "convoy");
}

water_splash* game::read_water_splash(istream& in) const
{
	return read_u162ptr(in, water_splashs, listsizes(7), listsizes(8), "water_splash");
}

sea_object* game::read_sea_object(istream& in) const
{
	unsigned nr = read_u16(in);
//cout << "xlate nr "<<nr<<"\n";
	if (nr == 0) return 0;
	unsigned sizes[9];
	for (unsigned typen = 0; typen <= 8; ++typen)
{
//cout<<"typen "<<typen<<" sizes "<<listsizes(typen)<<"\n";	
		sizes[typen] = listsizes(typen);
}
	if (nr > sizes[0] && nr <= sizes[1]) return ith_elem(ships, nr-sizes[0]-1);
	if (nr > sizes[1] && nr <= sizes[2]) return ith_elem(submarines, nr-sizes[1]-1);
	if (nr > sizes[2] && nr <= sizes[3]) return ith_elem(airplanes, nr-sizes[2]-1);
	if (nr > sizes[3] && nr <= sizes[4]) return ith_elem(torpedoes, nr-sizes[3]-1);
	if (nr > sizes[4] && nr <= sizes[5]) return ith_elem(depth_charges, nr-sizes[4]-1);
	if (nr > sizes[5] && nr <= sizes[6]) return ith_elem(gun_shells, nr-sizes[5]-1);
	if (nr > sizes[6] && nr <= sizes[7]) return ith_elem(convoys, nr-sizes[6]-1);
	if (nr > sizes[7] && nr <= sizes[8]) return ith_elem(water_splashs, nr-sizes[7]-1);
	system::sys().myassert(false, string("could not translate number to sea_object pointer"));
	return 0;
}
