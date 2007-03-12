#include "mutex.h"
#include <stdio.h>
int main()
{
	mutex m;
	printf("a\n");
	m.lock();
	printf("b\n");
	m.lock();
	printf("c\n");
	m.unlock();
	printf("d\n");
	m.unlock();
	printf("e\n");
	return 0;
}
