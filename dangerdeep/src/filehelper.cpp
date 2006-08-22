/*
Danger from the Deep - Open source submarine simulation
Copyright (C) 2003-2006  Thorsten Jordan, Luis Barrancos and others.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

// file helper functions
// (C)+(W) Thorsten Jordan. SEE LICENSE

#include "filehelper.h"
#include <vector>
using namespace std;

#ifdef WIN32
directory open_dir(const string& filename)
{
	directory d;
	d.temporary_used = true;
	d.dir = FindFirstFile((filename + "*.*").c_str(), &d.Win32_FileFind_Temporary);
	return d;
}
string read_dir(directory& d)
{
	if (d.temporary_used) {
		d.temporary_used = false;
		return d.Win32_FileFind_Temporary.cFileName;
	} else {
		BOOL b = FindNextFile(d.dir, &d.Win32_FileFind_Temporary);
		if (b == TRUE)
			return string(d.Win32_FileFind_Temporary.cFileName);
		else
			return "";
	}
}
void close_dir(directory d)
{
	FindClose(d.dir);
}
bool make_dir(const string& dirname)
{
	return (CreateDirectory(dirname.c_str(), NULL) == TRUE);
}
string get_current_directory(void)
{
	unsigned sz = 256;
	vector<char> s(sz);
	DWORD dw = 0;
	while (dw == 0) {
		dw = GetCurrentDirectory(sz, &s[0]);
		if (dw == 0) {
			sz += sz;
			s.resize(sz);
		}
	}
	return string(&s[0]) + PATHSEPARATOR;
}
bool is_directory(const string& filename)
{
	int err = GetFileAttributes(filename.c_str());
	if (err == -1) return false;
	return ((err & FILE_ATTRIBUTE_DIRECTORY) != 0);
}

#else	/* Win32 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
directory open_dir(const string& filename)
{
	directory d;
	d.dir = opendir(filename.c_str());
	if (!d.dir) {
		throw error("Error opening folder; '" + filename + "'" );
	}
	return d;
}

string read_dir(directory& d)
{
	struct dirent* dir_entry = readdir(d.dir);
	if (dir_entry) {
		return string(dir_entry->d_name);
	}
	return "";
}
void close_dir(directory d)
{
	closedir(d.dir);
}
bool make_dir(const string& dirname)
{
	int err = mkdir(dirname.c_str(), 0755);
	return (err != -1);
}
string get_current_directory(void)
{
	unsigned sz = 256;
	vector<char> s(sz);
	char* c = 0;
	while (c == 0) {
		c = getcwd(&s[0], sz-1);
		if (!c) {
			sz += sz;
			s.resize(sz);
		}
	}
	return string(&s[0]) + PATHSEPARATOR;
}
bool is_directory(const string& filename)
{
	struct stat fileinfo;
	int err = stat(filename.c_str(), &fileinfo);
	if (err != 0) return false;
	if (S_ISDIR(fileinfo.st_mode)) return true;
	return false;
}

#endif /* Win32 */
