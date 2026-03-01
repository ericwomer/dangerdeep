/*
Danger from the Deep - Open source submarine simulation
Copyright (C) 2003-2006  Thorsten Jordan, Luis Barrancos and others.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
*/

#include "string_split.h"
#include <sstream>

std::list<std::string> string_split(const std::string &src, char splitter) {
    std::list<std::string> result;
    if (src.empty()) {
        result.push_back(std::string());
        return result;
    }
    std::istringstream iss(src);
    std::string token;
    while (std::getline(iss, token, splitter)) {
        result.push_back(token);
    }
    // getline no añade token vacío si la cadena termina en splitter; mantener compatibilidad
    if (src.back() == splitter) {
        result.push_back(std::string());
    }
    return result;
}
