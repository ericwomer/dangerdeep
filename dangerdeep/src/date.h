// date
// subsim (C)+(W) Markus Petermann. SEE LICENSE

#include <iostream>
using namespace std;

#ifndef DATE_H
#define DATE_H

class date
{
public:
	enum month_type { January, February, March, April, May, June, July, August, September,
		November, December };

private:
	/// Year value.
	int year;
	/// Month value.
	int month;
	/// Day value.
	int day;

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
	date ( int year = 1939, month_type month = September, int day = 1 );
	/**
		Copy constructor.
		@param d date object which attributes are going to be copied.
	*/
	date ( const date& d );

	/**
		Returns day value.
		@return day value
	*/
	virtual int get_day () const { return day; }
	/**
		Returns month value. Be aware that month 0 is January, 11 is December.
		@return month value
	*/
	virtual int get_month () const { return month; }
	/**
		Returns year value.
		@return year value.
	*/
	virtual int get_year () const { return year; }

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
