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

#include <iostream>
#include <sstream>
#include <set>
#include <stdlib.h>
#include <GL/gl.h>

#include "tests.h"

using namespace std;

int tests::main()
{
	if ( loadlibs() )
	{
		cerr << BAD << "Failed to load libraries" << endl;
		return 1;
	}

	if ( load_ctx() )
	{
		cerr << BAD << "Failed to init GL connection" << endl;
		return 1;
	}

	if ( 0 ==  do_gl_tests() )
	{
		cout << endl << BAD << "Not all tests returned successful. Dangerdeep might not run well or at all on your hardware! Problems include:" << endl;
		{
			for( set<string>::const_iterator it = error_log.begin(); it != error_log.end(); it++ )
			{
				cout << it->c_str() << endl;
			}
		}
	} else {
		cout << endl << GOOD << "No problems were found. You should have no problems running Dangerdeep." << endl;
	}

	unload_ctx();

	unloadlibs();

	return 0;
}

void tests::load_gl_info()
{
	const char *c_vendor = (const char *)glGetString( GL_VENDOR );
	const char *c_render = (const char *)glGetString( GL_RENDERER );

	c_version = (const char *)glGetString( GL_VERSION );
	c_extensions = (const char *)glGetString( GL_EXTENSIONS );

	string vendor = c_vendor ? c_vendor : "Unknown";
	string render = c_render ? c_render : "Unknown";
	version = c_version ? c_version : "Unknown";
	extensions = c_extensions ? c_extensions : "Unknown";

	cout << START_ITEM << "Vendor: " << STOP_ITEM << vendor << endl;
	cout << START_ITEM << "Render: " << STOP_ITEM<< render << endl;
	cout << START_ITEM << "Version: " << STOP_ITEM<< version << endl;

	// some modifled dftd code to parse the extensions
	if (c_extensions)
	{
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

				spos = pos+1;
			}
		}
	}
}

int tests::pt_out( std::string message, enum status status )
{
	switch ( status )
	{
		case sGOOD:
			cout << GOOD << message << endl;
			return 1;
		break;
		case sMED:
			cout << MED << message << endl;
			warn_log.insert( message );
		break;
		case sBAD:
			cout << BAD << message << endl;
			error_log.insert( message );
		break;
	}


	return 0;
}

int tests::do_version_check()
{
	if ( c_version )
	{
		unsigned spos = 0;
		unsigned count = 0;
		unsigned last = 0;

		enum status status;

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
			status = sGOOD;
		} else if ( 1 == major && 5 == minor ) {
			status = sBAD; // we require OpenGL 2.x as a minimum.
		} else {
			status = sBAD;
		}

		MPT_OUT( "OpenGL Version: " << major << "." << minor << ".x ", status );
	} else {
		return pt_out( "No version", sBAD );
	}
}


int tests::do_texunit_check()
{
	int texture_units = 0;
	enum status status;

	glGetIntegerv(GL_MAX_TEXTURE_UNITS, &texture_units );

	if ( texture_units > 3 )
	{
		status = sGOOD;
	} else if( texture_units > 2 )
	{
		status = sMED;
	} else {
		status = sBAD;
	}

	MPT_OUT( "Found " << texture_units << " Texture Units ", status );
}

int tests::do_vbo_check()
{
	enum status status;
	if ( extension_supported( "GL_ARB_vertex_buffer_object" ) )
	{
		status = sGOOD;
	} else {
		status = sBAD;
	}

	return pt_out( "Support for vertex buffer objects", status );
}

int tests::do_fb_check()
{
	enum status status;
	if ( extension_supported( "GL_EXT_framebuffer_object" ) )
	{
		status = sGOOD;
	} else {
		status = sBAD;
	}

	return pt_out( "Support for framebuffer objects", status );
}
int tests::do_power2_check()
{
	enum status status;
	if ( extension_supported( "GL_ARB_texture_non_power_of_two" ) )
	{
		status = sGOOD;
	} else {
		status = sMED;
	}
		
	return pt_out( "Support for non power of two textures", status );
}

int tests::do_fshader_check()
{
	enum status status;
	if ( extension_supported( "GL_ARB_fragment_shader" ) )
	{
		status = sGOOD;
	} else {
		status = sBAD;
	}

	return pt_out( "Support for fragment shaders", status );
}

int tests::do_vshader_check()
{
	enum status status;
	if ( extension_supported( "GL_ARB_vertex_shader" ) )
	{
		status = sGOOD;
	} else {
		status = sBAD;
	}

	return pt_out( "Support for vertex shaders", status );
}

int tests::do_shaderobj_check()
{
	enum status status;
	if ( extension_supported( "GL_ARB_shader_objects" ) )
	{
		status = sGOOD;
	} else {
		status = sBAD;
	}

	return pt_out( "Support for shader objects", status );
}

int tests::do_gl_tests()
{
	int retval = 1;

	load_gl_info();

	if ( 0 == do_version_check() )
		retval = 0;

	if ( 0 == do_texunit_check() )
		retval = 0;
	
	if ( 0 == do_vbo_check() )
		retval = 0;

	if ( 0 == do_fb_check() )
		retval = 0;
	
	if ( 0 == do_power2_check() )
		retval = 0;

	if ( 0 == do_fshader_check() )
		retval = 0;

	if ( 0 == do_vshader_check() )
		retval = 0;

	if ( 0 == do_shaderobj_check() )
		retval = 0;

	return retval;
}

// more stolen code
bool tests::extension_supported(const string& s)
{
	set<string>::const_iterator it = supported_extensions.find(s);
	return (it != supported_extensions.end());
}

#if 0

int tests::loadlibs()
{
	char *error;
	opengl = dlopen( "libGL.so", RTLD_LAZY );

	if ( NULL == opengl )
	{
		cerr << "Failed to load: libGL.so" << endl;
		return 1;
	}

	dlerror();

	*(void **) (&DFTD_glGetString) = dlsym( opengl, "glGetString" );
	*(void **) (&DFTD_glGetIntegerv) = dlsym( opengl, "glGetIntegerv" );

	if ( ( error = dlerror() ) != NULL )
	{
		cerr << "Failed to load OpenGL symbols" << endl;
		return 1;
	}
	return 0;
}

int tests::unloadlibs()
{
	dlclose( opengl );
	return 1;
}

#else

int tests::loadlibs() { return 0; }
int tests::unloadlibs() { return 0; }

#endif
