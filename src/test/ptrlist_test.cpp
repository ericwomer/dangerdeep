#include "ptrlist.h"
#include <stdio.h>

struct A
{
	int x;
	A(int y=0): x(y) { printf("c't %p\n", this); }
	~A() { printf("D't %p\n", this); }
	void dodo() const { printf("A=%p x=%i\n", this, x); }
};


int main(int, char**)
{
	{
	ptrlist<A> foo;
	foo.push_back(new A(1));
	foo.push_back(new A(3));
	foo.push_back(new A(5));
	foo.push_back(new A(7));
	for (ptrlist<A>::const_iterator it = foo.begin(); it != foo.end(); ++it) {
		it->dodo();
	}
	}
	printf("ok\n");
	return -1;
}
