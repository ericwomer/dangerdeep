// network code
// danger from the deep (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef NETWORK_H
#define NETWORK_H

#include "SDL.h"
#include "SDL_net.h"
#include <vector>
#include <string>
using namespace std;

class network_connection
{
protected:
	UDPsocket sock;
	UDPpacket *in, *out;
	
	void init(Uint16 local_port);
	
	network_connection(const network_connection& );
	network_connection& operator= (const network_connection& );
	
public:
	void send_packet(const vector<Uint8>& data);
	vector<Uint8> receive_packet(IPaddress* ip = 0);
	void send_message(const string& msg);
	string receive_message(IPaddress* ip = 0);
	
	// create a connection on a local port (0 means any free port)
	network_connection(Uint16 local_port = 0);
	// create a connection and connect to a server
	network_connection(IPaddress serverip);
	// create a connection and connect to a server
	network_connection(const string& servername, Uint16 server_port);
	void bind(IPaddress ip);
	void bind(const string& servername, Uint16 server_port);
	void unbind(void);
	~network_connection();
	static string ip2string(IPaddress ip);
};

inline bool operator== (const IPaddress& a, const IPaddress& b)
{
	return a.host == b.host && a.port == b.port;
}

#endif
