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

// Event manager - manages game events and their lifecycle
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef EVENT_MANAGER_H
#define EVENT_MANAGER_H

#include <list>
#include <memory>

class event;
class user_interface;

/// Manages game events: creation, storage, evaluation, and cleanup
class event_manager {
  private:
    std::list<std::unique_ptr<event>> events;

  public:
    event_manager() = default;
    ~event_manager() = default;

    // Non-copyable
    event_manager(const event_manager &) = delete;
    event_manager &operator=(const event_manager &) = delete;

    /// Add an event to the queue (takes ownership)
    void add_event(std::unique_ptr<event> e);

    /// Add an event to the queue (convenience overload with raw pointer, takes ownership)
    void add_event(event *e);

    /// Get all events (const access)
    const std::list<std::unique_ptr<event>> &get_events() const { return events; }

    /// Evaluate all events with the given user interface
    void evaluate_events(user_interface &ui);

    /// Clear all events (called after evaluation or at end of simulation step)
    void clear_events();

    /// Check if there are pending events
    bool has_events() const { return !events.empty(); }

    /// Get number of pending events
    size_t event_count() const { return events.size(); }
};

#endif
