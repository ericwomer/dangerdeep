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

// Danger from the Deep, helper functions for stack trace or SIGSEGV handling
// (C)+(W) by Thorsten Jordan. See LICENSE

#ifndef FAULTHANDLER_H
#define FAULTHANDLER_H

/*
Win32 and MacOsX do not suppport backtracking
*/

#if defined (WIN32) || (defined (__APPLE__) && defined (__MACH__))

#include <stdio.h>

inline void print_stack_trace()
{
	//printf("Stack backtracing not supported on Win32 and MacOSX systems.\n");
}

void install_segfault_handler()
{
	//printf("SIGSEGV catching not supported on Win32 and MacOSX systems.\n");
}

#else	//non-WIN32-MacOSX

#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <cxxabi.h>      // Needed for __cxa_demangle
#include <signal.h>
#include <string>
#include <sstream>

// Note: use --export-dynamic as linker option or you won't get function names here.

inline void print_stack_trace()
{
	void *array[16];
	int size = backtrace(array, 16);
	if (size < 0) {
		fprintf(stderr, "Backtrace could not be created!\n");
		return;
	}
	char** strings = backtrace_symbols (array, size);
	if (!strings) {
		fprintf(stderr, "Could not get Backtrace symbols!\n");
		return;
	}

	fprintf(stderr, "Stack trace: (%u frames)\n", size);
	std::string addrs;
	std::list<std::string> lines;
	for (int i = 0; i < size; ++i) {
		std::string addr;
		std::string s(strings[i]);
		std::string::size_type p1 = s.rfind('[');
		std::string::size_type p2 = s.rfind(']');
		if ((p1 != std::string::npos) && (p2 != std::string::npos)) {
			addr = s.substr(p1 + 1, p2 - p1 - 1);
			addrs += addr + " ";
		}
		p1 = s.rfind("_Z");
		p2 = s.rfind('+');
		if (p2 == std::string::npos)
			p2 = s.rfind(')');
		std::string func;
		if (p1 != std::string::npos) {
			func = s.substr(p1, p2 - p1);
			int status = 0;
			char* c = abi::__cxa_demangle(func.c_str(), 0, 0, &status);
			if (c)
				func = c;
			else
				func = "???";
		} else {
			p1 = s.rfind('(');
			if (p1 != std::string::npos) {
				func = s.substr(p1 + 1, p2 - p1 - 1);
			} else {
				p2 = s.rfind('[');
				func = s.substr(0, p2 - 1);
			}
		}
		lines.push_back(addr + " in " + func);
		if (func == "main")
			break;
	}
	free (strings);

	// determine lines from addresses
	std::ostringstream oss;
	oss << "addr2line -e /proc/" << getpid() << "/exe -s " << addrs;
	FILE* f = popen(oss.str().c_str(), "r");
	if (f) {
		for (std::list<std::string>::iterator it = lines.begin(); it != lines.end(); ++it) {
			char tmp[128];
			fgets(tmp, 127, f);
			fprintf(stderr, "%s at %s", it->c_str(), tmp);
		}
		pclose(f);
	} else {
		for (std::list<std::string>::iterator it = lines.begin(); it != lines.end(); ++it) {
			fprintf(stderr, "%s\n", it->c_str());
		}
	}
}

void sigsegv_handler(int )
{
	fprintf(stderr, "SIGSEGV caught!\n");
	print_stack_trace();
	fprintf(stderr, "Aborting program.\n");
	abort();
}



void install_segfault_handler()
{
	signal(SIGSEGV, sigsegv_handler);
}


#endif	// WIN32 || MacOSX

#endif	// FAULTHANDLER_H
