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

// Base interface for user screens.
// subsim (C)+(W) Markus Petermann and Thorsten Jordan. SEE LICENSE

#ifndef USER_DISPLAY_H
#define USER_DISPLAY_H

#include <list>
#include <SDL.h>
#include "texture.h"
#include "global_data.h"

///\defgroup displays In-game user interface screens
///\brief Base class for a single screen of the ingame user interface.
///\ingroup displays
class user_display
{
private:
	// no empty construction, no copy
	user_display();
	user_display(user_display& );
	user_display& operator= (const user_display& );

protected:
	// common functions: draw_infopanel(class game& gm)

	// the display needs to know its parent (user_interface) to access common data
	class user_interface& ui;

	user_display(class user_interface& ui_) : ui(ui_) {}

	// commonly used helper classes
	class rotat_tex {
	public:
		rotat_tex() : left(0), top(0), centerx(0), centery(0) {}
		std::auto_ptr<texture> tex;
		int left, top, centerx, centery;
		void draw(double angle) const {
			// fixme: maybe rotate around pixel center (x/y + 0.5)
			tex->draw_rot(centerx, centery, angle, centerx - left, centery - top);
		}
		void set(texture* tex_, int left_, int top_, int centerx_, int centery_) {
			tex.reset(tex_);
			left = left_;
			top = top_;
			centerx = centerx_;
			centery = centery_;
		}
		void set(const std::string& filename, int left_, int top_, int centerx_, int centery_) {
			set(new texture(get_image_dir() + filename), left_, top_, centerx_, centery_);
		}
		bool is_mouse_over(int mx, int my) const {
			return (mx >= left && my >= top
				&& mx < left + int(tex->get_width())
				&& my < top + int(tex->get_height()));
		}
	protected:
		rotat_tex(const rotat_tex& );
		rotat_tex& operator= (const rotat_tex& );
	};

	class fix_tex {
	public:
		fix_tex() : left(0), top(0) {}
		std::auto_ptr<texture> tex;
		int left, top;
		void draw() const {
			tex->draw(left, top);
		}
		void set(texture* tex_, int left_, int top_) {
			tex.reset(tex_);
			left = left_;
			top = top_;
		}
		void set(const std::string& filename, int left_, int top_) {
			set(new texture(get_image_dir() + filename), left_, top_);
		}
		bool is_mouse_over(int mx, int my) const {
			return (mx >= left && my >= top
				&& mx < left + int(tex->get_width())
				&& my < top + int(tex->get_height()));
		}
	protected:
		fix_tex(const fix_tex& );
		fix_tex& operator= (const fix_tex& );
	};

public:
	// needed for correct destruction of heirs.
	virtual ~user_display() {}
	// very basic. Just draw display and handle input.
	virtual void display(class game& gm) const = 0;
	virtual void process_input(class game& gm, const SDL_Event& event) = 0;
	virtual void process_input(class game& gm, const std::list<SDL_Event>& events)
	{
		for (std::list<SDL_Event>::const_iterator it = events.begin();
		     it != events.end(); ++it)
			process_input(gm, *it);
	}
	// mask contains one bit per popup (at most 31 popups)
	virtual unsigned get_popup_allow_mask(void) const { return 0; }
};

#endif /* USER_DISPLAY_H */
