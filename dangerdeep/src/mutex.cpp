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

// multithreading primites: mutex
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "mutex.h"
#include "error.h"
#include <SDL.h>

mutex::mutex()
{
	mtx = SDL_CreateMutex();
	if (!mtx)
		throw sdl_error("mutex creation failed");
}



mutex::~mutex()
{
	SDL_DestroyMutex(mtx);
}



void mutex::lock()
{
	if (SDL_mutexP(mtx) < 0)
		throw sdl_error("mutex lock failed");
}



void mutex::unlock()
{
	if (SDL_mutexV(mtx) < 0)
		throw sdl_error("mutex unlock failed");
}
