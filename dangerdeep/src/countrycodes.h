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

// key name definitions
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef COUNTRYCODES_H
#define COUNTRYCODES_H

#include "date.h"

enum countrycode
{
	UNKNOWNCOUNTRY,
	GERMANY,
	GREATBRITAIN,
	FRANCE,
	UNITEDSTATES,
	CANADA,
	ITALY,
	NORWAY,
	SWEDEN,
	NETHERLANDS,
	DENMARK,
	BELGIUM,
	GREECE,
	SOWJETUNION,
	JAPAN,
	BRAZIL,
	IRELAND,
	PORTUGAL,
	TURKEY,
	AUSTRALIA,
	NR_OF_COUNTRIES
};

enum partycode
{
	UNKNOWNPARTY,
	NEUTRAL,
	ALLIES,
	AXIS,
	NR_OF_PARTIES
};

partycode party_of_country(countrycode c, date d);

#endif
