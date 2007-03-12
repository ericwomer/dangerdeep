/* compile:
g++ -Wall test4.cpp error.cpp thread.cpp mutex.cpp condvar.cpp message_queue.cpp -I/usr/include/SDL -lSDL
*/

#include "thread.h"
#include "mutex.h"
#include "condvar.h"
#include "message_queue.h"
#include "error.h"
#include <iostream>
using namespace std;

struct msg_A : public message
{
	void eval() const { cout << "msg A eval\n"; }
};

struct msg_B : public message
{
	void eval() const { cout << "msg B eval\n"; throw error("no way!"); }
};

struct msg_C : public message
{
	void eval() const { cout << "msg C eval\n"; }
};

class A : public thread
{
public:
	message_queue mq;
	A() { cout << "A: c'tor " << this << "\n"; }
	~A() { cout << "A: D'tor " << this << "\n"; }
	void loop() {
		cout << "A: waiting for messages\n";
		mq.process_messages();
	}
};

int main(int, char**)
{
	cout << "Here we go.\n";
	A* a = new A();
	a->start();
	thread::sleep(2000);
	cout << "send msg_A = " << a->mq.send(message::ptr(new msg_A())) << "\n";
	cout << "send msg_B = " << a->mq.send(message::ptr(new msg_B())) << "\n";
	cout << "send msg_C = " << a->mq.send(message::ptr(new msg_C()), false) << "\n";
	thread::sleep(2000);
	a->request_abort();
	a->mq.wakeup_receiver();
	a->join();
	cout << "cleaned up!\n";
}
