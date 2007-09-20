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
//  A logging implementation
//

#ifndef LOG_H
#define LOG_H

#include <iostream>

class log_stream : public std::ostream
{
 public:
	enum level {
		WARNING,
		INFO,
		DEBUG,
	};

	log_stream(level l);
	std::ostream& endl(std::ostream& os);

 protected:
	level mylevel;
};

class log
{
 public:
	static log_stream debug, info, warning;
	log();
	void write(std::ostream& out, log_stream::level limit_level) const;
	void append(log_stream::level l, const std::string& msg);

 protected:
	class log_internal* mylogint;
};

#endif
