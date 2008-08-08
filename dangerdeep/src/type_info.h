/*
 * Danger from the Deep - Open source submarine simulation
 * Copyright (C) 2003-2006  Thorsten Jordan, Luis Barrancos and others.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _TYPE_INFO_H
#define	_TYPE_INFO_H

#include <float.h>
#include <limits.h>

template <class T>
class type_info
{
public:
    static unsigned char max_value(unsigned char) { return CHAR_MAX; }
    static unsigned char min_value(unsigned char) { return CHAR_MIN; }
    static signed char max_value(signed char) { return SCHAR_MAX; }
    static signed char min_value(signed char) { return SCHAR_MIN; }
    
    static unsigned short max_value(unsigned short) { return USHRT_MAX; }
    static unsigned short min_value(unsigned short) { return 0; }    
    static signed short max_value(signed short) { return SHRT_MAX; }
    static signed short min_value(signed short) { return SHRT_MIN; }

    static unsigned int max_value(unsigned int) { return UINT_MAX; }
    static unsigned int min_value(unsigned int) { return 0; }
    static signed int max_value(signed int) { return INT_MAX; }
    static signed int min_value(signed int) { return INT_MIN; }
    
    static unsigned long max_value(unsigned long) { return ULONG_MAX; }
    static unsigned long min_value(unsigned long) { return 0; }
    static signed long max_value(signed long) { return LONG_MAX; }
    static signed long min_value(signed long) { return LONG_MIN; }
    
    static float max_value(float) { return FLT_MAX; }
    static float min_value(float) { return FLT_MIN; }
    
    static double max_value(double) { return DBL_MAX; }
    static double min_value(double) { return DBL_MIN; }
    
    static long double max_value(long double) { return LDBL_MAX; }
    static long double min_value(long double) { return LDBL_MIN; }
    
    static bool is_signed() {
        if (((T)-1) < 0) return true;
        else return false;
    }
};

#endif	/* _TYPE_INFO_H */

