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

// Scoring manager - manages sunken ships records
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef SCORING_MANAGER_H
#define SCORING_MANAGER_H

#include "date.h"
#include <list>
#include <string>

class xml_elem;

/// Record of a sunken ship
struct sink_record {
    date dat;
    std::string descr;        // fixme: store type, use a static ship function to retrieve a matching description, via specfilename!
    std::string mdlname;      // model file name string
    std::string specfilename; // spec file name (base model name)
    std::string layoutname;   // model skin
    unsigned tons;

    sink_record(date d, const std::string &s, const std::string &m,
                const std::string &sn, const std::string &ln, unsigned t)
        : dat(d), descr(s), mdlname(m), specfilename(sn), layoutname(ln), tons(t) {}
    sink_record(const xml_elem &parent);
    void save(xml_elem &parent) const;
};

/// Manages scoring and sunken ships records
class scoring_manager {
  private:
    std::list<sink_record> sunken_ships;

  public:
    scoring_manager() = default;
    ~scoring_manager() = default;

    // Non-copyable
    scoring_manager(const scoring_manager &) = delete;
    scoring_manager &operator=(const scoring_manager &) = delete;

    /// Record a sunken ship
    void record_sunk_ship(date d, const std::string &descr, const std::string &mdlname,
                          const std::string &specfilename, const std::string &layoutname, unsigned tons);

    /// Get all sunken ships records
    const std::list<sink_record> &get_sunken_ships() const { return sunken_ships; }

    /// Clear all records
    void clear() { sunken_ships.clear(); }

    /// Check if there are any records
    bool has_records() const { return !sunken_ships.empty(); }

    /// Get number of sunken ships
    size_t sunk_count() const { return sunken_ships.size(); }

    /// Calculate total tonnage sunk
    unsigned total_tonnage() const;

    /// Load from XML
    void load(const xml_elem &parent);

    /// Save to XML
    void save(xml_elem &parent) const;
};

#endif
