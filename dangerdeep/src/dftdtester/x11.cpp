#include <X11/Xlib.h>
#include <GL/glx.h>

#include <dlfcn.h>
#include <iostream>
#include <set>

#include "glue_x11.h"

#include "tests.h"
#include "x11.h"

using namespace std;

int x11::unload_ctx()
{
	glXDestroyContext( disp, ctx );
	XDestroyWindow( disp, win );
	XCloseDisplay( disp );
	return 1;
}

int x11::load_ctx()
{
	int items[] = { GLX_RGBA, GLX_RED_SIZE, 1, GLX_GREEN_SIZE, 1, GLX_BLUE_SIZE, 1, None };

	disp = XOpenDisplay(NULL);

	if ( !disp )
		return 1;

	root = RootWindow( disp, 0);
	if ( !root)
		return 1;

	xinfo = glXChooseVisual( disp, DefaultScreen(disp), items );

	if ( !xinfo )
		return 1;

	XSetWindowAttributes attr;

	attr.background_pixel = 0;
	attr.border_pixel = 0;
	attr.colormap = XCreateColormap(disp, root, xinfo->visual, AllocNone);
	attr.event_mask = StructureNotifyMask | ExposureMask;

	unsigned long mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

	win = XCreateWindow( disp, root, 0, 0, 100, 100, 0, xinfo->depth, InputOutput, xinfo->visual, mask, &attr);

	if ( !win )
		return 1;

	GLXContext ctx = glXCreateContext( disp, xinfo,  NULL, GL_TRUE );

	if ( !ctx )
		return 1;

	glXMakeCurrent( disp, win, ctx);

	return 0;
}

int x11::loadlibs()
{
	tests::loadlibs();

	char *error;
	xlib = dlopen( "libX11.so", RTLD_LAZY );

	if ( NULL == xlib )
	{
		cerr << "Failed to load: libGL.so" << endl;
		return 1;
	}

	dlerror();

	*(void **) (&DFTD_XOpenDisplay) = dlsym( xlib, "XOpenDisplay" );
	*(void **) (&DFTD_XCloseDisplay) = dlsym( xlib, "XCloseDisplay" );
	*(void **) (&DFTD_XCreateWindow) = dlsym( xlib, "XCreateWindow" );
	*(void **) (&DFTD_XCreateColormap) = dlsym( xlib, "XCreateColormap" );
	*(void **) (&DFTD_XDestroyWindow) = dlsym( xlib, "XDestroyWindow" );
	*(void **) (&DFTD_glXChooseVisual) = dlsym( opengl, "glXChooseVisual" );
	*(void **) (&DFTD_glXCreateContext) = dlsym( opengl, "glXCreateContext" );
	*(void **) (&DFTD_glXMakeCurrent) = dlsym( opengl, "glXMakeCurrent" );
	*(void **) (&DFTD_glXDestroyContext) = dlsym( opengl, "glXDestroyContext" );

	if ( ( error = dlerror() ) != NULL )
	{
		cerr << "Failed to load X11 symbols" << endl;
		return 1;
	}

	return 0;
}

int x11::unloadlibs()
{
	tests::unloadlibs();

	dlclose( xlib );
	return 1;
}
