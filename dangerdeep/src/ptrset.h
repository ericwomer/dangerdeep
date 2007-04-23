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

#ifndef PTRSET_H
#define PTRSET_H

#include <vector>
#include <stdexcept>

// special vector with fix maximum size, but variable fill level
template <class T>
class ptrset
{
 protected:
	std::vector<T*> data;
	unsigned fill;

 private:
	ptrset(const ptrset& );
	ptrset& operator= (const ptrset& );

	static const unsigned minsize = 8;

 public:
	ptrset(unsigned capacity = 0) : data(capacity), fill(0) {}
	~ptrset() { clear(); }

	unsigned size() const { return fill; }
	unsigned capacity() const { return data.size(); }
	void clear() {
		for (unsigned i = 0; i < fill; ++i) {
			delete data[i];
			data[i] = 0;
		}
		fill = 0;
	}
	void push_back(T* ptr) {
		if (fill == data.size()) {
			// resize space to store more pointers
			unsigned newsize = (fill < minsize) ? minsize : fill * 2;
			data.resize(newsize);
		}
		data[fill++] = ptr;
	}

	void compact() {
		unsigned j = 0;
		for (unsigned i = 0; i < fill; ++i) {
			if (data[i]) {
				T* tmp = data[i];
				data[i] = 0;
				data[j] = tmp;
				++j;
			}
		}
		fill = j;
		// check if space can be compressed
		if (fill < data.size() / 2 && data.size() >= 2 * minsize) {
			data.resize(data.size()/2);
		}
	}

	void reset(unsigned n) {
		delete data[n];
		data[n] = 0;
	}

	T* release(unsigned n) {
		T* res = data[n];
		data[n] = 0;
		return res;
	}

	T* const& operator[](unsigned n) const { return data[n]; }
	T* const& at(unsigned n) const { if (n >= fill) throw std::out_of_range("ptrset const at"); return data[n]; }
};


#endif
