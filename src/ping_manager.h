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

// Ping manager - manages active sonar pings
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef PING_MANAGER_H
#define PING_MANAGER_H

#include "angle.h"
#include "vector2.h"
#include <list>

class xml_elem;

/// Represents an active sonar ping
struct ping {
    vector2 pos;      ///< Position where ping was emitted
    angle dir;        ///< Direction of ping
    double time;      ///< Game time when ping was emitted
    double range;     ///< Maximum range of ping
    angle ping_angle; ///< Angular spread of ping cone

    ping(const vector2 &p, angle d, double t, double range_val, const angle &ping_angle_val)
        : pos(p), dir(d), time(t), range(range_val), ping_angle(ping_angle_val) {}
    ~ping() {}
    ping(const xml_elem &parent);
    void save(xml_elem &parent) const;
};

/// Manages active sonar pings in the game world
class ping_manager {
  private:
    std::list<ping> pings;
    double ping_remain_time; ///< How long pings remain visible (seconds)

  public:
    ping_manager();
    ~ping_manager() = default;

    // Non-copyable
    ping_manager(const ping_manager &) = delete;
    ping_manager &operator=(const ping_manager &) = delete;

    /// Add a new ping
    void add_ping(const vector2 &pos, angle dir, double time, double range, const angle &ping_angle);

    /// Remove pings older than ping_remain_time
    void update(double current_time);

    /// Get all active pings
    const std::list<ping> &get_pings() const { return pings; }

    /// Clear all pings
    void clear() { pings.clear(); }

    /// Check if there are any active pings
    bool has_pings() const { return !pings.empty(); }

    /// Get number of active pings
    size_t ping_count() const { return pings.size(); }

    /// Load pings from XML
    void load(const xml_elem &parent);

    /// Save pings to XML
    void save(xml_elem &parent) const;
};

#endif
