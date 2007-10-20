#include <X11/Xlib.h>
#include <GL/glx.h>
#include <dlfcn.h>

#include <iostream>
#include <set>

#include "tests.h"


#ifdef WIN32

int WinMain(HINSTANCE, HINSTANCE, LPSTR cmdline, int)
{
}

#else

int main(int argc, char** argv)
{
	tests *handle = new tests();
	return handle->main();
}

#endif
