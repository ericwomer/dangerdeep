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

#include "event_manager.h"
#include "event.h"
#include "user_interface.h"

void event_manager::add_event(std::unique_ptr<event> e) {
    if (e) {
        events.push_back(std::move(e));
    }
}

void event_manager::add_event(event *e) {
    if (e) {
        events.push_back(std::unique_ptr<event>(e));
    }
}

void event_manager::evaluate_events(user_interface &ui) {
    for (auto &evt : events) {
        evt->evaluate(ui);
    }
}

void event_manager::clear_events() {
    events.clear();
}
