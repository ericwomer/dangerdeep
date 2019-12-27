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

#ifndef NETWORK_H
#define NETWORK_H

#if 0

#include "SDL.h"
#include "SDL_net.h"
#include <vector>
#include <string>

class network_connection
{
protected:
	UDPsocket sock;
	UDPpacket *in, *out;
	
	void init(Uint16 local_port);
	
	network_connection(const network_connection& );
	network_connection& operator= (const network_connection& );
	
public:
	void send_packet(const std::vector<Uint8>& data);
	std::vector<Uint8> receive_packet(IPaddress* ip = 0);
	void send_message(const std::string& msg);
	std::string receive_message(IPaddress* ip = 0);
	
	// create a connection on a local port (0 means any free port)
	network_connection(Uint16 local_port = 0);
	// create a connection and connect to a server
	network_connection(IPaddress serverip);
	// create a connection and connect to a server
	network_connection(const std::string& servername, Uint16 server_port);
	void bind(IPaddress ip);
	void bind(const std::string& servername, Uint16 server_port);
	void unbind();
	~network_connection();
	static std::string ip2string(IPaddress ip);
};

inline bool operator== (const IPaddress& a, const IPaddress& b)
{
	return a.host == b.host && a.port == b.port;
}

#endif
#endif
