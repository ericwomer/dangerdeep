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
