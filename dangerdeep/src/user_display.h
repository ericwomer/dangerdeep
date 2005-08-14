// Base interface for user screens.
// subsim (C)+(W) Markus Petermann and Thorsten Jordan. SEE LICENSE

#ifndef USER_DISPLAY_H
#define USER_DISPLAY_H

#include <list>
#include <SDL.h>
#include "texture.h"
#include "global_data.h"

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
		void set(const char* filename, int left_, int top_, int centerx_, int centery_) {
			set(new texture(get_image_dir() + filename), left_, top_, centerx_, centery_);
		}
	protected:
		rotat_tex(const rotat_tex& );
		rotat_tex& operator= (const rotat_tex& );
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
