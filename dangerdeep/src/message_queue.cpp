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

// multithreading primitives: messaging, message queue
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "message_queue.h"
#include "error.h"
#include "system.h"
#include "thread.h"
#include "log.h"


void message::evaluate() const
{
	result = false;
	try {
		eval();
		result = true;
	}
	catch (std::exception& e) {
		// avoid to spam the log. define when needed.
		// log_debug("msg eval failed: " << e.what());
	}
}



message_queue::message_queue()
	: msginqueue(false), abortwait(false)
{
}



message_queue::~message_queue()
{
	// clean up queues, report all pending messages as failed, then wait
	// until ackqueue is empty... it should not happen though...
	bool ackqueueempty = true;
	{
		mutex_locker ml(mymutex);
		for (std::list<message*>::iterator it = myqueue.begin(); it != myqueue.end(); ++it) {
			if ((*it)->needsanswer) {
				ackqueue.push_back(*it);
			} else {
				delete *it;
			}
		}
		ackcondvar.signal();
		ackqueueempty = ackqueue.empty();
	}
	while (!ackqueueempty) {
		thread::sleep(10);
		mutex_locker ml(mymutex);
		ackqueueempty = ackqueue.empty();
	}
}



bool message_queue::send(message::ptr msg, bool waitforanswer)
{
	msg->needsanswer = waitforanswer;
	msg->result = false;
	message* msg_addr = msg.get();
	mutex_locker oml(mymutex);
	bool e = myqueue.empty();
	myqueue.push_back(msg.release());
	msginqueue = true;
	if (e) {
		emptycondvar.signal();
	}
	if (waitforanswer) {
		while (true) {
			ackcondvar.wait(mymutex);
			// check if this message has been acknowledged
			for (std::list<message*>::iterator it = ackqueue.begin(); it != ackqueue.end(); ) {
				if (*it == msg_addr) {
					// found it, return result, delete and unqueue message
					bool result = (*it)->result;
					delete *it;
					ackqueue.erase(it);
					return result;
				}
			}
		}
	}
	return true;
}



void message_queue::wakeup_receiver()
{
	// set a special flag to avoid another thread to enter the wait() command
	// if this signal comes while the other thread tests wether to enter wait state
	mutex_locker oml(mymutex);
	abortwait = true;
	emptycondvar.signal();
}



std::list<message*> message_queue::receive(bool wait)
{
	std::list<message*> result;
	mutex_locker oml(mymutex);
	if (myqueue.empty()) {
		if (wait && !abortwait) {
			emptycondvar.wait(mymutex);
		} else {
			// no need to wait, so clear abort signal
			abortwait = false;
			msginqueue = false;
			return result;
		}
	}

	abortwait = false;

	// if we woke up and queue is still empty, we received a wakeup signal
	// so result either is empty or contains the full queue after this line
	myqueue.swap(result);

	// any way, there are now no messages in the queue. return messages or empty list.
	msginqueue = false;
	return result;
}



void message_queue::acknowledge(message* msg)
{
	if (!msg)
		throw error("acknowledge without message called");
	bool needsanswer = msg->needsanswer;
	mutex_locker oml(mymutex);
	if (needsanswer) {
		ackqueue.push_back(msg);
		ackcondvar.signal();
	} else {
		delete msg;
	}
}



void message_queue::process_messages(bool wait)
{
	std::list<message*> msgs = receive(wait);
	while (!msgs.empty()) {
		msgs.front()->evaluate();
		acknowledge(msgs.front());
		msgs.pop_front();
	}
}
