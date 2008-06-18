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

// ptrset - set of ptrs, like auto_ptr
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef PTRVECTOR_H
#define PTRVECTOR_H

#include <vector>
#include <stdexcept>
#include <memory>

// same as std::vector regarding the interface, but handles pointers.
template <class T>
class ptrvector
{
 protected:
	std::vector<T*> data;

 private:
	ptrvector(const ptrvector& );
	ptrvector& operator= (const ptrvector& );

 public:
	ptrvector(size_t capacity = 0) : data(capacity) {}
	~ptrvector() { clear(); }

	void resize(size_t newsize) {
		if (newsize < size()) {
			for (size_t i = newsize; i < size(); ++i) {
				delete data[i];
				// set to zero, because if resize throws an exception,
				// objects could get destructed twice
				data[i] = 0;
			}
		}
		data.resize(newsize);
	}
	size_t size() const { return data.size(); }
	size_t capacity() const { return data.capacity(); }
	void clear() {
		for (size_t i = 0; i < size(); ++i) {
			delete data[i];
			// set to zero, because if clear throws an exception,
			// objects could get destructed twice
			data[i] = 0;
		}
		data.clear();
	}

	// exception safe, so first create space, then store
	void push_back(std::auto_ptr<T> ptr) {
		data.push_back(0);
		data.back() = ptr.release();
	}

	T* const& operator[](size_t n) const { return data[n]; }
	T* const& at(size_t n) const { return data.at(n); }

	void reset(size_t n, T* ptr) { delete data[n]; data[n] = ptr; }
	bool empty() const { return data.empty(); }
};


#endif
