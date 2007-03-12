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

// multithreading primitives: thread
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "thread.h"
#include "error.h"
#include <SDL.h>

/* fixme: SDL does not allow to set stack size of thread... */

int thread::thread_entry(void* arg)
{
	thread* t = (thread*)arg;
	try {
		t->run();
	}
	catch (std::exception& e) {
		return -1;
	}
	catch (...) {
		return -2;
	}
	return 0;
}



thread::thread()
	: thread_id(0),
	  thread_abort_request(false),
	  thread_started(false)
{
}



void thread::run()
{
	init();
	while (!abort_requested()) {
		loop();
	}
	deinit();
}



thread::~thread()
{
}



void thread::request_abort()
{
	thread_abort_request = true;
}



void thread::start()
{
	if (thread_abort_request)
		throw error("thread abort requested, but start() called");
	thread_id = SDL_CreateThread(thread_entry, this);
	if (!thread_id)
		throw sdl_error("thread start failed");
}



void thread::join()
{
	int result = 0;
	SDL_WaitThread(thread_id, &result);
	delete this;
	if (result < 0)
		throw error("thread aborted with error");

}



void thread::destruct()
{
	if (thread_started) {
		request_abort();
		join();
	} else {
		delete this;
	}
}



void thread::sleep(unsigned ms)
{
	SDL_Delay(ms);
}
