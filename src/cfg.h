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
#include <SDL.h>
#include "singleton.h"

///\brief This class stores and manages the global game configuration.
///\todo replace tinyxml code and include with own xml class.
class cfg : public singleton<class cfg>
{
	friend class singleton<cfg>;
public:
	///\brief Documentation is build of keys. Each key has a name and an value.
	struct key
	{
		std::string action;
		SDLKey keysym;
		bool ctrl, alt, shift;
		key() : keysym(SDLK_UNKNOWN), ctrl(false), alt(false), shift(false) {}
		~key() {}
		key(const std::string& ac, SDLKey ks, bool c, bool a, bool s) :
			action(ac), keysym(ks), ctrl(c), alt(a), shift(s) {}
		key(const key& k) : action(k.action), keysym(k.keysym), ctrl(k.ctrl), alt(k.alt), shift(k.shift) {}
		key& operator= (const key& k) { action = k.action; keysym = k.keysym; ctrl = k.ctrl; alt = k.alt;
			shift = k.shift; return *this; }
		std::string get_name() const; // uses SDLK_GetKeyName
		bool equal(const SDL_keysym& ks) const;
	};
private:
	cfg(const cfg& );
	cfg& operator= (const cfg& );

	std::map<std::string, bool> valb;
	std::map<std::string, int> vali;
	std::map<std::string, float> valf;
	std::map<std::string, std::string> vals;
	std::map<unsigned, key> valk;
	
	static cfg* myinst;
	
	cfg();
	
	// set dependent on registered type, used by load() and parse(), returns false if name
	// is unknown
	bool set_str(const std::string& name, const std::string& value);

public:
	~cfg();
	
	// load the values from a config file. Note! register() calls must be happen
	// *before* loading the values!
	void load(const std::string& filename);
	void save(const std::string& filename) const;
	
	void register_option(const std::string& name, bool value);
	void register_option(const std::string& name, int value);
	void register_option(const std::string& name, float value);
	void register_option(const std::string& name, const std::string& value);
	void register_key(const std::string& name, SDLKey keysym, bool ctrl, bool alt, bool shift);

	void set(const std::string& name, bool value);
	void set(const std::string& name, int value);
	void set(const std::string& name, float value);
	void set(const std::string& name, const std::string& value);
	void set_key(unsigned nr, SDLKey keysym, bool ctrl, bool alt, bool shift);
	
	bool getb(const std::string& name) const;
	int geti(const std::string& name) const;
	float getf(const std::string& name) const;
	std::string gets(const std::string& name) const;
	key getkey(unsigned nr) const;
	
	void parse_value(const std::string& s);	// give elements of command line array to it!
};

#endif
