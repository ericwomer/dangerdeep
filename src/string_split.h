/*
Danger from the Deep - Open source submarine simulation
Copyright (C) 2003-2006  Thorsten Jordan, Luis Barrancos and others.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
*/

#ifndef STRING_SPLIT_H
#define STRING_SPLIT_H

#include <list>
#include <string>

std::list<std::string> string_split(const std::string &src, char splitter = ',');

#endif
