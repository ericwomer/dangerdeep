// texts
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef TEXTS_H
#define TEXTS_H

#include <vector>
#include <string>

class texts
{
 public:
	enum category { common, languages, nr_of_categories };
 private:
	std::vector<std::vector<std::string> > strings;
	void read_category(category ct);
	std::string language_code;

	static const texts& texts::obj();

	texts(const std::string& langcode = "en");
 public:
	static void set_language(const std::string& language_code);
	static std::string get_language_code(void);
	static std::string get(unsigned no, category ct = common);
	static std::string numeric_from_date(const class date& d);
};

#endif
