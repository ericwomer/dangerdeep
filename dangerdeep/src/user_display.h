// Base interface for user screens.
// subsim (C)+(W) Markus Petermann. SEE LICENSE

#ifndef USER_DISPLAY_H
#define USER_DISPLAY_H

class user_display
{
public:
	virtual void display ( class game& gm ) = 0;
	virtual void check_key ( int keycode, class game& gm ) = 0;
	virtual void check_mouse ( int x, int y, int mb ) = 0;
};

#endif /* USER_DISPLAY_H */
