/*
Danger from the Deep - Open source submarine simulation
Copyright (C) 2007  Matthew Lawrence, Thorsten Jordan, 
Luis Barrancos and others.

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

// OpenGL capabilities tester program


// TODO:	WGL support
// 		Test under OSX
//

#include <X11/Xlib.h>
#include <GL/glx.h>
#include <dlfcn.h>

#include <iostream>
#include <set>

#include "tests.h"
#include "glue.h"


using namespace std;

int tests::main()
{
	if ( loadlibs() )
	{
		cerr << BAD << "Failed to load libraries" << endl;
		return 1;
	}

	if ( loadX() )
	{
		cerr << BAD << "Failed to init GLX" << endl;
		return 1;
	}

	if ( 0 ==  do_gl_tests() )
	{
		cout << endl << BAD << "Not all tests returned successful. Dangerdeep might not run well or at all on your hardware!" << endl;
	} else {
		cout << endl << GOOD << "No problems were found. You should have no problems running Dangerdeep." << endl;
	}

	closeX();

	unloadlibs();

	return 0;
}

int tests::do_gl_tests()
{
	int retval = 1;

	const char *c_vendor = glGetString( GL_VENDOR );
	const char *c_render = glGetString( GL_RENDERER );
	const char *c_version = glGetString( GL_VERSION );
	const char *c_extensions = glGetString( GL_EXTENSIONS );

	string vendor = c_vendor ? c_vendor : "Unknown";
	string render = c_render ? c_render : "Unknown";
	string version = c_version ? c_version : "Unknown";
	string extensions = c_extensions ? c_extensions : "Unknown";

	cout << START_ITEM << "Vendor: " << STOP_ITEM << vendor << endl;
	cout << START_ITEM << "Render: " << STOP_ITEM<< render << endl;
//	cout << START_ITEM << "Version: " << STOP_ITEM<< version << endl;


	// some modifled dftd code
	if (c_extensions)
	{
//		cout << START_ITEM << "Extensions: " << STOP_ITEM << endl;

		unsigned spos = 0;
		while ( spos < extensions.length() )
		{
			string::size_type pos = extensions.find(" ", spos);
			if (pos == string::npos)
			{
				supported_extensions.insert(extensions.substr(spos));
				spos = extensions.length();
			} else {
				supported_extensions.insert(extensions.substr(spos, pos-spos));

//				cout << "\t" << extensions.substr(spos, pos-spos) << endl;
				spos = pos+1;
			}
		}
	}

	if ( c_version )
	{
		unsigned spos = 0;
		unsigned count = 0;
		unsigned last = 0;

		int major = 0;
		int minor = 0;

		while ( spos < version.length() )
		{
			if ( '.' == version.c_str()[spos] )
			{
				string temp = version.substr( last, spos - last );
				last = spos + 1;

				if ( 0 == count )
				{
					major = atoi( temp.c_str() );
				} else if ( 1 == count )
				{
					minor = atoi( temp.c_str() );
					break;
				}
				count++;
			}
			spos++;
		}

		if ( 2 == major )
		{
			cout << GOOD;
		} else if ( 1 == major && 5 == minor ) {
			cout << MED;
			retval = 0;
		} else {
			cout << BAD;
			retval = 0;
		}

		cout << "OpenGL Version: " << major << "." << minor << ".x " << endl;

	} else {
		cout << "No version" << BAD << endl;
		retval = 0;
	}


	int texture_units = 0;
	glGetIntegerv(GL_MAX_TEXTURE_UNITS, &texture_units );

	if ( texture_units > 3 )
	{
		cout << GOOD;
	} else if( texture_units > 2 )
	{
		cout << MED;
		retval = 0;
	} else {
		cout << BAD;
		retval = 0;
	}
	cout << "Found " << texture_units << " Texture Units " << endl;

	if ( extension_supported( "GL_ARB_vertex_buffer_object" ) )
	{
		cout << GOOD;
	} else {
		cout << BAD;
		retval = 0;
	}
	cout << "Support for vertex buffer objects " << endl;

	if ( extension_supported( "GL_EXT_framebuffer_object" ) )
	{
		cout << GOOD;
	} else {
		cout << BAD;
		retval = 0;
	}
	cout << "Support for framebuffer objects " << endl;

	if ( extension_supported( "GL_ARB_texture_non_power_of_two" ) )
	{
		cout << GOOD;
	} else {
		cout << MED;
	}
	cout << "Support for non power of two textures " << endl;

	if ( extension_supported( "GL_ARB_fragment_shader" ) )
	{
		cout << GOOD;
	} else {
		cout << BAD;
		retval = 0;
	}
	cout << "Support for fragment shaders " << endl;

	if ( extension_supported( "GL_ARB_vertex_shader" ) )
	{
		cout << GOOD;
	} else {
		cout << BAD;
		retval = 0;
	}
	cout << "Support for vertex shaders " << endl;

	if ( extension_supported( "GL_ARB_shader_objects" ) )
	{
		cout << GOOD;
	} else {
		cout << BAD;
		retval = 0;
	}
	cout << "Support for shader objects " << endl;

	return retval;
}

// more stolen code
bool tests::extension_supported(const string& s)
{
	set<string>::const_iterator it = supported_extensions.find(s);
	return (it != supported_extensions.end());
}

int tests::closeX()
{
	glXDestroyContext( disp, ctx );
	XDestroyWindow( disp, win );
	XCloseDisplay( disp );
	return 1;
}

int tests::loadX()
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
int tests::loadlibs()
{
	char *error;
	opengl = dlopen( "libGL.so", RTLD_LAZY );
	xlib = dlopen( "libX11.so", RTLD_LAZY );

	if ( NULL == opengl )
	{
		cerr << "Failed to load: libGL.so" << endl;
		return 1;
	}

	if ( NULL == xlib )
	{
		cerr << "Failed to load: libGL.so" << endl;
		return 1;
	}

	dlerror();

	*(void **) (&DFTD_glGetString) = dlsym( opengl, "glGetString" );
	*(void **) (&DFTD_glXChooseVisual) = dlsym( opengl, "glXChooseVisual" );
	*(void **) (&DFTD_glXCreateContext) = dlsym( opengl, "glXCreateContext" );
	*(void **) (&DFTD_glXMakeCurrent) = dlsym( opengl, "glXMakeCurrent" );
	*(void **) (&DFTD_glXDestroyContext) = dlsym( opengl, "glXDestroyContext" );
	*(void **) (&DFTD_glGetIntegerv) = dlsym( opengl, "glGetIntegerv" );

	if ( ( error = dlerror() ) != NULL )
	{
		cerr << "Failed to load OpenGL symbols" << endl;
		return 1;
	}

	*(void **) (&DFTD_XOpenDisplay) = dlsym( xlib, "XOpenDisplay" );
	*(void **) (&DFTD_XCloseDisplay) = dlsym( xlib, "XCloseDisplay" );
	*(void **) (&DFTD_XCreateWindow) = dlsym( xlib, "XCreateWindow" );
	*(void **) (&DFTD_XCreateColormap) = dlsym( xlib, "XCreateColormap" );
	*(void **) (&DFTD_XDestroyWindow) = dlsym( xlib, "XDestroyWindow" );

	if ( ( error = dlerror() ) != NULL )
	{
		cerr << "Failed to load X11 symbols" << endl;
		return 1;
	}

	return 0;
}

int tests::unloadlibs()
{
	dlclose( opengl );
	dlclose( xlib );
	return 1;
}
