// network code
// danger from the deep (C)+(W) Thorsten Jordan. SEE LICENSE

#include "network.h"
#include "system.h"
#include <cstring>

#define PACKETSIZE 65535
#define CHANNEL 0

// do the packets have to be initialized/values filled in? no! ? fixme

vector<Uint8> network_connection::receive_packet(void)
{
	int rcvd = SDLNet_UDP_Recv(sock, in);
	if (rcvd) {
		vector<Uint8> data(in->len);
		memcpy(&data[0], in->data, in->len);
		return data;
	}
	return vector<Uint8>();
}

void network_connection::send_packet(const vector<Uint8>& data)
{
	unsigned ds = data.size();
	system::sys()->myassert(ds <= PACKETSIZE, "packet too long");
	out->len = ds;
	memcpy(out->data, &data[0], out->len);
	int error = SDLNet_UDP_Send(sock, CHANNEL, out);
	system::sys()->myassert(error != 0, "can't send UDP packet");
}

void network_connection::send_message(const string& msg)
{
	vector<Uint8> data(msg.length());
	memcpy(&data[0], &msg[0], msg.length());
	send_packet(data);
}

string network_connection::receive_message(void)
{
	vector<Uint8> data = receive_packet();
	string msg;
	msg.resize(data.size());
	memcpy(&msg[0], &data[0], data.size());
	return msg;
}

network_client::network_client(const string& hostname, unsigned port)
{
	int error;

	// resolve ip address for server
	IPaddress ipaddr;
	error = SDLNet_ResolveHost(&ipaddr, hostname.c_str(), port);
	system::sys()->myassert(error != -1, "can't resolve host");

	// open a socket on an unused port
	sock = SDLNet_UDP_Open(0);
	system::sys()->myassert(sock != 0, "can't open UDP socket");
	
	// allocate a packet to work with
	in = SDLNet_AllocPacket(PACKETSIZE);
	out = SDLNet_AllocPacket(PACKETSIZE);
	system::sys()->myassert(in != 0, "can't alloc UDP input packet");
	system::sys()->myassert(out != 0, "can't alloc UDP output packet");
	
	// bind server address to channel 0
	error = SDLNet_UDP_Bind(sock, CHANNEL, &ipaddr);
	system::sys()->myassert(error != -1, "can't bind UDP socket");
}

network_server::network_server(unsigned port)
{
	// open a socket on the specified port
	sock = SDLNet_UDP_Open(port);
	system::sys()->myassert(sock != 0, "can't open UDP socket");
	SDLNet_UDP_Unbind(sock, CHANNEL);

	// allocate a packet to work with
	in = SDLNet_AllocPacket(PACKETSIZE);
	out = SDLNet_AllocPacket(PACKETSIZE);
	system::sys()->myassert(in != 0, "can't alloc UDP input packet");
	system::sys()->myassert(out != 0, "can't alloc UDP output packet");
}

network_connection::~network_connection()
{	
	// close the socket
	SDLNet_UDP_Close(sock);
	
	// free packets
	SDLNet_FreePacket(out);
	SDLNet_FreePacket(in);
}
