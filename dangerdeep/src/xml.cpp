// xml access interface
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "xml.h"


xml_doc::xml_doc(std::string fn)
	: doc(fn), filename(fn)
{
}



void xml_doc::load()
{
	doc.LoadFile();
}



void xml_doc::save()
{
	doc.SaveFile();
}



xml_elem xml_doc::child(const std::string& name)
{
	TiXmlElement* e = doc.FirstChildElement(name);
	if (!e) throw xml_elem_error(name, filename);
	return xml_elem(e, filename);
}



xml_elem xml_doc::add_child(const std::string& name)
{
	TiXmlElement* e = new TiXmlElement(name);
	doc.LinkEndChild(e);
	return xml_elem(e, filename);
}



xml_elem xml_elem::child(const std::string& name)
{
	TiXmlElement* e = elem->FirstChildElement(name);
	if (!e) throw xml_elem_error(name, xmlfilename);
	return xml_elem(e, xmlfilename);
}



xml_elem xml_elem::add_child(const std::string& name)
{
	TiXmlElement* e = new TiXmlElement(name);
	elem->LinkEndChild(e);
	return xml_elem(e, xmlfilename);
}



std::string xml_elem::attr(const std::string& name) const
{
	const char* tmp = elem->Attribute(name);
	if (tmp) return std::string(tmp);
	return std::string();
}



int xml_elem::attri(const std::string& name) const
{
	const char* tmp = elem->Attribute(name);
	if (tmp) return atoi(tmp);
	return 0;
}



unsigned xml_elem::attru(const std::string& name) const
{
	return unsigned(attri(name));
}



float xml_elem::attrf(const std::string& name) const
{
	const char* tmp = elem->Attribute(name);
	if (tmp) return atof(tmp);
	return 0.0f;
}



void xml_elem::set_attr(const std::string& name, const std::string& val)
{
	elem->SetAttribute(name, val);
}



void xml_elem::set_attr(const std::string& name, unsigned u)
{
	set_attr(name, int(u));
}



void xml_elem::set_attr(const std::string& name, int i)
{
	elem->SetAttribute(name, i);
}



void xml_elem::set_attr(const std::string& name, float f)
{
	char tmp[32];
	sprintf(tmp, "%f", f);
	set_attr(name, tmp);
}



xml_elem::iterator xml_elem::iterate(const std::string& childname) const
{
	return iterator(*this, elem->FirstChildElement(childname), true);
}



xml_elem::iterator xml_elem::iterate() const
{
	return iterator(*this, elem->FirstChildElement(), false);
}



xml_elem xml_elem::iterator::elem() const
{
	if (!e) throw xml_error("elem() on empty iterator", parent.xmlfilename);
	return xml_elem(e, parent.xmlfilename);
}



void xml_elem::iterator::next()
{
	if (!e) throw xml_error("next() on empty iterator", parent.xmlfilename);
	if (samename)
		e = e->NextSiblingElement(e->Value());
	else
		e = e->NextSiblingElement();
}
