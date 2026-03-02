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

// ui messages implementation
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "ui_messages.h"
#include "color.h"
#include "font.h"
#include <algorithm>

ui_message_queue::ui_message_queue() {
}

ui_message_queue::~ui_message_queue() {
}

void ui_message_queue::add_message(const std::string &message, double current_time) {
    messages.push_back(std::make_pair(current_time, message));

    // Remove old messages if we exceed the maximum
    while (messages.size() > max_messages) {
        messages.pop_front();
    }

    // Also cleanup expired messages
    cleanup(current_time);
}

void ui_message_queue::draw(double current_time, int y_position, const font *fnt) const {
    if (!fnt) {
        return;
    }

    double vanish_time = current_time - message_vanish_time;
    int y = y_position - fnt->get_height();

    // Draw messages in reverse order (newest at bottom)
    for (std::list<std::pair<double, std::string>>::const_reverse_iterator it = messages.rbegin();
         it != messages.rend(); ++it) {
        // Skip messages that have completely vanished
        if (it->first < vanish_time) {
            break;
        }

        // Calculate alpha for fade-out effect
        double alpha = std::min(1.0, (it->first - vanish_time) / message_fadeout_time);
        color col(colorf(1, 1, 1, alpha));

        // Draw the message
        fnt->print(0, y, it->second, col, true);

        y -= fnt->get_height();
    }
}

void ui_message_queue::cleanup(double current_time) {
    double vanish_time = current_time - message_vanish_time;

    // Remove all expired messages
    for (std::list<std::pair<double, std::string>>::iterator it = messages.begin();
         it != messages.end();) {
        if (it->first < vanish_time) {
            it = messages.erase(it);
        } else {
            ++it;
        }
    }
}
