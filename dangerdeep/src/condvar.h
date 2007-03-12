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

#ifndef CONDVAR_H
#define CONDVAR_H

#include "mutex.h"

/// A classical condition variable.
///@note Condition variables work toger with class mutex.
class condvar
{
 protected:
	struct SDL_cond* cdv;
 private:
	condvar(const condvar& );
	condvar& operator=(const condvar& );
 public:
	/// create condition variable
	condvar();

	/// destroy condition variable
	~condvar();

	/// wait on condition
	///@param m - mutex to encapsulate waiting condition
	void wait(mutex& m);

	/// wait on condition with timeout
	///@param m - mutex to encapsulate waiting condition
	///@param ms - timeout value in milliseconds
	///@return true when woken up by signal, false on timeout
	bool timed_wait(mutex& m, unsigned ms);

	/// send signal to threads that are waiting on the condition.
	///@note Note that before sending the signal you must make the condition false that would
	///	let the other thread call the wait(), or race conditions can occour.
	///	Code that changes the condition must be encapsulated in a mutex-lock with the SAME mutex
	///	that is used for the wait.\n
	///	So typically call lock(); cond = false; signal(); unlock();
	void signal();
};

#endif
