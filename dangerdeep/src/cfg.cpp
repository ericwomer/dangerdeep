// this class holds the game's configuration
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "cfg.h"
#include "tinyxml/tinyxml.h"
#include "system.h"
#include <sstream>
using namespace std;


cfg* cfg::myinst = 0;


	
cfg::cfg()
{
}



cfg& cfg::instance(void)
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
	TiXmlDocument doc(filename);
	doc.LoadFile();
	TiXmlElement* root = doc.FirstChildElement("dftd-cfg");
	system::sys().myassert(root != 0, string("cfg: load(), no root element found in ") + filename);
	TiXmlElement* eattr = root->FirstChildElement();
	for ( ; eattr != 0; eattr = eattr->NextSiblingElement()) {
		const char* attrstr = eattr->Attribute("value");
		system::sys().myassert(attrstr != 0, string("cfg: load(), no value for node ") + eattr->Value());
		bool found = set_str(eattr->Value(), attrstr);
		system::sys().myassert(found, string("cfg: load(), name not registered : ") + eattr->Value());
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



void cfg::register_option(const string& name, int keysym, bool ctrl, bool alt, bool shift)
{
	int i = keysym;
	if (ctrl)  i |= 0x40000000;
	if (alt)   i |= 0x20000000;
	if (shift) i |= 0x10000000;
	vali[name] = i;
}



void cfg::set(const string& name, bool value)
{
	map<string, bool>::iterator it = valb.find(name);
	if (it != valb.end())
		it->second = value;
	else
		system::sys().myassert(false, string("cfg: set(), name not registered: ") + name);
}



void cfg::set(const string& name, int value)
{
	map<string, int>::iterator it = vali.find(name);
	if (it != vali.end())
		it->second = value;
	else
		system::sys().myassert(false, string("cfg: set(), name not registered: ") + name);
}



void cfg::set(const string& name, float value)
{
	map<string, float>::iterator it = valf.find(name);
	if (it != valf.end())
		it->second = value;
	else
		system::sys().myassert(false, string("cfg: set(), name not registered: ") + name);
}



void cfg::set(const string& name, const string& value)
{
	map<string, string>::iterator it = vals.find(name);
	if (it != vals.end())
		it->second = value;
	else
		system::sys().myassert(false, string("cfg: set(), name not registered: ") + name);
}


	
void cfg::set(const string& name, int keysym, bool ctrl, bool alt, bool shift)
{
	int i = keysym;
	if (ctrl)  i |= 0x40000000;
	if (alt)   i |= 0x20000000;
	if (shift) i |= 0x10000000;
	set(name, i);
}



bool cfg::getb(const string& name) const
{
	map<string, bool>::const_iterator it = valb.find(name);
	if (it != valb.end())
		return it->second;
	else
		system::sys().myassert(false, string("cfg: get(), name not registered: ") + name);
	return 0;
}



int cfg::geti(const string& name) const
{
	map<string, int>::const_iterator it = vali.find(name);
	if (it != vali.end())
		return it->second;
	else
		system::sys().myassert(false, string("cfg: get(), name not registered: ") + name);
	return 0;
}



float cfg::getf(const string& name) const
{
	map<string, float>::const_iterator it = valf.find(name);
	if (it != valf.end())
		return it->second;
	else
		system::sys().myassert(false, string("cfg: get(), name not registered: ") + name);
	return 0;
}



string cfg::gets(const string& name) const
{
	map<string, string>::const_iterator it = vals.find(name);
	if (it != vals.end())
		return it->second;
	else
		system::sys().myassert(false, string("cfg: get(), name not registered: ") + name);
	return 0;
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
