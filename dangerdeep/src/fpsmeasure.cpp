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

//
//  FPS measuring (C)+(W) 2008 Thorsten Jordan
//

#include "fpsmeasure.h"
#include "system.h"

fpsmeasure::fpsmeasure(float mi)
	: measure_interval(unsigned(1000 * mi + 0.5f)),
	  tm0(0), tm_lastframe(0), tm_lastmeasure(0), nr_frames(0), frames_lastmeasure(0), curfps(0),
	  slowest_frame(0), fastest_frame(0)
{
}

float fpsmeasure::account_frame()
{
	unsigned tm = sys().millisec();
	if (nr_frames == 0) {
		tm0 = tm;
		tm_lastframe = tm;
	} else {
		slowest_frame = std::max(slowest_frame, tm - tm_lastframe);
		fastest_frame = std::max(fastest_frame, tm - tm_lastframe);
		tm_lastframe = tm;
	}
	++nr_frames;
	if (tm >= tm_lastmeasure + measure_interval) {
		// measure
		curfps = (nr_frames - frames_lastmeasure) * 1000.0f / (tm - tm_lastmeasure);
		tm_lastmeasure = tm;
		frames_lastmeasure = nr_frames;
	}
	return curfps;
}

float fpsmeasure::get_total_fps() const
{
	unsigned tm = sys().millisec();
	return (nr_frames - 1) * 1000.0f / (tm - tm0);
}
