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

#include <string>

#include "thread.h"
#include "error.h"
#include "system.h"
#include "log.h"
#include <SDL.h>
#include <SDL_thread.h>

/* fixme: SDL does not allow to set stack size of thread... */

int thread::thread_entry(void* arg)
{
	thread* t = (thread*)arg;
	try {
		t->run();
	}
	catch (std::exception& e) {
		t->thread_error_message = e.what();
		return -1;
	}
	catch (...) {
		t->thread_error_message = "UNKNOWN";
		return -2;
	}
	return 0;
}



thread::thread(const char* name)
	: thread_id(0),
	  thread_abort_request(false),
	  thread_state(THRSTAT_NONE),
	  myname(name)
{
}



void thread::run()
{
	try {
		log::instance().new_thread(myname);
		init();
	}
	catch (std::exception& e) {
		// failed to initialize, report that
		mutex_locker ml(thread_state_mutex);
		thread_error_message = e.what();
		thread_state = THRSTAT_INIT_FAILED;
		thread_start_cond.signal();
		throw;
	}
	catch (...) {
		// failed to initialize, report that
		mutex_locker ml(thread_state_mutex);
		thread_error_message = "UNKNOWN";
		thread_state = THRSTAT_INIT_FAILED;
		thread_start_cond.signal();
		throw;
	}
	// initialization was successfully, report that
	{
		mutex_locker ml(thread_state_mutex);
		thread_state = THRSTAT_RUNNING;
		thread_start_cond.signal();
	}
	try {
		while (!abort_requested()) {
			loop();
		}
		deinit();
		log::instance().end_thread();
	}
	catch (std::exception& e) {
		// thread execution failed
		mutex_locker ml(thread_state_mutex);
		thread_error_message = e.what();
		thread_state = THRSTAT_ABORTED;
		throw;
	}
	catch (...) {
		// thread execution failed
		mutex_locker ml(thread_state_mutex);
		thread_error_message = "UNKNOWN";
		thread_state = THRSTAT_ABORTED;
		throw;
	}
	// normal execution finished
	mutex_locker ml(thread_state_mutex);
	thread_state = THRSTAT_FINISHED;
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
	mutex_locker ml(thread_state_mutex);
	if (thread_state != THRSTAT_NONE)
		throw error("thread already started, but start() called again");
	thread_id = SDL_CreateThread(thread_entry, this);
	if (!thread_id)
		throw sdl_error("thread start failed");
	// we could wait with timeout, but how long? init could take any time...
	thread_start_cond.wait(thread_state_mutex);
	// now check if thread has started
	if (thread_state == THRSTAT_INIT_FAILED)
		throw std::runtime_error(("thread start failed: ") + thread_error_message);
	// very rare, but possible
	else if (thread_state == THRSTAT_ABORTED)
		throw std::runtime_error(("thread run failed: ") + thread_error_message);
}



void thread::join()
{
	int result = 0;
	SDL_WaitThread(thread_id, &result);
	delete this;
	if (result < 0)
		throw error(std::string("thread aborted with error: ") + thread_error_message);

}



void thread::destruct()
{
	thread_state_t ts = THRSTAT_NONE;
	{
		mutex_locker ml(thread_state_mutex);
		ts = thread_state;
	}
	// request if thread runs, in that case send abort request
	if (ts == THRSTAT_RUNNING)
		request_abort();
	// request if thread has ever run, in that case we need to join
	if (thread_state != THRSTAT_NONE)
		join();
	else
		delete this;
}



void thread::sleep(unsigned ms)
{
	SDL_Delay(ms);
}



thread::id thread::get_my_id()
{
	return SDL_ThreadID();
}



thread::id thread::get_id() const
{
	return SDL_GetThreadID(thread_id);
}



bool thread::is_running()
{
	// only reading is normally safe, but not for multi-core architectures.
	mutex_locker ml(thread_state_mutex);
	return thread_state == THRSTAT_RUNNING;
}
