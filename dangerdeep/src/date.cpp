// date
// subsim (C)+(W) Markus Petermann. SEE LICENSE

#include <iomanip>
#include "date.h"
#include "texts.h"
#include "binstream.h"

void date::copy ( const date& d )
{
	memcpy ( &date_values, &d.date_values, sizeof date_values );
}

date::date ( unsigned year_, unsigned month_, unsigned day_, unsigned hour_,
	unsigned minute_, unsigned second_ )
{
	memset ( &date_values, 0, sizeof date_values );
	date_values[year]   = year_;
	date_values[month]  = month_;
	date_values[day]    = day_;
	date_values[hour]   = hour_;
	date_values[minute] = minute_;
	date_values[second] = second_;
}

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
	if ( texts::language == texts::german )
	{
		os << d.date_values[d.day]    << "." << d.date_values[d.month] << ".";
		os << d.date_values[d.year]   << " ";
		os << setw ( 2 ) << setfill ( '0' ) << d.date_values[d.hour]  << ":";
		os << setw ( 2 ) << setfill ( '0' ) << d.date_values[d.minute] << ":";
		os << setw ( 2 ) << setfill ( '0' ) << d.date_values[d.second];
	}
	else
	{
		os << d.date_values[d.year]   << "-" << d.date_values[d.month] << "-";
		os << d.date_values[d.day]    << " ";
		os << setw ( 2 ) << setfill ( '0' ) << d.date_values[d.hour]  << ":";
		os << setw ( 2 ) << setfill ( '0' ) << d.date_values[d.minute] << ":";
		os << setw ( 2 ) << setfill ( '0' ) << d.date_values[d.second];
	}

	return os;
}

void date::load(istream& in)
{
	for (unsigned i = 0; i < last_date_type; ++i)
		date_values[i] = read_u8(in);
}

void date::save(ostream& out) const
{
	for (unsigned i = 0; i < last_date_type; ++i)
		write_u8(out, date_values[i]);
}
