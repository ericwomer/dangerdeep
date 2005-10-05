// ptrvector
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef PTRVECTOR_H
#define PTRVECTOR_H

#include <vector>
#include <stdexcept>

template <class T>
class ptrvector
{
 protected:
	std::vector<T*> data;

	void data_delete(unsigned a, unsigned b) {
		for (unsigned i = a; i < b; ++i) {
			delete data[i];
		}
	}

	void pack_priv(unsigned emptyentries) {
		unsigned filled = data.size() - emptyentries;
		if (filled == data.size()) return;
		std::vector<T*> data2(filled);
		for (unsigned i = 0, j = 0; i < data.size(); ++i)
			if (data[i])
				data2[j++] = data[i];
		data.swap(data2);
	}

 private:
	ptrvector(const ptrvector& );
	ptrvector& operator= (const ptrvector& );

 public:
	ptrvector(unsigned size = 0) : data(size) {}
	~ptrvector() { clear(); }

	unsigned size() const { return data.size(); }
	unsigned capacity() const { return data.capacity(); }
	void clear() { data_delete(0, data.size()); }
	void reserve(unsigned n) {
		if (n >= data.size())
			data.reserve(n); //fixme, was wenn n<data.size() wird dann deleted?
	}
	void resize(unsigned n) {
		if (n < data.size())
			data_delete(n, data.size());
		data.resize(n);
	}
	void push_back(T* ptr) {
		data.push_back(ptr);
	}

	template <typename U>
	void for_each(U func) {
		unsigned emptyentries = 0;
		for (unsigned i = 0; i < data.size(); ++i) {
			if (data[i])
				func(data[i]);
			else
				++emptyentries;
		}
		if (emptyentries > 0)
			pack_priv(emptyentries);
	}
	template <typename U>
	void for_each_const(U func) const {
		for (unsigned i = 0; i < data.size(); ++i) {
			if (data[i])
				func(data[i]);
		}
	}

	void pack() {
		unsigned emptyentries = 0;
		for (unsigned i = 0; i < data.size(); ++i)
			if (data[i] == 0)
				++emptyentries;
		pack_priv(emptyentries);
	}

	T*& operator[](unsigned n) { return data[n]; }
	T* const& operator[](unsigned n) const { return data[n]; }
	T*& at(unsigned n) { return data.at(n); }
	T* const& at(unsigned n) const { return data.at(n); }
};



// special vector with fix maximum size, but variable fill level
template <class T>
class dynptrvector
{
 protected:
	std::vector<T*> data;
	unsigned fill;

 private:
	dynptrvector(const dynptrvector& );
	dynptrvector& operator= (const dynptrvector& );

 public:
	dynptrvector(unsigned capacity = 0) : data(capacity), fill(0) {}
	~dynptrvector() { clear(); }

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
		if (fill >= data.size())
			throw std::out_of_range("dynptrvector push_back out of space");
		data[fill++] = ptr;
	}
	void erase(unsigned idx) {
		if (idx >= fill)
			throw std::out_of_range("dynptrvector erase out of range");
		delete data[idx];
		--fill;
		data[idx] = data[fill];
		data[fill] = 0;
	}

	template <typename U>
	void for_each(U func) {
		for (unsigned i = 0; i < fill; ++i) {
			func(data[i]);
			if (data[i] == 0) {	// erased in func!
				--fill;
				data[i] = data[fill];
				data[fill] = 0;
				--i;	// handle that entry in next loop!
			}
		}
	}
	template <typename U>
	void for_each_const(U func) const {
		for (unsigned i = 0; i < fill; ++i) {
			func(data[i]);
		}
	}

	T*& operator[](unsigned n) { return data[n]; }
	T* const& operator[](unsigned n) const { return data[n]; }
	T*& at(unsigned n) { if (n >= fill) throw std::out_of_range("dynptrvector at"); return data[n]; }
	T* const& at(unsigned n) const { if (n >= fill) throw std::out_of_range("dynptrvector const at"); return data[n]; }
};


#endif
