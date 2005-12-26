// Danger from the Deep, helper functions for stack trace or SIGSEGV handling
// (C)+(W) by Thorsten Jordan. See LICENSE

#ifndef FAULTHANDLER_H
#define FAULTHANDLER_H

#ifndef WIN32

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
		unsigned p1 = s.rfind('[');
		unsigned p2 = s.rfind(']');
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
#else	// WIN32

void install_segfault_handler()
{
	printf("SIGSEGV catching not supported on Win32 systems.\n");
}
#endif	// WIN32

#endif	// FAULTHANDLER_H
