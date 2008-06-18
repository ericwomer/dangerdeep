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
// subsim (C)+(W) Markus Petermann and Thorsten Jordan. SEE LICENSE

#include <iomanip>
#include <cstdlib>

#include "date.h"
#include "texts.h"

using std::string;
using std::ostream;
using std::setw;
using std::setfill;

unsigned date::length_of_year(unsigned year)
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

unsigned date::length_of_month(unsigned year, unsigned month)
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
	set_linear();
}

void date::set_from_linear()
{
	unsigned lt = linear_time;
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

void date::set_linear()
{
	linear_time = 0;
	for (unsigned i = 1939; i < date_values[year]; ++i)
		linear_time += length_of_year(i);
	for (unsigned i = 1; i < date_values[month]; ++i)
		linear_time += length_of_month(date_values[year], i);
	linear_time += date_values[day] - 1;
	linear_time *= 86400;
	linear_time += date_values[hour] * 3600;
	linear_time += date_values[minute] * 60;
	linear_time += date_values[second];
}

date::date (unsigned lt)
	: linear_time(lt)
{
	set_from_linear();
}



date::date (const std::string& datestr)
{
	string::size_type moff = datestr.find("/");
	if (moff == string::npos)
		throw error("error in parsing date string, missed / for months");
	date_values[year] = atoi(datestr.substr(0, moff).c_str());
	string rest = datestr.substr(moff+1);
	string::size_type doff = rest.find("/");
	if (doff == string::npos)
		throw error("error in parsing date string, missed / for days");
	date_values[month] = atoi(rest.substr(0, doff).c_str());
	date_values[day] = atoi(rest.substr(doff+1).c_str());
	date_values[hour] = 0;
	date_values[minute] = 0;
	date_values[second] = 0;
	set_linear();
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

void date::load(const xml_elem& parent)
{
	xml_elem d = parent.child("date");
	string dy = d.attr("day");
	string tm = d.attr("time");
	date_values[year] = 0;
	date_values[month] = 0;
	date_values[day] = 0;
	date_values[hour] = 0;
	date_values[minute] = 0;
	date_values[second] = 0;
	sscanf(dy.c_str(), "%u/%u/%u", &date_values[year], &date_values[month], &date_values[day]);
	sscanf(tm.c_str(), "%u:%u:%u", &date_values[hour], &date_values[minute], &date_values[second]);
	// some plausibility checks
	if (date_values[year] < 1939 || date_values[year] > 1945)
		throw error("date year out of valid range");
	if (date_values[month] < 1 || date_values[month] > 12)
		throw error("date month out of valid range");
	if (date_values[day] < 1 || date_values[day] > 31)
		throw error("date day out of valid range");
	if (date_values[hour] > 23)
		throw error("date hour out of valid range");
	if (date_values[minute] > 59)
		throw error("date minute out of valid range");
	if (date_values[second] > 59)
		throw error("date second out of valid range");
	set_linear();
}

std::string date::to_str() const
{
	char tmp[32];
	sprintf(tmp, "%4u/%2u/%2u", date_values[year], date_values[month], date_values[day]);
	return std::string(tmp);
}

void date::save(xml_elem& parent) const
{
	xml_elem d = parent.add_child("date");
	char tmp[32];
	sprintf(tmp, "%4u/%2u/%2u", date_values[year], date_values[month], date_values[day]);
	d.set_attr(string(tmp), "day");
	sprintf(tmp, "%2u:%2u:%2u", date_values[hour], date_values[minute], date_values[second]);
	d.set_attr(string(tmp), "time");
}
