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




#if (defined (__APPLE__) && defined (__MACH__)) || defined MINGW32

#include <stdio.h>

__attribute__((always_inline))

inline void print_stack_trace()
{
	printf("Stack backtracing not supported on Win32 and MacOSX systems.\n");
}

void install_segfault_handler()
{
	printf("SIGSEGV catching not supported on Win32 and MacOSX systems.\n");
}

#elif defined WIN32

#include <windows.h>
#include <shlobj.h>
#include "dbghelp.h"

typedef BOOL (WINAPI *MINIDUMPWRITEDUMP)(HANDLE hprocess, DWORD pid, HANDLE hfile, MINIDUMP_TYPE dumptype,
									CONST PMINIDUMP_EXCEPTION_INFORMATION exceptionparam,
									CONST PMINIDUMP_USER_STREAM_INFORMATION userstreamparam,
									CONST PMINIDUMP_CALLBACK_INFORMATION callbackparam);
inline void print_stack_trace()
{
	printf("Stack backtracing not supported on Win32 and MacOSX systems.\n");
}

static  LONG WINAPI DangerdeepCrashDump(struct _EXCEPTION_POINTERS *pexceptioninfo)
{
	std::ofstream f("log.txt");
	log::instance().write(f, log::LOG_SYSINFO);

	MINIDUMPWRITEDUMP m_dump;
	static HANDLE m_hfile;
	static HMODULE m_hdll;
	char path[MAX_PATH + 1];
	char file[MAX_PATH + 1];
	BOOL ok;
	SYSTEMTIME sysTime = {0};
	GetSystemTime(&sysTime);

	SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path);

	_snprintf( file, MAX_PATH, "\\dangerdeep-%04u-%02u-%02u_%02u-%02u-%02u.dmp", sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond );
	std::string foo = path;
	foo = foo + file;
//	foo = foo + "\\dangerdeep.dmp";



	_MINIDUMP_EXCEPTION_INFORMATION exinfo;
	exinfo.ThreadId = ::GetCurrentThreadId();
	exinfo.ExceptionPointers = pexceptioninfo;
	exinfo.ClientPointers = NULL;



	m_hfile = CreateFile( foo.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);

	m_hdll = LoadLibrary("DBGHELP.DLL");

        m_dump = (MINIDUMPWRITEDUMP)::GetProcAddress(m_hdll, "MiniDumpWriteDump");

	ok = m_dump(GetCurrentProcess(), GetCurrentProcessId(), m_hfile, MiniDumpNormal, &exinfo, NULL, NULL );

	foo = "Please send the following file to the developers: " + foo;

	if ( ok )
	{
		MessageBox(NULL, foo.c_str(), "Core dumped", MB_OK | MB_TASKMODAL | MB_ICONERROR);
		return EXCEPTION_EXECUTE_HANDLER;
	}

	return EXCEPTION_CONTINUE_SEARCH;
}

void install_segfault_handler()
{
	SetUnhandledExceptionFilter( DangerdeepCrashDump );
}

#else	//non-WIN32-MacOSX

#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <cxxabi.h>      // Needed for __cxa_demangle
#include <signal.h>
#include <string>
#include <sstream>
#include <unistd.h>
#include <list>
#include <sys/types.h>

// Note: use --export-dynamic as linker option or you won't get function names here.

void print_stack_trace();
void sigsegv_handler(int number);
void install_segfault_handler();


#endif	// WIN32 || MacOSX

#endif	// FAULTHANDLER_H
