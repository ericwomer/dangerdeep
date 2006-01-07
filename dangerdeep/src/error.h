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

// Danger from the Deep, standard error/exception
// (C)+(W) by Thorsten Jordan. See LICENSE

#ifndef ERROR_H
#define ERROR_H

#include <string>
#include <exception>

// always throw this exception or a heir of it.
class error : public std::exception
{
 private:
	error();
	std::string msg;
 public:
	error(const std::string& s);
	virtual ~error() throw() {}
	const char* what() const throw() { return msg.c_str(); }
};

#endif
