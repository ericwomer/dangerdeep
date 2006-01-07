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

// this class holds the game's configuration
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef CFG_H
#define CFG_H

#include <map>
#include <string>
using namespace std;

#include <SDL.h>

/** This class stores and manages the global game configuration. */
///\todo replace tinyxml code and include with own xml class.
class cfg
{
public:
	/** Documentation is build of keys. Each key has a name and an value. */
	struct key
	{
		string action;
		SDLKey keysym;
		bool ctrl, alt, shift;
		key() : keysym(SDLK_UNKNOWN), ctrl(false), alt(false), shift(false) {}
		~key() {}
		key(const string& ac, SDLKey ks, bool c, bool a, bool s) :
			action(ac), keysym(ks), ctrl(c), alt(a), shift(s) {}
		key(const key& k) : action(k.action), keysym(k.keysym), ctrl(k.ctrl), alt(k.alt), shift(k.shift) {}
		key& operator= (const key& k) { action = k.action; keysym = k.keysym; ctrl = k.ctrl; alt = k.alt;
			shift = k.shift; return *this; }
		string get_name(void) const; // uses SDLK_GetKeyName
		bool equal(const SDL_keysym& ks) const;
	};
private:
	cfg(const cfg& );
	cfg& operator= (const cfg& );

	map<string, bool> valb;
	map<string, int> vali;
	map<string, float> valf;
	map<string, string> vals;
	map<unsigned, key> valk;
	
	static cfg* myinst;
	
	cfg();
	
	// set dependent on registered type, used by load() and parse(), returns false if name
	// is unknown
	bool set_str(const string& name, const string& value);

public:
	static cfg& instance(void);
	~cfg();
	
	// load the values from a config file. Note! register() calls must be happen
	// *before* loading the values!
	void load(const string& filename);
	void save(const string& filename) const;
	
	void register_option(const string& name, bool value);
	void register_option(const string& name, int value);
	void register_option(const string& name, float value);
	void register_option(const string& name, const string& value);
	void register_key(const string& name, SDLKey keysym, bool ctrl, bool alt, bool shift);

	void set(const string& name, bool value);
	void set(const string& name, int value);
	void set(const string& name, float value);
	void set(const string& name, const string& value);
	void set_key(unsigned nr, SDLKey keysym, bool ctrl, bool alt, bool shift);
	
	bool getb(const string& name) const;
	int geti(const string& name) const;
	float getf(const string& name) const;
	string gets(const string& name) const;
	key getkey(unsigned nr) const;
	
	void parse_value(const string& s);	// give elements of command line array to it!
};

#endif
