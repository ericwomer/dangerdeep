// date
// subsim (C)+(W) Markus Petermann. SEE LICENSE

#include <iostream>
using namespace std;

#ifndef DATE_H
#define DATE_H

class date
{
public:
	enum month_type { January = 1, February, March, April, May, June,
		July, August, September, October, November, December };
	enum date_type { year, month, day, hour, minute, second, last_date_type };

#ifdef WIN32
public:		// avoid compiler bug.
#else
private:
#endif
	unsigned date_values[last_date_type];

private:
	/**
		Copy the content of object d to the actual object.
		@param d date object which attributes are going to be copied.
	*/
	void copy ( const date& d );

public:
	/**
		Constructor.
		@param year year value.
		@param month month value. Must be a value of type month_type.
		@param day day value.
	*/
	date ( unsigned year = 1939, unsigned month = September, unsigned day = 1,
		unsigned hour = 0, unsigned minute = 0, unsigned second = 0 );
	/**
		Copy constructor.
		@param d date object which attributes are going to be copied.
	*/
	date ( const date& d );

	virtual unsigned get_value ( date_type dt ) const { return date_values[dt]; }
	virtual void set_value ( date_type dt, unsigned val ) { date_values[dt] = val; }

	virtual date& operator= ( const date& d );
	virtual bool operator< ( const date& d ) const;
	virtual bool operator<= ( const date& d ) const;
	virtual bool operator== ( const date& d ) const;
	virtual bool operator>= ( const date& d ) const;
	virtual bool operator> ( const date& d ) const;

	friend ostream& operator<< ( ostream& os, const date& d );
};

ostream& operator<< ( ostream& os, const date& d );

#endif /* DATE_H */
