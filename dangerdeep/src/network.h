// network code
// danger from the deep (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef NETWORK_H
#define NETWORK_H

#include "SDL.h"
#include "SDL_net.h"
#include <vector>
using namespace std;

class network_connection
{
protected:
	UDPsocket sock;
	UDPpacket *in, *out;
	
	network_connection() {};
	
public:
	vector<Uint8> receive_packet(void);
	void send_packet(const vector<Uint8>& data);
	void send_message(const string& msg);
	string receive_message(void);
	virtual ~network_connection();
};

class network_client : public network_connection
{
public:
	network_client(const string& hostname, unsigned port);
	virtual ~network_client() {}
};

class network_server : public network_connection
{
public:
	network_server(unsigned port);
	virtual ~network_server() {}
};

#endif
