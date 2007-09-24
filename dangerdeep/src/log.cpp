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
#include <map>
#include <string>
#include <iostream>
#include <stdexcept>
#include <SDL.h>


struct log_msg
{
	log::level lvl;
	Uint32 tid;
	Uint32 time;
	std::string msg;

	log_msg(log::level l, const std::string& m)
		: lvl(l),
		  tid(SDL_ThreadID()),
		  time(SDL_GetTicks()),
		  msg(m)
	{
	}

	std::string pretty_print() const
	{
		std::ostringstream oss;
		switch (lvl) {
		case log::WARNING:
			oss << "\033[1;31m";
			break;
		case log::INFO:
			oss << "\033[1;34m";
			break;
		case log::SYSINFO:
			oss << "\033[1;33m";
			break;
		case log::DEBUG:
			oss << "\033[1;32m";
			break;
		default:
			oss << "\033[0m";
		}
		oss << "[" << log::instance().get_thread_name() << "] <" << std::dec << time << "> " << msg << "\033[0m";
		return oss.str();
	}

	std::string pretty_print_console() const
	{
		std::ostringstream oss;
		switch (lvl) {
		case log::WARNING:
			oss << "$ff8080";
			break;
		case log::INFO:
			oss << "$c0c0ff";
			break;
		case log::SYSINFO:
			oss << "$ffff00";
			break;
		case log::DEBUG:
			oss << "$b0ffb0";
			break;
		default:
			oss << "$c0c0c0";
		}
		oss << "[" << log::instance().get_thread_name() << "] <" << std::dec << time << "> " << msg;
		return oss.str();
	}
};

class log_internal
{
public:
	mutex mtx;
	std::list<log_msg> loglines;
	std::map<Uint32, const char* > threadnames;
	log_internal() {}
};


log::log()
	: mylogint(0)
{
	mylogint = new log_internal();
	mylogint->threadnames[SDL_ThreadID()] = "__main__";
}

log* log::myinstance = 0;

bool log::copy_output_to_console = false;

log& log::instance()
{
	if (!myinstance)
		myinstance = new log();
	return *myinstance;
}

void log::append(log::level l, const std::string& msg)
{
	mutex_locker ml(mylogint->mtx);
	mylogint->loglines.push_back(log_msg(l, msg));
	if (copy_output_to_console) {
		std::cout << mylogint->loglines.back().pretty_print() << std::endl;
	}
}

void log::write(std::ostream& out, log::level limit_level) const
{
	// process log_msg and make ANSI colored text lines of it
	mutex_locker ml(mylogint->mtx);
	for (std::list<log_msg>::const_iterator it = mylogint->loglines.begin();
	     it != mylogint->loglines.end(); ++it) {
		if (it->lvl <= limit_level)
			out << it->pretty_print() << std::endl;
	}
}

std::string log::get_last_n_lines(unsigned n) const
{
	std::string result;
	mutex_locker ml(mylogint->mtx);
	unsigned l = mylogint->loglines.size();
	if (n > l) {
		for (unsigned k = 0; k < n - l; ++k)
			result += "\n";
		n = l;
	}
	std::list<log_msg>::const_iterator it = mylogint->loglines.end();
	for ( ; n > 0; --n)
		--it;
	for ( ; it != mylogint->loglines.end(); ++it) {
		result += it->pretty_print_console() + "\n";
	}
	return result;
}

void log::new_thread(const char* name)
{
	{
		mutex_locker ml(mylogint->mtx);
		mylogint->threadnames[SDL_ThreadID()] = name;
	}
	log_sysinfo("---------- < NEW > THREAD ----------");
}

void log::end_thread()
{
	log_sysinfo("---------- > END < THREAD ----------");
	mutex_locker ml(mylogint->mtx);
	mylogint->threadnames.erase(SDL_ThreadID());
}

const char* log::get_thread_name() const
{
	std::map<Uint32, const char * >::const_iterator it = mylogint->threadnames.find(SDL_ThreadID());
	if (it == mylogint->threadnames.end())
		throw std::runtime_error("no thread name registered for thread! BUG!");
	return it->second;
}

