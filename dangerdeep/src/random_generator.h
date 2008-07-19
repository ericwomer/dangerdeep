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
//  A generic generator for random numbers (C)+(W) 2008 Thorsten Jordan
//

#ifndef RANDOM_GENERATOR_H
#define RANDOM_GENERATOR_H

class random_generator
{
 public:
	random_generator(unsigned seed = 0) : reg(seed) {}
	virtual ~random_generator() {}
	virtual unsigned rnd() { chaos(); return reg; }
	virtual float rndf() { unsigned n = rnd(); return float(double(n)/unsigned(-1)); }
	virtual void set_seed(unsigned seed) { reg = seed; }

 protected:
	virtual void chaos() {
		reg = reg * 9699691 + 223092870;
	}

	unsigned reg;
};

#endif
