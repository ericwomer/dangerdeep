// file helper functions
// (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef FILEHELPER_H
#define FILEHELPER_H

#include <string>
using namespace std;

// directory reading/writing encapsulated for compatibility

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
struct directory {
	HANDLE dir;
	WIN32_FIND_DATA Win32_FileFind_Temporary;	// needed because findfirst does
	bool temporary_used;				// open and first read together
	bool is_valid(void) const { return dir != 0; }
};
#define PATHSEPARATOR "\\"
#else
#include <dirent.h>
struct directory {
	DIR* dir;
	bool is_valid(void) const { return dir != 0; }
};
#define PATHSEPARATOR "/"
#endif

// file helper interface

directory open_dir(const string& filename);	// returns an invalid directory, if filename doesn't exist
string read_dir(directory d);	// returns empty string if directory is fully read.
void close_dir(directory d);
bool make_dir(const string& dirname);	// returns true on success
string get_current_directory(void);	// return absolute path
bool is_directory(const string& filename);	// test if file is directory

#endif
