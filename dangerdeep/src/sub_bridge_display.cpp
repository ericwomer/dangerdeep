// user display: submarine's bridge
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "system.h"
#include "image.h"
#include "texture.h"
#include "game.h"
#include "sub_bridge_display.h"
#include "user_interface.h"
#include "keys.h"
#include "cfg.h"



void sub_bridge_display::pre_display(game& gm) const
{
	glClear(GL_DEPTH_BUFFER_BIT);
}



freeview_display::projection_data sub_bridge_display::get_projection_data(game& gm) const
{
	projection_data pd = freeview_display::get_projection_data(gm);
	if (glasses_in_use) {
		pd.x = 0;
		pd.y = 0;
		pd.w = sys().get_res_x();
		pd.h = sys().get_res_x();
		pd.fov_x = 20.0;
	}
	return pd;
}



void sub_bridge_display::post_display(game& gm) const
{
	sys().prepare_2d_drawing();
	if (glasses_in_use) {
		glasses_tex->draw(0, 0, 1024, 768);
	}
	sys().unprepare_2d_drawing();
}



sub_bridge_display::sub_bridge_display(user_interface& ui_) : freeview_display(ui_), glasses_in_use(false)
{
	pos = vector3(0, 0, 6);//fixme, depends on sub
	aboard = true;
	withunderwaterweapons = false;
	drawbridge = true;
	glasses_tex.reset(new texture(get_texture_dir() + "glasses.png", texture::LINEAR, texture::CLAMP_TO_EDGE));
}



void sub_bridge_display::process_input(class game& gm, const SDL_Event& event)
{
	switch (event.type) {
	case SDL_KEYDOWN:
		if (cfg::instance().getkey(KEY_TOGGLE_ZOOM_OF_VIEW).equal(event.key.keysym)) {
			glasses_in_use = !glasses_in_use;
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
