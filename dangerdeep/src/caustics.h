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

// underwater caustic simulation
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef CAUSTICS_H
#define CAUSTICS_H

/*
	This class contains map for underwater caustic
*/


#include "texture.h"




class caustics
{
protected:
	double mytime;
	std::vector<texture *> texture_pointers;
	unsigned int current_texture;

public:
	caustics();
	~caustics();

	void set_time(double tm);
	texture *get_map() const;
};




#endif
