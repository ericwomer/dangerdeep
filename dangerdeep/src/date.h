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

// date
// subsim (C)+(W) Markus Petermann. SEE LICENSE

#include <iostream>

#include "xml.h"

#ifndef DATE_H
#define DATE_H

///\brief Date representation, either as linear time or as year, month, day etc.
class date
{
public:
	enum month_type { January = 1, February, March, April, May, June,
		July, August, September, October, November, December };
	enum date_type { year, month, day, hour, minute, second, last_date_type };

private:
	unsigned date_values[last_date_type];
	unsigned linear_time;	// in seconds from 1.1.1939

	void set_from_linear();
	void set_linear();

public:
	static unsigned length_of_year(unsigned year);
	static unsigned length_of_month(unsigned year, unsigned month);

	/**
		Constructor.
		@param year year value.
		@param month month value. Must be a value of type month_type.
		@param day day value.
		no default values for year,month,day because it's not senseful at it makes
		calling this c'tor ambiguous
	*/
	date ( unsigned year, unsigned month, unsigned day,
		unsigned hour = 0, unsigned minute = 0, unsigned second = 0 );

	// construct from linear time
	date (unsigned lt = 0);

	// construct from string of form yyyy/mm/dd
	date (const std::string& datestr);

	// save as date string yyyy/mm/dd, does not save time of day
	std::string to_str() const;

	unsigned get_value ( date_type dt ) const { return date_values[dt]; }
	void set_value ( date_type dt, unsigned val ) { date_values[dt] = val; }
	
	unsigned get_time() const { return linear_time; }

	bool operator< ( const date& d ) const;
	bool operator<= ( const date& d ) const;
	bool operator== ( const date& d ) const;
	bool operator>= ( const date& d ) const;
	bool operator> ( const date& d ) const;

	void load(const xml_elem& parent);
	void save(xml_elem& parent) const;
};

#endif /* DATE_H */
