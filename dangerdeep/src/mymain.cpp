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

// Danger from the Deep (C)+(W) Thorsten Jordan. SEE LICENSE
//
// system dependent main
//
// WIN32: use WinMain, divide cmd line to list of strings
// UNIX: C-like main, make strings from char** array
// MacOSX: ? fixed with objective C code ?
//

#include "faulthandler.h"
#include <list>
#include <string>
#include <exception>
#include <iostream>
using namespace std;

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

int mymain(list<string>& args);

int call_mymain(list<string>& args)
{
	try {
		return mymain(args);
	}
	catch (std::exception& e) {
		cerr << "Caught exception: " << e.what() << "\n";
		print_stack_trace();
		return -1;
	}
	catch (...) {
		cerr << "Caught unknown exception!\n";
		print_stack_trace();
		return -2;
	}
}

#ifdef WIN32

int WinMain(HINSTANCE, HINSTANCE, LPSTR cmdline, int)
{
	string mycmdline(cmdline);
	list<string> args;
	// parse mycmdline
	while (mycmdline.length() > 0) {
		string::size_type st = mycmdline.find(" ");
		args.push_back(mycmdline.substr(0, st));
		if (st == string::npos) break;
		mycmdline = mycmdline.substr(st+1);
	}
	return call_mymain(args);
}

#else	// UNIX

int main(int argc, char** argv)
{
	list<string> args;
	//parse argc, argv, do not store program name
	while (argc > 1) {
		args.push_front(string(argv[--argc]));
	}
	return call_mymain(args);
}

#endif
