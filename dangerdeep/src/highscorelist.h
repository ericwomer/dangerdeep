// a highscore list
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef HIGHSCORELIST_H
#define HIGHSCORELIST_H

#include <iostream>
#include <vector>
#include <string>
using namespace std;

class highscorelist
{
	friend void check_for_highscore(const class game* );

public:
	struct entry {
		unsigned points;
		string name;
		// missing: maybe start & end date, realism factor, rank/merits
		entry() : points(0), name("--------") {}
		entry(unsigned p, const string& n) : points(p), name(n) {}
		entry(istream& in);
		~entry() {}
		entry(const entry& e) : points(e.points), name(e.name) {}
		entry& operator= (const entry& e) { points = e.points; name = e.name; return *this; }
		void save(ostream& out) const;
		bool is_worse_than(unsigned pts) const;	// is entry worse than given value?
	};

protected:
	vector<entry> entries;
	
public:
	highscorelist(unsigned maxentries = 10);
	~highscorelist() {}
	highscorelist(const string& filename);	// read from file
	highscorelist(const highscorelist& h);
	highscorelist& operator= (const highscorelist& h);
	void save(const string& filename) const;
	unsigned get_listpos_for(unsigned points) const; // returns place in list or entries.size() if not in list
	bool is_good_enough(unsigned points) const; // check if score is good enough for an entry
	void record(unsigned points, const string& name); // record entry if it is good enough
	void show(class widget* parent) const;
};

#endif
