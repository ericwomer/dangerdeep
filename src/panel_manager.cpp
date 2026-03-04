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

// panel manager implementation
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "panel_manager.h"
#include "texts.h"
#include "widget.h"
#include <SDL.h>
#include <iomanip>
#include <sstream>

panel_manager::panel_manager(int x, int y, int width, int height)
    : is_visible(true), panel_widget(nullptr) {

    // Create main panel widget
    panel_widget = std::make_unique<widget>(x, y, width, height, "", nullptr);
    panel_widget->set_background(0);

    // Panel text labels and initial values
    // Labels: Heading, Speed, Depth, Bearing, Time Scale, Time
    int label_ids[6] = {1, 4, 5, 2, 98, 61};
    const char *initial_values[6] = {"000", "000", "000", "000", "000", "00:00:00"};

    // Create label and value text widgets
    for (int i = 0; i < 6; ++i) {
        int offset_x = 8 + i * (width - 2 * 8) / 6;

        // Add label
        std::string label_text = texts::get(label_ids[i]);
        vector2i label_size = widget::get_theme()->myfont->get_size(label_text);
        panel_widget->add_child(new widget_text(offset_x, 4, 0, 0, label_text));

        // Add value text (stored for later updates)
        value_texts[i] = new widget_text(offset_x + 8 + label_size.x, 4, 0, 0, initial_values[i]);
        panel_widget->add_child(value_texts[i]);
    }
}

panel_manager::~panel_manager() = default;

void panel_manager::draw(double heading, double speed, double depth,
                         double bearing, unsigned time_scale,
                         const std::string &game_time) {
    if (!is_visible) {
        return;
    }

    // Update value texts
    std::ostringstream oss;

    // Heading (0)
    oss << std::setw(3) << std::left << static_cast<unsigned>(heading);
    value_texts[0]->set_text(oss.str());
    oss.str("");

    // Speed (1)
    oss << std::setw(3) << std::left << static_cast<unsigned>(std::fabs(std::round(speed)));
    value_texts[1]->set_text(oss.str());
    oss.str("");

    // Depth (2)
    oss << std::setw(3) << std::left << static_cast<unsigned>(std::round(std::max(0.0, depth)));
    value_texts[2]->set_text(oss.str());
    oss.str("");

    // Bearing (3)
    oss << std::setw(3) << std::left << static_cast<unsigned>(bearing);
    value_texts[3]->set_text(oss.str());
    oss.str("");

    // Time scale (4)
    oss << std::setw(3) << std::left << time_scale;
    value_texts[4]->set_text(oss.str());

    // Game time (5)
    value_texts[5]->set_text(game_time);

    // Draw the panel
    panel_widget->draw();
}

bool panel_manager::check_mouse_event(const game_event &event) {
    if (!is_visible) {
        return false;
    }
    return panel_widget->check_for_mouse_event(event);
}

int panel_manager::get_y_position() const {
    return panel_widget->get_pos().y;
}
