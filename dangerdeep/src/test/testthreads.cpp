// g++ -Wall testthreads.cpp thread.cpp message_queue.cpp condvar.cpp mutex.cpp error.cpp -I/usr/include/SDL -lSDL

#include "thread.h"
#include "message_queue.h"
#include <iostream>
using namespace std;

// 500 threads are too much for SDL!
const unsigned nrthr = 200;
// on a 3GHz machine ca. 2.4million messages were sent and received in 2 seconds.

class A* trs[nrthr];

class A : public thread
{
	message_queue mq;
	unsigned s, m, p;
public:
	struct msg : public message
	{
		A& a;
		msg(A& aa) : a(aa) {}
		void eval() const { a.exec_msg(); }
	};
	A* nextrcv() {
		s = (s * m + p) % nrthr;
		return trs[s];
	}

	unsigned sent, rcvd;
	A(unsigned s_, unsigned m_, unsigned p_) : s(s_), m(m_), p(p_), sent(0), rcvd(0) {}
	void loop() {
		for (unsigned i = 0; i < 3; ++i) {
			A* t = nextrcv();
			t->do_sth();
			++sent;
		}
		// normally this is first command of thread, but in this test we need to send
		// messages first
		mq.process_messages();
	}
	void do_sth() {
		mq.send(::auto_ptr<message>(new msg(*this)), false /* async, or it doesnt work */);
	}
	void exec_msg() {
		++rcvd;
	}
	void request_abort() {
		thread::request_abort();
		mq.wakeup_receiver();
	}
	  
};

unsigned rd[10] = { 343, 511, 191, 217, 841, 47, 59, 111, 975, 247 };
unsigned rdc = 0;
unsigned next_rd() { rdc = (rdc + 1) % 10; return rd[rdc]; }

int main(int, char**)
{
	for (unsigned i = 0; i < nrthr; ++i) {
		trs[i] = new A(next_rd(), next_rd(), next_rd());
	}
	cout << "prepare for STORM\n";
	for (unsigned i = 0; i < nrthr; ++i) {
		trs[i]->start();
	}
	cout << "if you can read this VERY soon after the last message, your operating system is good\n";
	// now heavy storm of messaging and processing for 2 seconds
	thread::sleep(2000);
	for (unsigned i = 0; i < nrthr; ++i) {
		trs[i]->request_abort();
	}
	// check if they are all gone
	while (true) {
		unsigned run = 0;
		for (unsigned i = 0; i < nrthr; ++i) {
			if (trs[i]->is_running())
				++run;
		}
		cout << "Still running: " << run << "\n";
		thread::sleep(100);
		if (run == 0) break;
	}
	// statistics
	unsigned st = 0, rc = 0;
	for (unsigned i = 0; i < nrthr; ++i) {
		st += trs[i]->sent;
		rc += trs[i]->rcvd;
	}
	cout << "total messages sent: " << st << " received: " << rc << "\n";
	// destroy all
	for (unsigned i = 0; i < nrthr; ++i) {
		trs[i]->join();
	}
	cout << "ok!\n";
	return 0;
}
