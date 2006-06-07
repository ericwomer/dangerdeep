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

#include "moon.h"
#include "xml.h"
#include "texture.h"
#include "datadirs.h"



unsigned moon::get_phase_id(const double time) const
{
	if(phases[last_phase_id+1].time<=time) {
	//	find new time interval
		//	try next time interval
		if(phases[last_phase_id+2].time>time) return last_phase_id+1;

		//	binary search
		unsigned begin=0, end=phases.size()-2;
		while( end-begin > 1) {
			unsigned i = (begin+end)/2;
			if( phases[i].time > time ) end = i;
			else begin = i;
		}

		return begin;
	}

	return last_phase_id;
}


unsigned moon::get_texture_id(const phase_data &phase1, const phase_data &phase2, const double time) const
{
	unsigned tmp = phase2.phase;
	if(phase2.phase<phase1.phase) tmp += moon_texture_files.size()+1;
	unsigned phase = phase1.phase + (unsigned) ( (tmp-phase1.phase)*(time-phase1.time)/(phase2.time-phase1.time) );

	return phase%moon_texture_files.size();
}


moon::moon()
 : last_phase_id(0)
{
	xml_doc moon_data(get_data_dir()+"environment/moon_data.xml");
	moon_data.load();

	xml_elem parent_node = moon_data.child("moon_data");

	xml_elem textures_node = parent_node.child("textures");
	for (xml_elem::iterator it = textures_node.iterate("texture"); !it.end(); it.next()) {
		moon_texture_files.push_back(it.elem().attr("filename"));
	}

	xml_elem phases_node = parent_node.child("phases");
	for (xml_elem::iterator it = phases_node.iterate("moon_phase"); !it.end(); it.next()) {
		phase_data phase;
		phase.time = it.elem().attrf("time");
		phase.phase = it.elem().attri("phase");

		phases.push_back(phase);
	}

	moon_texture = texture::ptr(new texture(get_texture_dir()+moon_texture_files[0], texture::LINEAR));
}


void moon::update_moon_texture(const double time)
{
	last_phase_id = get_phase_id(time);

	const phase_data &phase1 = phases[last_phase_id];
	const phase_data &phase2 = phases[last_phase_id+1];
	unsigned texture_id = get_texture_id(phase1, phase2, time);

	if(last_texture_id != texture_id) {
		moon_texture.reset( new texture(get_texture_dir()+moon_texture_files[texture_id], texture::LINEAR) );
		last_texture_id = texture_id;
	}
}


texture *moon::get_texture() const
{
	return moon_texture.get();
}
