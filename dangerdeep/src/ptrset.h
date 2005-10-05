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

	// this function must not be called while iterating a ptrset manually!
	// use for_each to be safe!
	// note that func *may* iterate over the ptrset though!
	template <typename U>
	void for_each_with_erase(U func) {
		for (unsigned i = 0; i < fill; ++i) {
			func(data[i]);
			if (data[i] == 0) {	// erased in func!
				--fill;
				data[i] = data[fill];
				data[fill] = 0;
				--i;	// handle that entry in next loop!
			}
		}
		// check if space can be compressed
		if (fill < data.size() / 2 && data.size() >= 2 * minsize) {
			data.resize(data.size()/2);
		}
	}
	template <typename U>
	void for_each(U func) const {
		for (unsigned i = 0; i < fill; ++i) {
			func(data[i]);
		}
	}

	T* const& operator[](unsigned n) const { return data[n]; }
	T* const& at(unsigned n) const { if (n >= fill) throw std::out_of_range("ptrset const at"); return data[n]; }
};


#endif
