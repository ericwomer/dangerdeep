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


#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "oglext/OglExt.h"

#include <vector>

#include "xml.h"
#include "system.h"
#include "stars.h"
#include "datadirs.h"



const float MIN_TWINKLE_MAGNITUDE = 5.2f; // stars with mag > this value twinkle and all the other have alpha [1.0, 0.6]


stars::stars(const float max_magnitude)
 : star_count_static(0), star_count(0)
{
	//	load star data
	xml_doc doc(get_data_dir() + "environment/stars_data.xml");
	doc.load();
	xml_elem root = doc.child("stars_data");
	
	std::vector<vector3f> star_pos;
	star_pos.reserve(3000);
	star_colors.reserve(3000);
	
	for(xml_elem::iterator it=root.iterate(); !it.end(); it.next())
	{
		xml_elem star_node = it.elem();
		
		float mag = star_node.attrf("mag");
		
		if(mag > max_magnitude) break;	//	skip star if its magnitude > max (input sorted by mag)

		colorf col;

		col.a = 1.2f - mag/10;	// mag=6 => a=0.6
		if(col.a > 1.0f) col.a = 1.0f;

		vector3f pos( star_node.attrf("x"), star_node.attrf("y"), star_node.attrf("z"));
		pos.normalize();

		char spectrum = star_node.attr("spectrum")[0];
		switch (spectrum) {
			case 'O': 
				col = colorf(0.8 /1.3,  1.0 /1.3, 1.3 /1.3);
				break;
			case 'B': 
				col = colorf(0.9 /1.2,  1.0 /1.2, 1.2 /1.2);
				break;
			case 'A': 
				col = colorf(0.95/1.15, 1.0 /1.15,1.15/1.15);
				break;
			case 'F': 
				col = colorf(1.05/1.05, 1.0 /1.05,1.05/1.05);
				break;
			case 'G': 
				col = colorf(1.3 /1.3,  1.0 /1.3, 0.9 /1.3);
				break;
			case 'K': 
				col = colorf(1.15/1.15, 0.95/1.15,0.8 /1.15);
				break;
			case 'M': 
				col = colorf(1.15/1.15, 0.85/1.15,0.8 /1.15);
				break;
			case 'R': 
				col = colorf(1.3 /1.3,  0.85/1.3, 0.6 /1.3);
				break;
			case 'S': 
				col = colorf(1.5 /1.5,  0.8 /1.5, 0.2 /1.5);
				break;
			case 'N': 
				col = colorf(1.5 /1.5,  0.8 /1.5, 0.2 /1.5);
				break;
			case 'W': 
				col = colorf(1.5 /1.5,  0.8 /1.5, 0.2 /1.5);
				break;
			case 'X': 
				col = colorf(1.0,  1.0, 1.0);
				break;
			default: 
				col = colorf(1.0,  1.0, 1.0);
				break;

		}
		
		star_pos.push_back(pos);
		star_colors.push_back(col);
		star_count++;
		
		if(mag < MIN_TWINKLE_MAGNITUDE)
			star_count_static++;
	}

	sys().add_console("%d stars loaded.", star_pos.size());
	
	star_positions.init_data(star_pos.size() * sizeof(vector3f), &star_pos[0], GL_STATIC_DRAW);
}



void stars::display(const float max_view_dist) const
{
	// render stars
	glBindTexture(GL_TEXTURE_2D, 0);

	glEnable(GL_POINT_SMOOTH);

	glPushMatrix();
	glScalef(max_view_dist * 0.95, max_view_dist * 0.95, max_view_dist * 0.95);

	glEnableClientState(GL_VERTEX_ARRAY);
	star_positions.bind();
	glVertexPointer(3, GL_FLOAT, 0, 0);
	glEnableClientState(GL_COLOR_ARRAY);
	// update alpha values for twinkle stars
	colorf *color = (colorf *) &star_colors[star_count_static];
	for(unsigned int i = star_count_static; i < star_count; i++)	
	{
		color->a = 0.5f + ( 0.3f*rand()/RAND_MAX );  //	alpha in range [0.5, 0.8]
		color++;
	}
	star_colors_VBO.init_data(star_colors.size() * sizeof(colorf), &star_colors[0], GL_STREAM_DRAW);
	star_colors_VBO.bind();
	glColorPointer(4, GL_FLOAT, 0, 0);
	glDrawArrays(GL_POINTS, 0, star_count);
	
	star_positions.unbind();
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	glPopMatrix();

	glDisable(GL_POINT_SMOOTH);
}
