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



void cfg::load(const string& filename)
{
	TiXmlDocument doc(filename);
	doc.LoadFile();
	// just one node dftd-cfg, element childs of <name value="value's value"/>
	//parse values!, fixme
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



void cfg::parse_value(const char* c)
{
	// give elements of command line array to it! fixme
	// format of c must be --name=value or --name or --noname for bool values.
	// search for "=" in the string as first
}
