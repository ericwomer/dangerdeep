// a highscore list
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "highscorelist.h"
#include "binstream.h"
#include <fstream>

highscorelist::entry::entry(istream& in)
{
	points = read_u32(in);
	name = read_string(in);
}

void highscorelist::entry::save(ostream& out) const
{
	write_u32(out, points);
	write_string(out, name);
}

bool highscorelist::entry::is_worse_than(unsigned pts) const
{
	return points < pts;
}

highscorelist::highscorelist(unsigned maxentries)
{
	entries.resize(maxentries);
}

highscorelist::highscorelist(const string& filename)
{
	ifstream in(filename.c_str(), ios::in|ios::binary);
	unsigned maxentries = read_u8(in);
	entries.resize(maxentries);
	for (unsigned i = 0; i < maxentries; ++i)
		entries[i] = entry(in);
}

highscorelist::highscorelist(const highscorelist& h)
{
	entries = h.entries;
}

highscorelist& highscorelist::operator= (const highscorelist& h)
{
	entries = h.entries;
	return *this;
}

void highscorelist::save(const string& filename) const
{
	ofstream out(filename.c_str(), ios::out|ios::binary);
	Uint8 n = Uint8(entries.size());
	write_u8(out, n);
	for (unsigned i = 0; i < n; ++i)
		entries[i].save(out);
}

bool highscorelist::is_good_enough(unsigned points) const
{
	for (unsigned i = 0; i < entries.size(); ++i)
		if (entries[i].is_worse_than(points))
			return true;
	return false;
}

void highscorelist::record(unsigned points, const string& name)
{
	for (unsigned i = 0; i < entries.size(); ++i) {
		if (entries[i].is_worse_than(points)) {
			// move rest down one step
			for (unsigned j = entries.size(); j > i+1; --j)
				entries[j-1] = entries[j-2];
			entries[i] = entry(points, name);
			return;
		}
	}
	// no entry
}
