// user display: submarine's UZO
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "system.h"
#include "image.h"
#include "texture.h"
#include "game.h"
#include "submarine.h"
#include "sub_uzo_display.h"
#include "user_interface.h"
#include "keys.h"
#include "cfg.h"



void sub_uzo_display::pre_display(game& gm) const
{
	glClear(GL_DEPTH_BUFFER_BIT);
}



freeview_display::projection_data sub_uzo_display::get_projection_data(class game& gm) const
{
	projection_data pd;
	pd.x = 0;
	pd.y = sys().get_res_y() - sys().get_res_x()/2;
	pd.w = sys().get_res_x();
	pd.h = sys().get_res_x()/2;
	// with normal fov of 70 degrees, this is 1.5 / 6.0 magnification
	pd.fov_x = zoomed ? 13.31 : 50.05;	//fixme: historic values?
	pd.near_z = 1.0;
	pd.far_z = gm.get_max_view_distance();
	return pd;
}



void sub_uzo_display::post_display(game& gm) const
{
	sys().prepare_2d_drawing();
	uzotex->draw(0, 0, 512, 512);
	uzotex->draw_hm(512, 0, 512, 512);
	glBindTexture(GL_TEXTURE_2D, 0);
	glColor3ub(0, 0, 0);
	sys().draw_rectangle(0, 512, 1024, 256);
	ui.draw_infopanel();
	sys().unprepare_2d_drawing();
}



sub_uzo_display::sub_uzo_display(user_interface& ui_) : freeview_display(ui_), zoomed(false)
{
	uzotex = new texture(get_texture_dir() + "uzo.png");

	pos = vector3(0, 0, 6);//fixme, depends on sub
	aboard = true;
	withunderwaterweapons = false;
	drawbridge = false;
}



sub_uzo_display::~sub_uzo_display()
{
	delete uzotex;
}



void sub_uzo_display::process_input(class game& gm, const SDL_Event& event)
{
	submarine* sub = dynamic_cast<submarine*>(gm.get_player());
	if (sub->is_submerged()) {
	}
	switch (event.type) {
	case SDL_KEYDOWN:
		if (cfg::instance().getkey(KEY_TOGGLE_ZOOM_OF_VIEW).equal(event.key.keysym)) {
			zoomed = !zoomed;
		} else {
			switch(event.key.keysym.sym) {
				// filter away keys NP_1...NP_9 to avoid moving viewer like in freeview mode
			case SDLK_KP8: return;
			case SDLK_KP2: return;
			case SDLK_KP4: return;
			case SDLK_KP6: return;
			case SDLK_KP1: return;
			case SDLK_KP3: return;
			default: break;
			}
		}
	default: break;
	}
	freeview_display::process_input(gm, event);
}
