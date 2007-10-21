
#include <iostream>
#include <set>

#include "tests.h"

#ifdef WIN32

#include <windows.h>
#include "win32.h"

int WINAPI WinMain (HINSTANCE hThisInstance, HINSTANCE hPrevInstance, LPSTR lpszArgument, int nFunsterStil)
{
	win32 *handle = new win32();

	handle->args( hThisInstance, hPrevInstance, lpszArgument, nFunsterStil );

	return handle->main();
}

#else

#include <dlfcn.h>
#include <X11/Xlib.h>
#include <GL/glx.h>
#include "x11.h"

int main(int argc, char** argv)
{
	x11 *handle = new x11();
	return handle->main();
}

#endif
