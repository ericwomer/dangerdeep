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

// Time freezer - manages game time pausing
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef TIME_FREEZER_H
#define TIME_FREEZER_H

/// Manages game time freezing (pausing) for UI operations
class time_freezer {
  private:
    unsigned freezetime;       ///< Total accumulated frozen time (milliseconds)
    unsigned freezetime_start; ///< When current freeze started (0 if not frozen)

  public:
    time_freezer();
    ~time_freezer() = default;

    // Non-copyable
    time_freezer(const time_freezer &) = delete;
    time_freezer &operator=(const time_freezer &) = delete;

    /// Start freezing time (pauses game simulation)
    void freeze();

    /// Stop freezing time (resumes game simulation)
    void unfreeze();

    /// Check if currently frozen
    bool is_frozen() const { return freezetime_start > 0; }

    /// Get total accumulated frozen time (milliseconds)
    unsigned get_freezetime() const { return freezetime; }

    /// Get when current freeze started (0 if not frozen)
    unsigned get_freezetime_start() const { return freezetime_start; }

    /// Process and reset accumulated freeze time
    /// @return accumulated frozen time in milliseconds
    unsigned process_freezetime() {
        unsigned f = freezetime;
        freezetime = 0;
        return f;
    }

    /// Load from XML
    void load(unsigned ft, unsigned fts) {
        freezetime = ft;
        freezetime_start = fts;
    }

    /// Get state for saving
    void get_state(unsigned &ft, unsigned &fts) const {
        ft = freezetime;
        fts = freezetime_start;
    }
};

#endif
