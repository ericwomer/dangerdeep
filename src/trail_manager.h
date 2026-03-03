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

// Trail manager - manages position trail recording timing
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef TRAIL_MANAGER_H
#define TRAIL_MANAGER_H

/// Manages timing for trail position recording
class trail_manager {
  private:
    static constexpr double TRAIL_TIME = 1.0; // Time between records (seconds)
    double last_trail_time;

  public:
    trail_manager() : last_trail_time(0.0) {}
    explicit trail_manager(double current_time) : last_trail_time(current_time - TRAIL_TIME) {}
    ~trail_manager() = default;

    // Non-copyable
    trail_manager(const trail_manager &) = delete;
    trail_manager &operator=(const trail_manager &) = delete;

    /// Check if it's time to record a new trail position
    bool should_record(double current_time) const {
        return current_time >= last_trail_time + TRAIL_TIME;
    }

    /// Mark that a trail was recorded at this time
    void record_trail(double current_time) {
        last_trail_time = current_time;
    }

    /// Get last trail record time
    double get_last_trail_time() const { return last_trail_time; }

    /// Set last trail record time (for loading from save)
    void set_last_trail_time(double time) { last_trail_time = time; }

    /// Get trail recording interval
    static constexpr double get_trail_interval() { return TRAIL_TIME; }
};

#endif
