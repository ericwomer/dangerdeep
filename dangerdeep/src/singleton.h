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

// this class holds the game's configuration
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef SINGLETON_H
#define SINGLETON_H

template<typename D>
class singleton
{
 private:
	static D*& instance_ptr() { static D* myinstanceptr = 0; return myinstanceptr; }

 public:
	// since D is constructed not before first call, it avoids
	// the static initialization order fiasco.
	static D& instance() { D*& p = instance_ptr(); if (!p) p = new D(); return *p; }

	static void create_instance(D* ptr) { D*& p = instance_ptr(); delete p; p = ptr; }

	static void release_instance() { D*& p = instance_ptr(); delete p; p = 0; }

 protected:
	singleton() {}

 private:
	singleton(const singleton& );
	singleton& operator= (const singleton& );
};

#endif
