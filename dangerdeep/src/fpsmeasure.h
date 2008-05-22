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

#ifndef FPSMEASURE_H
#define FPSMEASURE_H

class fpsmeasure
{
 public:
	fpsmeasure(float measure_interval = 5.0f);
	float account_frame();
	float get_total_fps() const;
	unsigned get_slowest_frame_time_ms() const { return slowest_frame; }
	unsigned get_fastest_frame_time_ms() const { return fastest_frame; }

 protected:
	const unsigned measure_interval;
	unsigned tm0, tm_lastframe, tm_lastmeasure, nr_frames, frames_lastmeasure;
	float curfps;
	unsigned slowest_frame, fastest_frame;
};

#endif
