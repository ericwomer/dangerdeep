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

// network code
// danger from the deep (C)+(W) Thorsten Jordan. SEE LICENSE

#include "network.h"
#include "system.h"
#include <cstring>
#include <sstream>
using std::vector;
using std::string;
using std::ostringstream;

#define PACKETSIZE 65535
#define CHANNEL 0

void network_connection::init(Uint16 local_port)
{
	// open a socket on a port
	sock = SDLNet_UDP_Open(local_port);
	//sys().myassert(sock != 0, "can't open UDP socket");

	// allocate packets to work with
	in = SDLNet_AllocPacket(PACKETSIZE);
	out = SDLNet_AllocPacket(PACKETSIZE);
	//sys().myassert(in != 0, "can't alloc UDP input packet");
	//sys().myassert(out != 0, "can't alloc UDP output packet");
}

void network_connection::send_packet(const vector<Uint8>& data)
{
	unsigned ds = data.size();
	//sys().myassert(ds <= PACKETSIZE, "packet too long");
	out->len = ds;
	memcpy(out->data, &data[0], out->len);
	int error = SDLNet_UDP_Send(sock, CHANNEL, out);
	//sys().myassert(error != 0, "can't send UDP packet");
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
	//sys().myassert(error != -1, "can't bind UDP socket");
}

void network_connection::bind(const string& servername, Uint16 server_port)
{
	IPaddress ip;
	SDLNet_ResolveHost(&ip, servername.c_str(), server_port);
	bind(ip);
}

void network_connection::unbind()
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
