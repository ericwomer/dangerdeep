// Base interface for user screens.
// subsim (C)+(W) Markus Petermann and Thorsten Jordan. SEE LICENSE

#ifndef USER_DISPLAY_H
#define USER_DISPLAY_H

#include <list>
using namespace std;

#include <SDL.h>

class user_display
{
public:
	// very basic. Just draw display and handle input.
	virtual void display(class game& gm) const = 0;
	virtual void process_input(const SDL_Event& event) = 0;
	virtual void process_input(const list<SDL_Event>& events)
	{
		for (list<SDL_Event>::const_iterator it = events.begin();
		     it != events.end(); ++it)
			process_input(*it);
	}
};

#endif /* USER_DISPLAY_H */
