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

//
//  An memory allocator for STL data types with alignment. (C)+(W) 2006 Thorsten Jordan
//

#ifndef ALIGN16_ALLOCATOR_H
#define ALIGN16_ALLOCATOR_H

// We can't heir from std::allocator, since its functions are not virtual,
// and thus overloading doesn't work as needed.
// The alignment could be fetched as extra parameter, so we can align to
// any value. However C++ guarantees already 8-byte alignment, and we
// never need more than 16-byte alignment.
// Code taken and adapted from gcc 4.1.2 standard include header files.

template<typename _Tp>
class align16_allocator
{
public:
	typedef size_t     size_type;
	typedef ptrdiff_t  difference_type;
	typedef _Tp*       pointer;
	typedef const _Tp* const_pointer;
	typedef _Tp&       reference;
	typedef const _Tp& const_reference;
	typedef _Tp        value_type;

	template<typename _Tp1> struct rebind
        { typedef align16_allocator<_Tp1> other; };

	align16_allocator() throw() { }
	align16_allocator(const align16_allocator&) throw() { }
	template<typename _Tp1> align16_allocator(const align16_allocator<_Tp1>&) throw() { }
	~align16_allocator() throw() { }
	
	pointer address(reference __x) const { return &__x; }
	const_pointer address(const_reference __x) const { return &__x; }

	pointer allocate(size_type __n, const void* = 0)
	{ 
		if (__builtin_expect(__n + 16 > this->max_size(), false))
			std::__throw_bad_alloc();

		// allocate 16 bytes more to make 16 bytes alignment.
		// Uses 4 bytes before returned address to store original
		// pointer. Since default alignment is at least 4, we have
		// at least 4 bytes available before the aligned address.
		char* p = (char*)( ::operator new(__n * sizeof(_Tp) + 16) );
		char* r = (char*)((((unsigned long)p) + 16) & ~0xf);
		//printf("new orig=%p algn=%p\n", p, r);
		*(char**)(r - 4) = p;
		return (_Tp*)r;

	}

	void deallocate(pointer __p, size_type)
	{
		char* p = *(char**)(((char*)__p) - 4);
		//printf("delete algn=%p orig=%p\n", __p, p);
		::operator delete(p);
	}

	size_type max_size() const throw() 
	{ return size_t(-1) / sizeof(_Tp); }

	void construct(pointer __p, const _Tp& __val) 
	{ ::new(__p) _Tp(__val); }

	void destroy(pointer __p) { __p->~_Tp(); }
};

template<typename _Tp> inline bool operator==(const align16_allocator<_Tp>&, const align16_allocator<_Tp>&)
{ return true; }
  
template<typename _Tp> inline bool operator!=(const align16_allocator<_Tp>&, const align16_allocator<_Tp>&)
{ return false; }

#endif



#if 0
// Some test code with usage example.
#include <vector>
#include <list>
#include <stdio.h>

int main(int, char**)
{
	std::vector<char> test1(1);
	std::vector<char, align16_allocator<char> > test2(1);
	printf("addrs, %p %p\n", &test1[0], &test2[0]);
	std::list<char, align16_allocator<char> > test3;
	test3.push_back('A');
	test3.push_back('B');
	test3.push_back('C');
	return 0;
}
#endif
