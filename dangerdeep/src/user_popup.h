// Base interface for user screen popups.
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef USER_POPUP_H
#define USER_POPUP_H

#include <list>
#include <SDL.h>

class user_popup
{
private:
	// no empty construction, no copy
	user_popup();
	user_popup(user_popup& );
	user_popup& operator= (const user_popup& );

protected:
	// display position.
	unsigned x, y;

	// the display needs to know its parent (user_interface) to access common data
	class user_interface& ui;

	user_popup(class user_interface& ui_) : ui(ui_) {}

public:
	// needed for correct destruction of heirs.
	virtual ~user_popup() {}
	// very basic. Just draw display and handle input.
	virtual void display(class game& gm) const = 0;
	// returns true if event was used
	virtual bool process_input(class game& gm, const SDL_Event& event) = 0;
	virtual void process_input(class game& gm, std::list<SDL_Event>& events)
	{
		std::list<SDL_Event>::iterator it = events.begin();
		while (it != events.end()) {
			std::list<SDL_Event>::iterator it2 = it;
			++it;
			bool used = process_input(gm, *it2);
			if (used) events.erase(it2);
		}
	}
};

#endif /* USER_POPUP_H */
