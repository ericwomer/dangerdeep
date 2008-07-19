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

#include "cfg.h"
#include "keys.h"
#include "global_data.h"
#include "system.h"
#include "log.h"
#include <sstream>
#include "xml.h"
using namespace std;



cfg* cfg::myinst = 0;

string cfg::key::get_name() const
{
	string result = SDL_GetKeyName(SDLKey(keysym));
	if (shift) result = string("Shift + ") + result;
	if (alt) result = string("Alt + ") + result;
	if (ctrl) result = string("Ctrl + ") + result;
	return result;
}



bool cfg::key::equal(const SDL_keysym& ks) const
{
	return (ks.sym == keysym
		&& ctrl == ((ks.mod & (KMOD_LCTRL | KMOD_RCTRL)) != 0)
		&& alt == ((ks.mod & (KMOD_LALT | KMOD_RALT | KMOD_MODE)) != 0)
		&& shift == ((ks.mod & (KMOD_LSHIFT | KMOD_RSHIFT)) != 0));
}



cfg::cfg()
{
}



cfg::~cfg()
{
}



bool cfg::set_str(const string& name, const string& value)
{
	map<string, bool>::iterator it = valb.find(name);
	if (it != valb.end()) {
		if (value == "true" || value == "yes") it->second = true;
		else if (value == "false" || value == "no") it->second = false;
		else it->second = bool(atoi(value.c_str()));
	} else {
		map<string, int>::iterator it = vali.find(name);
		if (it != vali.end()) {
			it->second = atoi(value.c_str());
		} else {
			map<string, float>::iterator it = valf.find(name);
			if (it != valf.end()) {
				it->second = atof(value.c_str());
			} else {
				map<string, string>::iterator it = vals.find(name);
				if (it != vals.end()) {
					it->second = value;
				} else {
					return false;
				}
			}
		}
	}
	return true;
}



void cfg::load(const string& filename)
{
	xml_doc doc(filename);
	doc.load();
	xml_elem root = doc.child("dftd-cfg");
	for (xml_elem::iterator it = root.iterate(); !it.end(); it.next()) {
		if (it.elem().get_name() == "keys") {
			for (xml_elem::iterator it2 = it.elem().iterate("key"); !it2.end(); it2.next()) {
				string keyname = it2.elem().attr("action");
				// get key number for this action from table
				unsigned nr = NR_OF_KEY_IDS;
				for (unsigned i = 0; i < NR_OF_KEY_IDS; ++i) {
					if (string(key_names[i].name) == keyname) {
						nr = i;
					}
				}
				if (nr == NR_OF_KEY_IDS) {
					log_warning("found key with invalid name " << keyname << " in config file");
					continue;
				}
				SDLKey keysym = SDLKey(it2.elem().attri("keysym"));
				bool ctrl = it2.elem().attrb("ctrl");
				bool alt = it2.elem().attrb("alt");
				bool shift = it2.elem().attrb("shift");
				set_key(nr, keysym, ctrl, alt, shift);
			}
		} else {
			bool found = set_str(it.elem().get_name(), it.elem().attr());
			if (!found)
				log_warning("config option not registered: " << it.elem().get_name());
		}
	}
}



void cfg::save(const string& filename) const
{
	xml_doc doc(filename);
	xml_elem root = doc.add_child("dftd-cfg");
	for (map<string, bool>::const_iterator it = valb.begin(); it != valb.end(); ++it) {
		root.add_child(it->first).set_attr(it->second);
	}
	for (map<string, int>::const_iterator it = vali.begin(); it != vali.end(); ++it) {
		root.add_child(it->first).set_attr(it->second);
	}
	for (map<string, float>::const_iterator it = valf.begin(); it != valf.end(); ++it) {
		root.add_child(it->first).set_attr(it->second);
	}
	for (map<string, string>::const_iterator it = vals.begin(); it != vals.end(); ++it) {
		root.add_child(it->first).set_attr(it->second);
	}
	xml_elem keys = root.add_child("keys");
	for (map<unsigned, key>::const_iterator it = valk.begin(); it != valk.end(); ++it) {
		xml_elem key = keys.add_child("key");
		key.set_attr(it->second.action, "action");
		key.set_attr(int(it->second.keysym), "keysym");
		key.set_attr(it->second.ctrl, "ctrl");
		key.set_attr(it->second.alt, "alt");
		key.set_attr(it->second.shift, "shift");
	}
	doc.save();
}



void cfg::register_option(const string& name, bool value)
{
	valb[name] = value;
}



void cfg::register_option(const string& name, int value)
{
	vali[name] = value;
}



void cfg::register_option(const string& name, float value)
{
	valf[name] = value;
}



void cfg::register_option(const string& name, const string& value)
{
	vals[name] = value;
}



void cfg::register_key(const string& name, SDLKey keysym, bool ctrl, bool alt, bool shift)
{
	unsigned nr = NR_OF_KEY_IDS;
	for (unsigned i = 0; i < NR_OF_KEY_IDS; ++i) {
		if (string(key_names[i].name) == name) {
			nr = i;
		}
	}
	if (nr == NR_OF_KEY_IDS)
		throw error(string("register_key with invalid name ")+ name);
	valk[nr] = key(name, keysym, ctrl, alt, shift);
}



void cfg::set(const string& name, bool value)
{
	map<string, bool>::iterator it = valb.find(name);
	if (it != valb.end())
		it->second = value;
	else
		throw error(string("cfg: set(), name not registered: ") + name);
}



void cfg::set(const string& name, int value)
{
	map<string, int>::iterator it = vali.find(name);
	if (it != vali.end())
		it->second = value;
	else
		throw error(string("cfg: set(), name not registered: ") + name);
}



void cfg::set(const string& name, float value)
{
	map<string, float>::iterator it = valf.find(name);
	if (it != valf.end())
		it->second = value;
	else
		throw error(string("cfg: set(), name not registered: ") + name);
}



void cfg::set(const string& name, const string& value)
{
	map<string, string>::iterator it = vals.find(name);
	if (it != vals.end())
		it->second = value;
	else
		throw error(string("cfg: set(), name not registered: ") + name);
}


	
void cfg::set_key(unsigned nr, SDLKey keysym, bool ctrl, bool alt, bool shift)
{
	map<unsigned, key>::iterator it = valk.find(nr);
	if (it != valk.end())
		it->second = key(it->second.action, keysym, ctrl, alt, shift);
	else
		throw error(string("cfg: set_key(), key number not registered: "));
}



bool cfg::getb(const string& name) const
{
	map<string, bool>::const_iterator it = valb.find(name);
	if (it != valb.end())
		return it->second;
	else
		throw error(string("cfg: get(), name not registered: ") + name);
	return 0;
}



int cfg::geti(const string& name) const
{
	map<string, int>::const_iterator it = vali.find(name);
	if (it != vali.end())
		return it->second;
	else
		throw error(string("cfg: get(), name not registered: ") + name);
	return 0;
}



float cfg::getf(const string& name) const
{
	map<string, float>::const_iterator it = valf.find(name);
	if (it != valf.end())
		return it->second;
	else
		throw error(string("cfg: get(), name not registered: ") + name);
	return 0;
}



string cfg::gets(const string& name) const
{
	map<string, string>::const_iterator it = vals.find(name);
	if (it != vals.end())
		return it->second;
	else
		throw error(string("cfg: get(), name not registered: ") + name);
	return 0;
}



cfg::key cfg::getkey(unsigned nr) const
{
	map<unsigned, key>::const_iterator it = valk.find(nr);
	if (it != valk.end())
		return it->second;
	else
		throw error(string("cfg: getkey(), key number not registered: "));
	return key();
}



// format of s must be --name=value or --name or --noname for bool values.
void cfg::parse_value(const string& s)
{
	// fixme: ignore values for unregistered names?
	if (s.length() < 3 || s[0] != '-' || s[1] != '-') return;	// ignore it
	string::size_type st = s.find("=");
	string s0, s1;
	if (st == string::npos) {
		if (s.substr(2, 2) == "no") {
			s0 = s.substr(4);
			s1 = "false";
		} else {
			s0 = s.substr(2);
			s1 = "true";
		}
	} else {
		s0 = s.substr(2, st-2);
		s1 = s.substr(st+1);
	}
	set_str(s0, s1);	// ignore value if name is unkown
}
