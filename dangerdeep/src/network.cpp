// network code
// danger from the deep (C)+(W) Thorsten Jordan. SEE LICENSE

#include "network.h"
#include "system.h"
#include <cstring>
#include <sstream>

#define PACKETSIZE 65535
#define CHANNEL 0

void network_connection::init(Uint16 local_port)
{
	// open a socket on a port
	sock = SDLNet_UDP_Open(local_port);
	system::sys().myassert(sock != 0, "can't open UDP socket");

	// allocate packets to work with
	in = SDLNet_AllocPacket(PACKETSIZE);
	out = SDLNet_AllocPacket(PACKETSIZE);
	system::sys().myassert(in != 0, "can't alloc UDP input packet");
	system::sys().myassert(out != 0, "can't alloc UDP output packet");
}

void network_connection::send_packet(const vector<Uint8>& data)
{
	unsigned ds = data.size();
	system::sys().myassert(ds <= PACKETSIZE, "packet too long");
	out->len = ds;
	memcpy(out->data, &data[0], out->len);
	int error = SDLNet_UDP_Send(sock, CHANNEL, out);
	system::sys().myassert(error != 0, "can't send UDP packet");
}

vector<Uint8> network_connection::receive_packet(IPaddress* ip)
{
	int rcvd = SDLNet_UDP_Recv(sock, in);
	if (rcvd) {
		vector<Uint8> data(in->len);
		memcpy(&data[0], in->data, in->len);
		if (ip)
			*ip = in->address;
		return data;
	}
	if (ip) {
		ip->host = 0;
		ip->port = 0;
	}
	return vector<Uint8>();
}

void network_connection::send_message(const string& msg)
{
	vector<Uint8> data(msg.length());
	memcpy(&data[0], &msg[0], msg.length());
	send_packet(data);
}

string network_connection::receive_message(IPaddress* ip)
{
	vector<Uint8> data = receive_packet(ip);
	string msg;
	msg.resize(data.size());
	memcpy(&msg[0], &data[0], data.size());
	return msg;
}

network_connection::network_connection(Uint16 local_port)
{
	init(local_port);
}

network_connection::network_connection(IPaddress serverip)
{
	init(0);
	bind(serverip);
}

network_connection::network_connection(const string& servername, Uint16 server_port)
{
	init(0);
	bind(servername, server_port);
}

void network_connection::bind(IPaddress ip)
{
	// bind server address to channel
	int error = SDLNet_UDP_Bind(sock, CHANNEL, &ip);
	system::sys().myassert(error != -1, "can't bind UDP socket");
}

void network_connection::bind(const string& servername, Uint16 server_port)
{
	IPaddress ip;
	SDLNet_ResolveHost(&ip, servername.c_str(), server_port);
	bind(ip);
}

void network_connection::unbind(void)
{
	SDLNet_UDP_Unbind(sock, CHANNEL);
}
	
network_connection::~network_connection()
{	
	// close the socket
	SDLNet_UDP_Close(sock);
	
	// free packets
	SDLNet_FreePacket(out);
	SDLNet_FreePacket(in);
}

string network_connection::ip2string(IPaddress ip)
{
	Uint32 ipnum = SDL_SwapBE32(ip.host);
	Uint16 port = SDL_SwapBE16(ip.port);
	ostringstream os;
	os << ((ipnum >> 24) & 0xff) << "." <<
		((ipnum >> 16) & 0xff) << "." <<
		((ipnum >> 8) & 0xff) << "." <<
		((ipnum     ) & 0xff) << ":" <<
		port;
	return os.str();
}
