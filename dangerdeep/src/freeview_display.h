// user display: free 3d view
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef FREEVIEW_DISPLAY_H
#define FREEVIEW_DISPLAY_H

#include "user_display.h"
#include "angle.h"
#include "vector3.h"

class freeview_display : public user_display
{
protected:
	// position and direction of viewer,maybe store in ui
	angle bearing;
	angle elevation;	// -90...90 deg (look down ... up)

	vector3 pos;		// maybe via template function, viewing position (additional if aboard)

	bool aboard;		// is player aboard?
	bool withunderwaterweapons;	// draw underwater weapons?
	bool drawbridge;	// draw bridge if aboard?

	freeview_display();

	// display() calls these functions
	void pre_display(class game& gm) const;
	void get_viewport(unsigned& x, unsigned& y, unsigned& w, unsigned& h) const;
//fixme: reflections need special viewport... depends on detail settings. mabye retrieve from ui
	matrix4 get_projection_matrix(void) const;
	matrix4 get_modelview_matrix(void) const;
	void post_display(class game& gm) const;

	// draw all sea_objects
	void draw_objects(class game& gm, const vector3& viewpos) const;

	// draw the whole view
	void draw_view(class game& gm) const;

public:
	freeview_display(class user_interface& ui_);
	virtual ~freeview_display();

	virtual void display(class game& gm) const;
	virtual void process_input(class game& gm, const SDL_Event& event);
};

#endif
