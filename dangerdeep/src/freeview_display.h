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
	// position and direction of viewer
	angle bearing;
	angle elevation;	// -90...90 deg (look down ... up)
	vector3 pos;

	freeview_display();

	void prepare_display(void) const;
	void set_projection_matrix_and_viewport(void) const;
	void set_modelview_matrix(void) const;

	// drawing is split up
	void draw_sky(void) const;//maybe some of them need a ref to game!
	void draw_water(void) const;
	void draw_terrain(void) const;
	// choose wether the function should draw only things that are completely above
	// the water surface (for mirror images) or complete objects.
	void draw_objects(void/*bool onlyabovewater, bool withunderwaterweapons*/) const;

public:
	freeview_display(class user_interface& ui_);
	virtual ~freeview_display();

	virtual void display(class game& gm) const;
	virtual void process_input(class game& gm, const SDL_Event& event);
};

#endif
