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

// panel manager - manages information panel display
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef PANEL_MANAGER_H
#define PANEL_MANAGER_H

#include <memory>
#include <string>
#include <SDL_events.h>

class widget;
class widget_text;

///\brief Manages the information panel display
///
/// This subsystem handles the bottom information panel that displays
/// key game stats: heading, speed, depth, bearing, time scale, and game time.
/// Extracted from user_interface to separate panel-specific responsibilities.
class panel_manager {
  public:
    /// Constructor
    /// @param x - panel X position
    /// @param y - panel Y position  
    /// @param width - panel width
    /// @param height - panel height
    panel_manager(int x, int y, int width, int height);
    
    ~panel_manager();

    /// Draw the info panel with current values
    /// @param heading - player heading in degrees
    /// @param speed - player speed in m/s
    /// @param depth - player depth in meters (positive = underwater)
    /// @param bearing - absolute bearing in degrees
    /// @param time_scale - current time compression factor
    /// @param game_time - current game time for display
    void draw(double heading, double speed, double depth, double bearing, 
              unsigned time_scale, const std::string &game_time);

    /// Check if panel should process mouse event
    /// @param event - SDL event to check
    /// @return true if event was handled by panel
    bool check_mouse_event(const SDL_Event &event);

    /// Set panel visibility
    /// @param visible - true to show panel, false to hide
    void set_visible(bool visible) { is_visible = visible; }

    /// Get panel visibility
    bool is_visible_state() const { return is_visible; }

    /// Get panel Y position (for message rendering)
    int get_y_position() const;

  protected:
    bool is_visible;
    std::unique_ptr<widget> panel_widget;
    widget_text *value_texts[6]; // Not owned, children of panel_widget

  private:
    panel_manager(const panel_manager &) = delete;
    panel_manager &operator=(const panel_manager &) = delete;
};

#endif // PANEL_MANAGER_H
