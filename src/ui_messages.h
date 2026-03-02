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

// ui messages - manages timed messages that fade out
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef UI_MESSAGES_H
#define UI_MESSAGES_H

#include <list>
#include <string>
#include <utility>

class font;
class color;

///\brief Manages a queue of timed messages that fade out over time
class ui_message_queue {
  public:
    ui_message_queue();
    ~ui_message_queue();

    /// Add a message to the queue (with current time)
    void add_message(const std::string &message, double current_time);

    /// Draw all visible messages (that haven't expired yet)
    /// @param current_time - current game time
    /// @param y_position - y coordinate where to start drawing (top message)
    /// @param fnt - font to use for rendering
    void draw(double current_time, int y_position, const font *fnt) const;

    /// Remove expired messages
    void cleanup(double current_time);

    /// Get number of messages in queue
    size_t size() const { return messages.size(); }

    /// Clear all messages
    void clear() { messages.clear(); }

  protected:
    /// List of messages with their creation time
    std::list<std::pair<double, std::string>> messages;

    /// Configuration constants
    static constexpr double message_vanish_time = 10.0; // seconds until message disappears
    static constexpr double message_fadeout_time = 2.0; // seconds for fade out animation
    static constexpr size_t max_messages = 6;           // maximum number of messages to keep

  private:
    ui_message_queue(const ui_message_queue &) = delete;
    ui_message_queue &operator=(const ui_message_queue &) = delete;
};

#endif // UI_MESSAGES_H
