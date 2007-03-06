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

// ptrlist - list of ptrs, like auto_ptr
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef PTRLIST_H
#define PTRLIST_H

#include <list>
#include <stdexcept>

// same as std::list regarding the interface (partly), but handles pointers.
template <class T>
class ptrlist
{
 protected:
	std::list<T*> data;

 private:
	ptrlist(const ptrlist& );
	ptrlist& operator= (const ptrlist& );

 public:
	ptrlist() {}
	~ptrlist() { clear(); }

	size_t size() const { return data.size(); }
	void clear() {
		while (!data.empty()) {
			delete data.front();
			data.pop_front();
		}
	}

	// exception safe, so first create space, then store
	void push_back(std::auto_ptr<T> ptr) {
		data.push_back(0);
		data.back() = ptr.release();
	}
	void push_front(std::auto_ptr<T> ptr) {
		data.push_front(0);
		data.front() = ptr.release();
	}
	void push_back(T* ptr) {
		std::auto_ptr<T> p(ptr);
		push_back(p);
	}
	void push_front(T* ptr) {
		std::auto_ptr<T> p(ptr);
		push_front(p);
	}

	void pop_front() {
		if (!data.empty()) {
			delete data.front();
			data.pop_front();
		}
	}
	void pop_back() {
		if (!data.empty()) {
			delete data.back();
			data.pop_back();
		}
	}

	T* const& front() const { return *data.front(); }
	T* const& back() const { return *data.back(); }

	bool empty() const { return data.empty(); }

	struct const_iterator
	{
		typename std::list<T*>::const_iterator it;

		const_iterator(typename std::list<T*>::const_iterator i) : it(i) {}
		T& operator* () const { return *(*it); }
		T* operator-> () const { return *it; }
		const_iterator& operator++ () { ++it; return *this; }
		const_iterator& operator-- () { --it; return *this; }
		bool operator== (const const_iterator& other) const { return it == other.it; }
		bool operator!= (const const_iterator& other) const { return it != other.it; }
	};

	const_iterator begin() const { return const_iterator(data.begin()); }
	const_iterator end() const { return const_iterator(data.end()); }
};


#endif
