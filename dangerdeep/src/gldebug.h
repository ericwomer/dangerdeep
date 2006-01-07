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

/* dirty, dirty hack but I like it ;-) */

#ifndef GLe
#include <stdio.h>
static GLenum global_error;

/* These macros are useful for finding errors with a particular OpenGL function 
 * although they will pick up errors that happened beforehand
 */

#if 0
#define glTexEnvf(x1, x2, x3) glTexEnvf(x1, x2, x3); global_error = glGetError(); \
printf("OpenGL error on line %d: glTexEnvf(): %s\n", __LINE__, (char*) gluErrorString(global_error));
	
#define glTexEnvfv(x1, x2, x3) glTexEnvfv(x1, x2, x3); global_error = glGetError(); \
printf("OpenGL error on line %d: glTexEnvfv(): %s\n", __LINE__, (char*) gluErrorString(global_error));
#endif

/* 
 * This macro can be peppered through the source and will report any OpenGL errors
 * It's name is kept deliberately short
 * A good place to put it would be the end of each function that contains OpenGL calls
 */
#define GLe global_error = glGetError(); \
if(global_error) printf("OpenGL error: %s in %s line %d\n", (char*) gluErrorString(global_error), __FILE__, __LINE__);
#else
#endif
