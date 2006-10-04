/* OglExt.h                                               Copyright (C) 2006 Thomas Jansen (jansen@caesar.de) */
/*                                                                  (C) 2006 research center caesar           */
/*                                                                                                            */
/* This file is part of OglExt, a free OpenGL extension library.                                              */
/*                                                                                                            */
/* This program is free software; you can redistribute it and/or modify it under the terms of the GNU  Lesser */
/* General Public License as published by the Free Software Foundation; either version 2.1 of the License, or */
/* (at your option) any later version.                                                                        */
/*                                                                                                            */
/* This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the */
/* implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR  PURPOSE.   See  the  GNU  Lesser  General */
/* Public License for more details.                                                                           */
/*                                                                                                            */
/* You should have received a copy of the GNU Lesser General Public License along with this library; if  not, */
/* write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA       */

#ifndef	_OGLEXT_H_
#define	_OGLEXT_H_

#ifdef __cplusplus
   extern "C" {
#endif


/* ---[ INCLUDE ALL API FUNCTIONS ]-------------------------------------------------------------------------- */

#define	GL_GLEXT_PROTOTYPES
#include "glext.h"


/* ---[ ADDITIONAL MACRO FOR VERSION COMPARISON ]------------------------------------------------------------ */

#define	GLEX_VERSION(MAJOR, MINOR, RELEASE)		((GLuint) ((MAJOR << 24) | (MINOR << 16) | (RELEASE << 0)))


/* ---[ ADDITIONAL API FUNCTIONS ]--------------------------------------------------------------------------- */

GLAPI			GLboolean			glexExtensionsSupported(char const * szExtensions);	/*  Extensions supported? */
GLAPI			GLuint				glexGetVersion();													/*     Return GL version. */

#ifdef __cplusplus
   }
#endif

#endif	/* _OGLEXT_H_ */
