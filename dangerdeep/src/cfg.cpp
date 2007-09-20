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
#include "tinyxml/tinyxml.h"
#include "global_data.h"
#include "system.h"
#include "log.h"
#include <sstream>
#include "xml.h"
using namespace std;



cfg* cfg::myinst = 0;


// later: remove that crap when tinyxml is not used directly any longer
string XmlAttrib(class TiXmlElement* elem, const char* attrname)
{
	const char* tmp = elem->Attribute(attrname);
	if (tmp) return string(tmp);
	return string();
}
unsigned XmlAttribu(class TiXmlElement* elem, const char* attrname)
{
	return unsigned(atoi(XmlAttrib(elem, attrname).c_str()));
}
float XmlAttribf(class TiXmlElement* elem, const char* attrname)
{
	return float(atof(XmlAttrib(elem, attrname).c_str()));
}


	
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



cfg& cfg::instance()
{
	if (!myinst) myinst = new cfg();
	return *myinst;
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
	// test hack - later all xml code should be changed, so that it uses xml.h only
	// and not tinyxml
	/*
	xml_doc doc2(filename);
	doc2.load();
	xml_elem root2 = doc2.child("dftd-cfg");
	*/

	TiXmlDocument doc(filename);
	doc.LoadFile();
	TiXmlElement* root = doc.FirstChildElement("dftd-cfg");
	sys().myassert(root != 0, string("cfg: load(), no root element found in ") + filename);
	TiXmlElement* eattr = root->FirstChildElement();
	for ( ; eattr != 0; eattr = eattr->NextSiblingElement()) {
		if (string(eattr->Value()) == "keys") {
			// read keys
			TiXmlElement* ke = eattr->FirstChildElement();
			for ( ; ke != 0; ke = ke->NextSiblingElement()) {
				string keyname = XmlAttrib(ke, "action");
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
				SDLKey keysym = SDLKey(XmlAttribu(ke, "keysym"));
				bool ctrl = XmlAttribu(ke, "ctrl") != 0;
				bool alt = XmlAttribu(ke, "alt") != 0;
				bool shift = XmlAttribu(ke, "shift") != 0;
				set_key(nr, keysym, ctrl, alt, shift);
			}
		} else {
			string attrstr = XmlAttrib(eattr, "value");
			bool found = set_str(eattr->Value(), attrstr);
			if (!found)
				log_warning("config option not registered: " << eattr->Value());
		}
	}
}



void cfg::save(const string& filename) const
{
	TiXmlDocument doc(filename);
	TiXmlElement* root = new TiXmlElement("dftd-cfg");
	doc.LinkEndChild(root);
	for (map<string, bool>::const_iterator it = valb.begin(); it != valb.end(); ++it) {
		TiXmlElement* attr = new TiXmlElement(it->first);
		attr->SetAttribute("value", (it->second) ? "true" : "false");
		root->LinkEndChild(attr);
	}
	for (map<string, int>::const_iterator it = vali.begin(); it != vali.end(); ++it) {
		TiXmlElement* attr = new TiXmlElement(it->first);
		ostringstream oss;
		oss << it->second;
		attr->SetAttribute("value", oss.str());
		root->LinkEndChild(attr);
	}
	for (map<string, float>::const_iterator it = valf.begin(); it != valf.end(); ++it) {
		TiXmlElement* attr = new TiXmlElement(it->first);
		ostringstream oss;
		oss << it->second;
		attr->SetAttribute("value", oss.str());
		root->LinkEndChild(attr);
	}
	for (map<string, string>::const_iterator it = vals.begin(); it != vals.end(); ++it) {
		TiXmlElement* attr = new TiXmlElement(it->first);
		attr->SetAttribute("value", it->second);
		root->LinkEndChild(attr);
	}
	TiXmlElement* keys = new TiXmlElement("keys");
	root->LinkEndChild(keys);
	for (map<unsigned, key>::const_iterator it = valk.begin(); it != valk.end(); ++it) {
		TiXmlElement* attr = new TiXmlElement("key");
		attr->SetAttribute("action", it->second.action);
		attr->SetAttribute("keysym", it->second.keysym);
		attr->SetAttribute("ctrl", it->second.ctrl);
		attr->SetAttribute("alt", it->second.alt);
		attr->SetAttribute("shift", it->second.shift);
		keys->LinkEndChild(attr);
	}
	doc.SaveFile();
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
	sys().myassert(nr != NR_OF_KEY_IDS, string("register_key with invalid name ")+ name);
	valk[nr] = key(name, keysym, ctrl, alt, shift);
}



void cfg::set(const string& name, bool value)
{
	map<string, bool>::iterator it = valb.find(name);
	if (it != valb.end())
		it->second = value;
	else
		sys().myassert(false, string("cfg: set(), name not registered: ") + name);
}



void cfg::set(const string& name, int value)
{
	map<string, int>::iterator it = vali.find(name);
	if (it != vali.end())
		it->second = value;
	else
		sys().myassert(false, string("cfg: set(), name not registered: ") + name);
}



void cfg::set(const string& name, float value)
{
	map<string, float>::iterator it = valf.find(name);
	if (it != valf.end())
		it->second = value;
	else
		sys().myassert(false, string("cfg: set(), name not registered: ") + name);
}



void cfg::set(const string& name, const string& value)
{
	map<string, string>::iterator it = vals.find(name);
	if (it != vals.end())
		it->second = value;
	else
		sys().myassert(false, string("cfg: set(), name not registered: ") + name);
}


	
void cfg::set_key(unsigned nr, SDLKey keysym, bool ctrl, bool alt, bool shift)
{
	map<unsigned, key>::iterator it = valk.find(nr);
	if (it != valk.end())
		it->second = key(it->second.action, keysym, ctrl, alt, shift);
	else
		sys().myassert(false, string("cfg: set_key(), key number not registered: "));
}



bool cfg::getb(const string& name) const
{
	map<string, bool>::const_iterator it = valb.find(name);
	if (it != valb.end())
		return it->second;
	else
		sys().myassert(false, string("cfg: get(), name not registered: ") + name);
	return 0;
}



int cfg::geti(const string& name) const
{
	map<string, int>::const_iterator it = vali.find(name);
	if (it != vali.end())
		return it->second;
	else
		sys().myassert(false, string("cfg: get(), name not registered: ") + name);
	return 0;
}



float cfg::getf(const string& name) const
{
	map<string, float>::const_iterator it = valf.find(name);
	if (it != valf.end())
		return it->second;
	else
		sys().myassert(false, string("cfg: get(), name not registered: ") + name);
	return 0;
}



string cfg::gets(const string& name) const
{
	map<string, string>::const_iterator it = vals.find(name);
	if (it != vals.end())
		return it->second;
	else
		sys().myassert(false, string("cfg: get(), name not registered: ") + name);
	return 0;
}



cfg::key cfg::getkey(unsigned nr) const
{
	map<unsigned, key>::const_iterator it = valk.find(nr);
	if (it != valk.end())
		return it->second;
	else
		sys().myassert(false, string("cfg: getkey(), key number not registered: "));
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
