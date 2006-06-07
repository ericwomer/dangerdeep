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

#ifndef MOON_H
#define MOON_H


#include <vector>


#include "texture.h"


///\brief Moon rendering.
class moon
{
	private:
		struct phase_data {
			double time;			//	time
			unsigned phase;		//	id of moon phase texture file
		};

		std::vector<std::string> moon_texture_files;
		std::vector<phase_data> phases;
		unsigned last_phase_id;		//	last position in phases vector
		unsigned last_texture_id;		//	last position in moon_texture_files vector
		texture::ptr moon_texture;

		unsigned get_phase_id(const double time) const;
		unsigned get_texture_id(const phase_data &phase1, const phase_data &phase2, const double time) const;

	public:
		moon();

		void update_moon_texture(const double time);
		texture *get_texture() const;
};

#endif
