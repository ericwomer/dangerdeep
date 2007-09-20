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

#include "log.h"
#include "mutex.h"
#include <list>
#include <string>
#include <SDL.h>

log_stream::log_stream(level l)
	: mylevel(l)
{
}

// fixme: make access to list thread safe!


std::ostream& log_stream::endl(std::ostream& os)
{
	// add a line to the log file
	return os;
}



struct log_msg
{
	log_stream::level lvl;
	Uint32 tid;
	Uint32 time;
	std::string msg;

	log_msg(log_stream::level l, const std::string& m)
		: lvl(l),
		  tid(SDL_ThreadID()),
		  time(SDL_GetTicks()),
		  msg(m)
	{
	}

	std::string pretty_print() const {
		// fixme, use ansi coloring
		return msg;
	}
};

class log_internal
{
public:
	mutex mtx;
	std::list<log_msg> loglines;
	log_internal() {}
};


log::log()
	: mylogint(0)
{
	mylogint = new log_internal();
}

void log::append(log_stream::level l, const std::string& msg)
{
	mutex_locker ml(mylogint->mtx);
	mylogint->loglines.push_back(log_msg(l, msg));
}

void log::write(std::ostream& out, log_stream::level limit_level) const
{
	// process log_msg and make ANSI colored text lines of it
	mutex_locker ml(mylogint->mtx);
	for (std::list<log_msg>::const_iterator it = mylogint->loglines.begin();
	     it != mylogint->loglines.end(); ++it) {
		out << it->pretty_print() << std::endl;
	}
}
