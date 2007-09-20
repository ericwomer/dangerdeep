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

#include <sstream>

#define log_template(x, y) do { std::ostringstream oss; oss << x; log::instance().append(log::y, oss.str()); } while(0)
#define log_debug(x) log_template(x, DEBUG)
#define log_info(x) log_template(x, INFO)
#define log_warning(x) log_template(x, WARNING)

class log
{
 public:
	enum level {
		WARNING,
		INFO,
		DEBUG,
	};

	static log& instance();
	log();
	void write(std::ostream& out, log::level limit_level) const;
	void append(log::level l, const std::string& msg);

 protected:
	class log_internal* mylogint;
	static log* myinstance;
};

#endif
