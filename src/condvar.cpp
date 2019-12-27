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

// multithreading primitives: condition variable
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "condvar.h"
#include "error.h"
#include <SDL.h>

condvar::condvar()
{
	cdv = SDL_CreateCond();
	if (!cdv)
		throw sdl_error("condvar creation failed");
}



condvar::~condvar()
{
	SDL_DestroyCond(cdv);
}



void condvar::wait(mutex& m)
{
	if (SDL_CondWait(cdv, m.mtx) < 0)
		throw sdl_error("condvar wait failed");
}



bool condvar::timed_wait(mutex& m, unsigned ms)
{
	int res = SDL_CondWaitTimeout(cdv, m.mtx, ms);
	if (res == SDL_MUTEX_TIMEDOUT)
		return false;
	if (res < 0)
		throw sdl_error("condvar timed_wait failed");
	return true;
}



void condvar::signal()
{
	if (SDL_CondBroadcast(cdv) < 0)
		throw sdl_error("condvar signal failed");
}
