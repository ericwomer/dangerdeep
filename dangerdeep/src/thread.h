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

#ifndef THREAD_H
#define THREAD_H

#include "condvar.h"
#include <stdexcept>
#include <stdint.h>

/// base class for threads.
///@note Each thread should be an instance of a class that heirs
///	from class thread. Overload some member functions to fill in code for the thread,
///	like init(), deinit(), loop()
///	threads must be allocated with new.\n
///	heir from this class and implement init() / loop() / deinit()
class thread
{
 private:
	enum thread_state_t {
		THRSTAT_NONE,		// before start
		THRSTAT_RUNNING,	// normal operation
		THRSTAT_FINISHED,	// after thread has exited (can't be restarted)
		THRSTAT_INIT_FAILED,	// when init has failed
		THRSTAT_ABORTED		// when run/deinit has failed (internal error!)
	};

	struct SDL_Thread* thread_id;
	bool thread_abort_request;
	thread_state_t thread_state;
	mutex thread_state_mutex;
	condvar thread_start_cond;
	std::string thread_error_message; // to pass exception texts via thread boundaries
	const char* myname;

	void run();

	// can't copy thread objects
	thread(const thread& );
	thread& operator=(const thread& );
	thread();

 public:
	static int thread_entry(void* arg);
 protected:
	virtual ~thread();

	virtual void init() {}	///< will be called once after thread starts
	virtual void loop() {}	///< will be called periodically in main thread loop
	virtual void deinit() {} ///< will be called once after main thread loop ends

	bool abort_requested() const { return thread_abort_request; }
 public:
	/// create a thread
	thread(const char* name /* = "fixme"*/);

	/// abort thread (do not force, just request)
	virtual void request_abort();

	/// start thread execution
	/// thread will run in a loop, calling loop() each time. It will automatically check the abort flag
	/// anything that needs to be done before or after the loop can be placed in redefined
	/// constructors or destructors.
	void start();

	/// caller thread waits for completion of this thread,
	/// object storage is freed after thread completion.
	void join();

	/// destroy thread, try to abort and join it or delete the object if it hasn't started yet.
	void destruct();

	/// let this thread sleep
	///@param ms - sleep time in milliseconds
	static void sleep(unsigned ms);

	/// define SDL conform thread id type
	typedef uint32_t id;

	/// get ID of current (caller) thread
	static id get_my_id();

	/// get ID of this thread
	id get_id() const;

	/// request if thread runs
	bool is_running();

	/// an auto_ptr similar class for threads
	template<class T>
	class auto_ptr
	{
		auto_ptr(const auto_ptr& );
		auto_ptr& operator=(const auto_ptr& );

		T* p;
	public:
		/// construct thread auto pointer with thread pointer
		///@note will throw error when t is not a thread
		auto_ptr(T* t = 0) : p(0) { reset(t); }
		/// destruct thread auto pointer (will destruct thread)
		~auto_ptr() { reset(0); }
		/// reset pointer (destructs current thread)
		///@note will throw error when t is not a thread
		///@note seems bizarre, use with care!
		void reset(T* t = 0) {
			// extra paranoia test to ensure we handly only thread objects here
			if (t && (dynamic_cast<thread*>(t) == 0))
				throw std::invalid_argument("invalid pointer given to thread::auto_ptr!");
			if (p) p->destruct();
			p = t;
		}
		/// use pointer like normal pointer
		T* operator-> () const { return p; }
	};
};

#endif
