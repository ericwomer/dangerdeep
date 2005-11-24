// xml access interface
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef XML_H
#define XML_H

#include <string>
#include "error.h"
#include "vector3.h"
#include "quaternion.h"
#include "angle.h"

class TiXmlElement;
class TiXmlDocument;



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
	xml_elem(TiXmlElement* e) : elem(e) {}

	std::string doc_name() const;

	friend class xml_doc;
 public:
	std::string attr(const std::string& name = "value") const;
	int attri(const std::string& name = "value") const;
	unsigned attru(const std::string& name = "value") const;
	double attrf(const std::string& name = "value") const;
	vector3 attrv3() const;
	quaternion attrq() const;
	angle attra() const;
	xml_elem child(const std::string& name) const;
	bool has_child(const std::string& name) const;
	xml_elem add_child(const std::string& name);
	void set_attr(const std::string& val, const std::string& name = "value");
	void set_attr(unsigned u, const std::string& name = "value");
	void set_attr(int i, const std::string& name = "value");
	void set_attr(double f, const std::string& name = "value");
	void set_attr(const vector3& v);
	void set_attr(const quaternion& q);
	void set_attr(angle a);
	std::string get_name() const;
	std::string child_text() const;	// returns value of text child, throws error if there is none

	class iterator {
	private:
		iterator();
	protected:
		const xml_elem& parent;
		TiXmlElement* e;
		bool samename;	// iterate over any children or only over children with same name
		iterator(const xml_elem& parent_, TiXmlElement* elem_ = 0, bool samename_ = true)
			: parent(parent_), e(elem_), samename(samename_) {}

		friend class xml_elem;
	public:
		xml_elem elem() const;
		void next();
		bool end() const { return e == 0; }
	};
	friend class iterator;

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
	TiXmlDocument* doc;
 public:
	xml_doc(std::string fn);
	void load();
	void save();
	xml_elem child(const std::string& name);
	xml_elem add_child(const std::string& name);
	std::string get_filename() const;
};

#endif // XML_H
