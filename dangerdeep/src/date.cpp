// date
// subsim (C)+(W) Markus Petermann. SEE LICENSE

#include "date.h"

void date::copy ( const date& d )
{
	year = d.year;
	month = d.month;
	day = d.day;
}

date::date ( int year, month_type month, int day ) : year ( year ), month ( month ),
	day ( day )
{}

date::date ( const date& d )
{
	copy ( d );
}

date& date::operator= ( const date& d )
{
	if ( this != &d )
		copy ( d );

	return *this;
}

bool date::operator< ( const date& d ) const
{
	int res = year - d.year;

	if ( !res )
		res = month - d.month;

	if ( !res )
		res = day - d.day;

	return res < 0;
}

bool date::operator<= ( const date& d ) const
{
	return !( d < *this );
}

bool date::operator== ( const date& d ) const
{
	return !( *this < d ) && !( d < *this );
}

bool date::operator>= ( const date& d ) const
{
	return !( *this < d );
}

bool date::operator> ( const date& d ) const
{
	return ( d < *this );
}

ostream& operator<< ( ostream& os, const date& d )
{
	os << d.day << "." << d.month << "." << d.year;

	return os;
}
