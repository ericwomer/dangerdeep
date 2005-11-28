// date
// subsim (C)+(W) Markus Petermann. SEE LICENSE

#include <iostream>

#include "xml.h"

#ifndef DATE_H
#define DATE_H

class date
{
public:
	enum month_type { January = 1, February, March, April, May, June,
		July, August, September, October, November, December };
	enum date_type { year, month, day, hour, minute, second, last_date_type };

private:
	unsigned date_values[last_date_type];
	unsigned linear_time;	// in seconds from 1.1.1939

	unsigned length_of_year(unsigned year) const;
	unsigned length_of_month(unsigned year, unsigned month) const;

	void set_from_linear();
	void set_linear();

public:
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

	unsigned get_value ( date_type dt ) const { return date_values[dt]; }
	void set_value ( date_type dt, unsigned val ) { date_values[dt] = val; }
	
	unsigned get_time(void) const { return linear_time; }

	bool operator< ( const date& d ) const;
	bool operator<= ( const date& d ) const;
	bool operator== ( const date& d ) const;
	bool operator>= ( const date& d ) const;
	bool operator> ( const date& d ) const;

	friend std::ostream& operator<< ( std::ostream& os, const date& d );
	
	void load(const xml_elem& parent);
	void save(xml_elem& parent) const;
};

std::ostream& operator<< ( std::ostream& os, const date& d );

#endif /* DATE_H */
