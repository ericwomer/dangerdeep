// xml access interface
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef XML_H
#define XML_H

#include <string>
#include "error.h"
#include "tinyxml/tinyxml.h"


class xml_error : public error
{
 public:
	xml_error(const std::string& name, const std::string& fn)
		: error(std::string("xml error: ") + name + std::string(", file: ") + fn) {}
};



class xml_elem_error : public xml_error
{
 public:
	xml_elem_error(const std::string& name, const std::string& fn)
		: xml_error(std::string("failed to get element ") + name, fn) {}
};



class xml_elem
{
 private:
	xml_elem();
 protected:
	TiXmlElement* elem;
	const std::string& xmlfilename;	// used for error reporting
	xml_elem(TiXmlElement* e, const std::string& xfn) : elem(e), xmlfilename(xfn) {}

	friend class xml_doc;
	friend class iterator;
 public:
	std::string attr(const std::string& name = "value") const;
	int attri(const std::string& name = "value") const;
	unsigned attru(const std::string& name = "value") const;
	float attrf(const std::string& name = "value") const;
	xml_elem child(const std::string& name);
	xml_elem add_child(const std::string& name);
	void set_attr(const std::string& name, const std::string& val);
	void set_attr(const std::string& name, unsigned u);
	void set_attr(const std::string& name, int i);
	void set_attr(const std::string& name, float f);

	class iterator {
	private:
		iterator();
	protected:
		const xml_elem& parent;
		TiXmlElement* e;
		bool samename;
		iterator(const xml_elem& parent_, TiXmlElement* elem_ = 0, bool samename_ = true)
			: parent(parent_), e(elem_), samename(samename_) {}

		friend class xml_elem;
	public:
		xml_elem elem() const;
		void next();
		bool end() const { return e == 0; }
	};

	// iterate this way: for (iterator it = e.iterate("name"); !it.end(); it.next()) { ... }
	iterator iterate(const std::string& childname) const;
	// iterate over any child
	iterator iterate() const;
};



class xml_doc
{
 private:
	xml_doc();
	xml_doc(const xml_doc& );
	xml_doc& operator= (const xml_doc& );
 protected:
	TiXmlDocument doc;
	std::string filename;
 public:
	xml_doc(std::string fn);
	void load();
	void save();
	xml_elem child(const std::string& name);
	xml_elem add_child(const std::string& name);
	std::string get_filename() const { return filename; }
};

#endif // XML_H
