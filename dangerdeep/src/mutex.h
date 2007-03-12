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

#ifndef MUTEX_H
#define MUTEX_H

/// A classical mutex.
class mutex
{
	//friend class condvar; // later
 protected:
	struct SDL_mutex* mtx;
 private:
	mutex(const mutex& );
	mutex& operator=(const mutex& );
 public:
	/// create a mutex
	mutex();

	/// destroy mutex
	~mutex();

	/// lock mutex
	void lock();

	/// unlock the mutex
	void unlock();
};



/// A handy helper class for mutexes.
///@note Create a local object of that class and give it a mutex. It will lock the mutex
///	in its constructor and automatically unlock it in the destructor.
///	That way the unlock() is done automagically when a throw or return is executed anywhere
///	inside the block or function that the object is declared in.
class mutex_locker
{
 protected:
	mutex& mymutex;
 private:
	mutex_locker();
	mutex_locker(const mutex_locker& );
	mutex_locker& operator=(const mutex_locker& );
 public:
	/// create mutex locker
	///@param mtx - mutex to lock
	///@note will instantly lock the mutex that was given as parameter
	mutex_locker(mutex& mtx) : mymutex(mtx) { mymutex.lock(); }

	/// destroy mutex locker
	///@note will unlock the mutex that was given to the constructor
	~mutex_locker() { mymutex.unlock(); }
};

#endif
