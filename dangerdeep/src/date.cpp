// date
// subsim (C)+(W) Markus Petermann. SEE LICENSE

#include <iomanip>
#include "date.h"
#include "texts.h"
#include "binstream.h"

void date::copy ( const date& d )
{
	memcpy ( &date_values, &d.date_values, sizeof date_values );
	linear_time = d.linear_time;
}

unsigned date::length_of_year(unsigned year) const
{
	if (year % 4 == 0) {
		if (year % 100 == 0) {
			if (year % 400 == 0) {
				return 366;
			} else {
				return 365;
			}
		} else {
			return 366;
		}
	}
	return 365;
}

unsigned date::length_of_month(unsigned year, unsigned month) const
{
	if (month == January || month == March || month == May || month == July || month == August
			|| month == October || month == December)
		return 31;
	if (month == February)
		return (length_of_year(year) == 366) ? 29 : 28;
	return 30;
}

date::date ( unsigned year_, unsigned month_, unsigned day_, unsigned hour_,
	unsigned minute_, unsigned second_ )
{
	date_values[year]   = year_;
	date_values[month]  = month_;
	date_values[day]    = day_;
	date_values[hour]   = hour_;
	date_values[minute] = minute_;
	date_values[second] = second_;
	
	// compute linear time
	// compute number of days from 1.1.1939 first
	unsigned d = 0;
	for (unsigned y = 1939; y < year_; ++y)
		d += length_of_year(y);
	for (unsigned m = January; m < month_; ++m)
		d += length_of_month(year_, m);
	d += day_ - 1;
	linear_time = d * 86400 + hour_ * 3600 + minute_ * 60 + second_;
}

date::date (unsigned lt)
{
	linear_time = lt;
	date_values[second] = lt % 60;
	lt = lt / 60;
	date_values[minute] = lt % 60;
	lt = lt / 60;
	date_values[hour] = lt % 24;
	lt = lt / 24;
	// now lt is the number of days since 1.1.1939
	unsigned y = 1939;
	while (lt >= length_of_year(y)) {
		lt -= length_of_year(y);
		++y;
	}
	date_values[year] = y;
	// now lt is the number of days since begin of year
	unsigned m = January;
	while (lt >= length_of_month(y, m)) {
		lt -= length_of_month(y, m);
		++m;
	}
	date_values[month] = m;
	date_values[day] = 1 + lt;
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
	return unsigned(linear_time/86400) < unsigned(d.linear_time/86400);
}

bool date::operator<= ( const date& d ) const
{
	return unsigned(linear_time/86400) <= unsigned(d.linear_time/86400);
}

bool date::operator== ( const date& d ) const
{
	return unsigned(linear_time/86400) == unsigned(d.linear_time/86400);
}

bool date::operator>= ( const date& d ) const
{
	return unsigned(linear_time/86400) >= unsigned(d.linear_time/86400);
}

bool date::operator> ( const date& d ) const
{
	return unsigned(linear_time/86400) > unsigned(d.linear_time/86400);
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
	date_values[0] = read_u16(in);
	for (unsigned i = 1; i < last_date_type; ++i)
		date_values[i] = read_u8(in);
	linear_time = read_u32(in);
}

void date::save(ostream& out) const
{
	write_u16(out, date_values[0]);
	for (unsigned i = 1; i < last_date_type; ++i)
		write_u8(out, date_values[i]);
	write_u32(out, linear_time);
}
