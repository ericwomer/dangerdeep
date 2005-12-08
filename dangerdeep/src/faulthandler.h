// Danger from the Deep, helper functions for stack trace or SIGSEGV handling
// (C)+(W) by Thorsten Jordan. See LICENSE

#ifndef FAULTHANDLER_H
#define FAULTHANDLER_H

#ifndef WIN32

#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>

#include <signal.h>

void print_stack_trace()
{
	void *array[10];
	size_t size = backtrace(array, 10);
	char** strings = backtrace_symbols (array, size);

	printf("Stack trace: (%u frames)\n", size);
	for (unsigned i = 0; i < size; ++i) {
		printf("[%2d] %s\n", i, strings[i]);
	}

	free (strings);
}



void sigsegv_handler(int )
{
	printf("SIGSEGV caught!\n");
	print_stack_trace();
	printf("Aborting program.\n");
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
