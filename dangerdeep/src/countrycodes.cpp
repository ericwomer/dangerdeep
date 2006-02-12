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

#include "countrycodes.h"

const char* countrycodes[NR_OF_COUNTRIES] =
{
	"??",
	"DE",
	"GB",
	"FR",
	"US",
	"CD",
	"IT",
	"NO",
	"SW",
	"NL",
	"DK",
	"BE",
	"GR",
	"SU",
	"JP",
	"BZ",
	"IR",
	"PT",
	"TK",
	"AU"
};

const char* parties[NR_OF_PARTIES] = 
{
	"?",
	"NEUTRAL",
	"ALLIES",
	"AXIS"
};

partycode party_of_country(countrycode c, date d)
{
	switch (c) {
	case UNKNOWNCOUNTRY:
		return UNKNOWNPARTY;
	case GERMANY:
		return AXIS;
	case GREATBRITAIN:
		return ALLIES;
	case FRANCE:
		if (d < date(1940, 6, 30) || d > date(1944, 8, 30)) // fixme: we need more accurate dates.
			return ALLIES;
		return AXIS;
	case UNITEDSTATES:
		if (d < date(1941, 12, 11))	// fixme: exact date?
			return NEUTRAL;
		return ALLIES;
	case CANADA:
		return ALLIES;
	case ITALY:
		if (d < date(1943, 8, 30)) // fixme: we need more accurate dates.
			return AXIS;
		return ALLIES;
	case NORWAY:
		return ALLIES;	// exile, but neutral before 1940
	case SWEDEN:
		return NEUTRAL;
	case NETHERLANDS:
		return ALLIES;	// neutral before 1940
	case DENMARK:
		return ALLIES;	// neutral before 1940
	case BELGIUM:
		return ALLIES;	// neutral before 1940
	case GREECE:
		return ALLIES;	// neutral before 1941
	case SOWJETUNION:
		if (d < date(1941, 6, 22))
			return NEUTRAL;
		return ALLIES;
	case JAPAN:
		return AXIS;
	case BRAZIL:
		return ALLIES;	// neutral at first
	case IRELAND:
		return NEUTRAL;
	case PORTUGAL:
		return NEUTRAL;
	case TURKEY:
		return ALLIES;	// ???
	case AUSTRALIA:
		return ALLIES;
	default:
		return UNKNOWNPARTY;
	}
}
