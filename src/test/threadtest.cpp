/* compile:
g++ -Wall test4.cpp error.cpp thread.cpp mutex.cpp condvar.cpp -I/usr/include/SDL -lSDL
*/

#include "thread.h"
#include "mutex.h"
#include "condvar.h"
#include <iostream>
using namespace std;

mutex* m = 0;
condvar* c = 0;

class A : public thread
{
public:
	A() { cout << "A: c'tor " << this << "\n"; }
	~A() { cout << "A: D'tor " << this << "\n"; }
	void loop() {
		cout << "A: lock m?\n";
		m->lock();
		sleep(500);
		cout << "A: m locked, wait\n";
		c->wait(*m);
		cout << "A: woken up, locked\n";
		sleep(500);
		m->unlock();
		cout << "A: ok!\n";
		sleep(1000);
	}
};

class B : public thread
{
public:
	B() { cout << "B: c'tor " << this << "\n"; }
	~B() { cout << "B: D'tor " << this << "\n"; }
	void loop() {
		cout << "B: lock m?\n";
		m->lock();
		sleep(400);
		cout << "B: m locked, signal\n";
		c->signal();
		cout << "B: signalled!\n";
		sleep(400);
		m->unlock();
		cout << "B: ok!\n";
		sleep(800);
	}
};

int main(int, char**)
{
	mutex m2;
	condvar c2;
	m = &m2;
	c = &c2;
	A* a = new A();
	B* b = new B();
	a->start();
	b->start();
	thread::sleep(10000);
	a->request_abort();
	b->request_abort();
	c->signal();
	a->join();
	b->join();
	cout << "cleaned up!\n";
}
