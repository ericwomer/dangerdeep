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

#ifndef FILEHELPER_H
#define FILEHELPER_H

#include <string>

// directory reading/writing encapsulated for compatibility

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
struct directory {
	HANDLE dir;
	WIN32_FIND_DATA Win32_FileFind_Temporary;	// needed because findfirst does
	bool temporary_used;				// open and first read together
	bool is_valid() const { return dir != 0; }
};
#define PATHSEPARATOR "\\"
#else
#include <dirent.h>
struct directory {
	DIR* dir;
	bool is_valid() const { return dir != 0; }
};
#define PATHSEPARATOR "/"
#endif

// file helper interface

///\brief Open directory and return a handle for it.
///Returns an invalid directory if filename doesn't exist
directory open_dir(const std::string& filename);

///\brief Read next filename from directory.
///Returns an empty string if directory is fully read.
std::string read_dir(directory& d);

///\brief Close directory handle.
void close_dir(directory d);

///\brief Make new directory. Returns true on success.
bool make_dir(const std::string& dirname);

///\brief Returns absolute path of current working directory.
std::string get_current_directory();

///\brief Test if the given filename is a directory.
bool is_directory(const std::string& filename);

///\brief Test if the given filename is a file (can be read by fopen())
bool is_file(const std::string& filename);

#endif
