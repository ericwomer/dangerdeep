// ptrvector
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef PTRVECTOR_H
#define PTRVECTOR_H

#include <vector>
#include <algorithm>

template <class T>
class ptrvector
{
 protected:
	std::vector<T*> data;
	mutable unsigned index_searchptr;

	void data_delete(unsigned a, unsigned b) {
		for (unsigned i = a; i < b; ++i) {
			delete data[i];
		}
	}

 private:
	ptrvector(const ptrvector& );
	ptrvector& operator= (const ptrvector& );
 public:
	ptrvector(unsigned size = 0) : data(size), index_searchptr(0) {}
	~ptrvector() { clear(); }

	unsigned size() const { return data.size(); }
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
		index_searchptr = data.size();
	}

	int find_empty_index() const {
		if (index_searchptr >= data.size())
			index_searchptr = 0;
		// search from index ptr to end
		for (unsigned j = index_searchptr; j < data.size(); ++j) {
			if (data[j] == 0) {
				index_searchptr = j + 1;
				return int(j);
			}
		}
		// search from begin to index ptr
		for (unsigned j = 0; j < index_searchptr; ++j) {
			if (data[j] == 0) {
				index_searchptr = j + 1;
				return int(j);
			}
		}
		return -1;
	}

	template <typename U>
	U for_each(const U& func) { return std::for_each(data.begin(), data.end(), func); }
	template <typename U>
	U for_each(const U& func) const { return std::for_each(data.begin(), data.end(), func); }
	void pack() {
		unsigned filled = 0;
		for (unsigned i = 0; i < data.size(); ++i)
			if (data[i])
				++filled;
		if (filled == data.size()) return;
		std::vector<T*> data2(filled);
		for (unsigned i = 0, j = 0; i < data.size(); ++i)
			if (data[i])
				data2[j++] = data[i];
		data.swap(data2);
	}
	T*& operator[](unsigned n) { return data[n]; }
	T* const& operator[](unsigned n) const { return data[n]; }
	T*& at(unsigned n) { return data.at(n); }
	T* const& at(unsigned n) const { return data.at(n); }
};


#endif
