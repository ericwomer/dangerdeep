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

// Network manager - manages multiplayer networking (currently unused/legacy)
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <vector>

class network_connection;

/// Network game types
enum class NetworkType {
    SinglePlayer = 0, ///< Local single player game
    Server = 1,       ///< Multiplayer server
    Client = 2        ///< Multiplayer client
};

/// Manages multiplayer network connections and message passing (legacy code, currently unused)
class network_manager {
  private:
    NetworkType type;                                     ///< Type of network game
    network_connection *server_connection;                ///< Connection to server (if client)
    std::vector<network_connection *> client_connections; ///< Connections to clients (if server)

  public:
    network_manager();
    ~network_manager();

    // Non-copyable
    network_manager(const network_manager &) = delete;
    network_manager &operator=(const network_manager &) = delete;

    /// Get network type
    NetworkType get_type() const { return type; }

    /// Check if this is a multiplayer game
    bool is_multiplayer() const { return type != NetworkType::SinglePlayer; }

    /// Check if this is the server
    bool is_server() const { return type == NetworkType::Server && server_connection == nullptr; }

    /// Check if this is a client
    bool is_client() const { return type == NetworkType::Client && server_connection != nullptr; }

    /// Get server connection (nullptr if this is server)
    network_connection *get_server_connection() const { return server_connection; }

    /// Get client connections (empty if this is client)
    const std::vector<network_connection *> &get_client_connections() const { return client_connections; }

    // Note: receive_commands() and send() methods are currently commented out in original code
    // They would handle command distribution and execution in multiplayer games
};

#endif
