// this class holds the game's configuration
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef CFG_H
#define CFG_H

#include <map>
#include <string>
using namespace std;

class cfg
{
private:
	cfg(const cfg& );
	cfg& operator= (const cfg& );

	map<string, bool> valb;
	map<string, int> vali;
	map<string, float> valf;
	map<string, string> vals;
	
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

	void set(const string& name, bool value);
	void set(const string& name, int value);
	void set(const string& name, float value);
	void set(const string& name, const string& value);
	
	bool getb(const string& name) const;
	int geti(const string& name) const;
	float getf(const string& name) const;
	string gets(const string& name) const;
	
	void parse_value(const string& s);	// give elements of command line array to it!
};

#endif
