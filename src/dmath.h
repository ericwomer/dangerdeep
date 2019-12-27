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

// defines to help with c99 math functions in different targets/compilers
// (C)+(W) by Thorsten Jordan, Matt Lawrence. See LICENSE

#ifndef DMATH_H
#define DMATH_H

#if (defined(__APPLE__) && defined(__GNUC__)) || defined(__MACOSX__)
	#include <complex.h>
	#ifndef isfinite
		#define isfinite(x) finite(x)
	#endif
#elif defined(WIN32)
	#include <float.h>
	#ifndef isfinite
		#define isfinite(x) _finite(x)
	#endif
#elif defined( USINGICC )
	#define isfinite(x) (fpclassify(x) != FP_NAN && fpclassify(x) != FP_INFINITE)
#elif defined( USINGSUN )
	#define isfinite(x) finite(x)
#else
	using std::isfinite;
#endif

#endif // DMATH_H
