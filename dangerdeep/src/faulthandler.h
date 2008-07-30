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

#if (defined (__APPLE__) && defined (__MACH__))

#include <stdio.h>

inline void print_stack_trace()
{
	//printf("Stack backtracing not supported on Win32 and MacOSX systems.\n");
}

void install_segfault_handler()
{
	//printf("SIGSEGV catching not supported on Win32 and MacOSX systems.\n");
}

#elif defined WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dbghelp.h>
#include <tlhelp32.h>

#define MAX_STRING_SIZE 4096
#define MAX_TRACE_LEN 24

#define DBG_DEFAULT "dbghelp.dll"
#define DBG_PATH_32 "\\Debugging Tools for Windows\\dbghelp.dll"
#define DBG_PATH_64 "\\Debugging Tools for Windows 64-Bit\\dbghelp.dll"

// dlopen stuff
HMODULE dbg_lib = NULL;
typedef BOOL (__stdcall *d_StackWalk64)( DWORD MachineType, HANDLE hProcess, HANDLE hThread, LPSTACKFRAME64 StackFrame, PVOID ContextRecord, PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemoryRoutine, PFUNCTION_TABLE_ACCESS_ROUTINE64 FunctionTableAccessRoutine, PGET_MODULE_BASE_ROUTINE64 GetModuleBaseRoutine, PTRANSLATE_ADDRESS_ROUTINE64 TranslateAddress );
typedef BOOL (__stdcall *d_SymGetSymFromAddr64)( IN HANDLE hProcess, IN DWORD64 dwAddr, OUT PDWORD64 pdwDisplacement, OUT PIMAGEHLP_SYMBOL64 Symbol );
typedef BOOL (__stdcall *d_SymGetLineFromAddr64)( IN HANDLE hProcess, IN DWORD64 dwAddr, OUT PDWORD64 pdwDisplacement, OUT PIMAGEHLP_LINE64 Line );
typedef BOOL (__stdcall *d_SymInitialize)( IN HANDLE hProcess, IN PSTR UserSearchPath, IN BOOL fInvadeProcess );
typedef DWORD (__stdcall *d_SymGetOptions)( VOID );
typedef DWORD (__stdcall *d_SymSetOptions)( IN DWORD SymOptions );

typedef DWORD64 (__stdcall *d_SymLoadModule64)( IN HANDLE hProcess, IN HANDLE hFile, IN PSTR ImageName, IN PSTR ModuleName, IN DWORD64 BaseOfDll, IN DWORD SizeOfDll );
typedef PVOID (__stdcall *d_SymFunctionTableAccess64)( HANDLE hProcess, DWORD64 AddrBase );
typedef DWORD64 (__stdcall *d_SymGetModuleBase64)( IN HANDLE hProcess, IN DWORD64 dwAddr );
d_StackWalk64 p_StackWalk64 = NULL;
d_SymGetSymFromAddr64 p_SymGetSymFromAddr64 = NULL;
d_SymInitialize p_SymInitialize = NULL;
d_SymGetOptions p_SymGetOptions = NULL;
d_SymSetOptions p_SymSetOptions = NULL;
d_SymLoadModule64 p_SymLoadModule64 = NULL;
d_SymFunctionTableAccess64 p_SymFunctionTableAccess64 = NULL;
d_SymGetModuleBase64 p_SymGetModuleBase64 = NULL;
d_SymGetLineFromAddr64 p_SymGetLineFromAddr64 = NULL;

HANDLE thread = NULL;
HANDLE process = NULL;
char *sym_paths = NULL;


bool loadlibs()
{
	TCHAR path[MAX_STRING_SIZE];
	TCHAR extra[MAX_STRING_SIZE];

	// look for wintel debugging lib
	if (GetEnvironmentVariable( "ProgramFiles", path, 4096) > 0)
	{
		memcpy( extra, path, MAX_STRING_SIZE );
		strcat_s( extra, DBG_PATH_32 );
		if ( INVALID_FILE_ATTRIBUTES != GetFileAttributes( extra ) )
		{
			// found 32bit extra libs installed...
			dbg_lib = LoadLibrary( extra );
		} else {
			// try 64bits?
			memcpy( extra, path, MAX_STRING_SIZE );
			strcat_s( extra, DBG_PATH_64 );
			if ( INVALID_FILE_ATTRIBUTES != GetFileAttributes( extra ) )
			{
				// people use 64bit windows?
				dbg_lib = LoadLibrary( extra );
			} else {
				// use one shipped with windows (maybe^H^H^H^H^H mostly broken...)
				dbg_lib = LoadLibrary( DBG_DEFAULT );
			}
		}
	} else {
		return false; // you lack a programfiles dir... you suck at windows...
	}

	if ( NULL == dbg_lib ) // just fail...
		return false;

	p_StackWalk64 = (d_StackWalk64) GetProcAddress( dbg_lib, "StackWalk64" );
	p_SymGetSymFromAddr64 = (d_SymGetSymFromAddr64) GetProcAddress( dbg_lib, "SymGetSymFromAddr64" );
	p_SymGetLineFromAddr64 = (d_SymGetLineFromAddr64) GetProcAddress( dbg_lib, "SymGetLineFromAddr64" );
	p_SymInitialize = (d_SymInitialize) GetProcAddress( dbg_lib, "SymInitialize" );
	p_SymGetOptions = (d_SymGetOptions) GetProcAddress( dbg_lib, "SymGetOptions" );
	p_SymSetOptions = (d_SymSetOptions) GetProcAddress( dbg_lib, "SymSetOptions" );
	p_SymLoadModule64 = (d_SymLoadModule64) GetProcAddress( dbg_lib, "SymLoadModule64" );
	p_SymFunctionTableAccess64 = (d_SymFunctionTableAccess64) GetProcAddress( dbg_lib, "SymFunctionTableAccess64" );
	p_SymGetModuleBase64 = (d_SymGetModuleBase64) GetProcAddress( dbg_lib, "SymGetModuleBase64" );

	return true;
}

void build_path()
{
	int available_space = MAX_STRING_SIZE;
	char temp_path[MAX_STRING_SIZE];
    sym_paths = (char*) malloc(MAX_STRING_SIZE);

    if (NULL == sym_paths)
	{
		return;
	}

	memset( sym_paths, 0, MAX_STRING_SIZE );
	
	strncat( sym_paths, ".;", MAX_STRING_SIZE );
	available_space -= 3;

	memset( temp_path, 0, MAX_STRING_SIZE );
	if ( GetCurrentDirectoryA( MAX_STRING_SIZE, temp_path ) > 0)
	{
		strncat( sym_paths, temp_path, available_space );
		available_space -= strlen( temp_path ) - 1;
		sym_paths[MAX_STRING_SIZE - available_space] = ';';
		available_space--;
	}

	memset( temp_path, 0, MAX_STRING_SIZE );
	if ( GetModuleFileNameA( NULL, temp_path, MAX_STRING_SIZE) > 0)
	{
		char *ptr = &temp_path[strlen(temp_path)];
		while( *(--ptr) != '\\')
			*ptr = NULL;
		*(ptr--) = NULL; // rm final \
		
		strncat( sym_paths, temp_path, available_space );
		available_space -= strlen( temp_path );
		sym_paths[MAX_STRING_SIZE - available_space] = ';';
		available_space--;
	}

	memset( temp_path, 0, MAX_STRING_SIZE );
	if ( GetEnvironmentVariableA( "SYSTEMROOT", temp_path, MAX_STRING_SIZE) > 0)
	{
		strncat( sym_paths, temp_path, available_space );
		available_space -= strlen( temp_path );
		sym_paths[MAX_STRING_SIZE - available_space] = ';';
		available_space--;

		strcat( temp_path, "\\system32" );
		strncat( sym_paths, temp_path, available_space );
		available_space -= strlen( temp_path );
		sym_paths[MAX_STRING_SIZE - available_space] = ';';
		available_space--;
	}
}

bool loadsyms()
{
	int symopts;

	if ( false == p_SymInitialize( process, sym_paths, false ) )
	{
		return false;
	}

	symopts = p_SymGetOptions();
	symopts |= SYMOPT_LOAD_LINES;
	symopts |= SYMOPT_FAIL_CRITICAL_ERRORS;
	symopts = p_SymSetOptions( symopts );

	// toolhelp32
	HANDLE snap = CreateToolhelp32Snapshot( TH32CS_SNAPMODULE, 0 );
	MODULEENTRY32 module;
	module.dwSize = sizeof( module );
	bool carry_test;

	if ( INVALID_HANDLE_VALUE == snap )
	{
		return false;
	}


	int count = 0;
	for( carry_test = !!Module32First( snap, &module );
		carry_test; 
		carry_test = !!Module32Next( snap, &module ) )
	{
		count++;
		char exepath[MAX_STRING_SIZE]; memset( exepath, 0, MAX_STRING_SIZE );
		char modname[MAX_STRING_SIZE]; memset( modname, 0, MAX_STRING_SIZE );
		strncpy( exepath, module.szExePath, 260 );
		strncpy( modname, module.szModule, 256 );
/*		WideCharToMultiByte( CP_ACP, WC_NO_BEST_FIT_CHARS, module.szExePath, wcsnlen( module.szExePath, 260 ), exepath, MAX_STRING_SIZE, NULL, NULL );
		WideCharToMultiByte( CP_ACP, WC_NO_BEST_FIT_CHARS, module.szModule, wcsnlen( module.szModule, 256 ), modname, MAX_STRING_SIZE, NULL, NULL );*/
		if ( 0 != p_SymLoadModule64( process, NULL, modname, NULL, (DWORD64)module.modBaseAddr, module.modBaseSize ) )
		{
/*			pout( "Loaded: " );
			pout( modname );
			pout( "\n" );*/
		}
	}

	CloseHandle( snap );

	return true;
}

CONTEXT get_context()
{
	CONTEXT ctx;
	memset(&ctx, 0, sizeof( ctx ) );
	ctx.ContextFlags = CONTEXT_FULL;
	RtlCaptureContext( &ctx );
	return ctx;
}

void trace()
{
	STACKFRAME64 stack;
	memset(&stack, 0, sizeof( STACKFRAME64 ));
	CONTEXT ctx = get_context();
	// 32bit only... lazy today... fix up later TODO
	stack.AddrPC.Offset = ctx.Eip;
	stack.AddrFrame.Offset = ctx.Ebp;
	stack.AddrStack.Offset = ctx.Esp;
	stack.AddrPC.Mode = AddrModeFlat;
	stack.AddrFrame.Mode = AddrModeFlat;
	stack.AddrStack.Mode = AddrModeFlat;

	DWORD64 displacement;
	IMAGEHLP_SYMBOL64 *symbol = (IMAGEHLP_SYMBOL64 *) malloc( sizeof(IMAGEHLP_SYMBOL64) + MAX_STRING_SIZE );
	symbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64);
	symbol->MaxNameLength = MAX_STRING_SIZE;
  
	char buffer[MAX_STRING_SIZE];
	char function[MAX_STRING_SIZE];
	char file[MAX_STRING_SIZE];
	int line_num;

	IMAGEHLP_LINE64 line;
	memset(&line, 0, sizeof(IMAGEHLP_LINE64));
	line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

	for( int ix =0; ix < MAX_TRACE_LEN; ix++ )
	{
		displacement = 0;
		memset( &buffer, 0, MAX_STRING_SIZE);
		memset( &function, 0, MAX_STRING_SIZE);
		memset( &file, 0, MAX_STRING_SIZE);
		line_num = 0;

		if ( !p_StackWalk64( IMAGE_FILE_MACHINE_I386, process, thread, &stack, &ctx, NULL, p_SymFunctionTableAccess64, p_SymGetModuleBase64, NULL ) )
			break;

		// check for endless loops
		if( stack.AddrReturn.Offset == stack.AddrPC.Offset )
			return;

		if ( 0 != stack.AddrPC.Offset )
		{
			if ( false != p_SymGetSymFromAddr64( process, stack.AddrPC.Offset, &displacement, symbol ) )
			{
				strncpy( function, symbol->Name, MAX_STRING_SIZE );
			} else {
				strncpy( function, "No Symbol", 10 );
			}

			if ( false != p_SymGetLineFromAddr64( process, stack.AddrPC.Offset, &displacement, &line ) )
			{
				strncpy( file, line.FileName, MAX_STRING_SIZE );
				line_num = line.LineNumber;
			} else {
				strncpy( file, "Unknown", MAX_STRING_SIZE );
			}

			//printf( "%d: %s %s:%d\n", ix, function, file, line_num );
			log_info( "\t" << ix << ": " << function << "\t" << file << ":" << line_num );
		}
	}

	free( symbol );
}


inline void print_stack_trace()
{
	log_info( "Stack Trace:" );

	if ( !loadlibs() )
	{
		log_info( "Failed to load calltrace libraries" );
		return;
	}

	// load info
	thread = GetCurrentThread();
	process = GetCurrentProcess();
	build_path();

	if ( !loadsyms() )
	{
		log_info( "Failed to load debug symbols" );
		return;
	}

	// do the trace
	trace();

	free( sym_paths );
}

long WINAPI exception_handler( struct _EXCEPTION_POINTERS *nfo )
{
	print_stack_trace();
	ExitProcess( 11 );
}

void install_segfault_handler()
{
	SetUnhandledExceptionFilter( exception_handler );
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
