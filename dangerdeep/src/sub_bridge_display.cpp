// user display: submarine's bridge
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "system.h"
#include "image.h"
#include "texture.h"
#include "game.h"
#include "sub_bridge_display.h"
#include "user_interface.h"



void sub_bridge_display::pre_display(game& gm) const
{
	glClear(GL_DEPTH_BUFFER_BIT);
}



freeview_display::projection_data sub_bridge_display::get_projection_data(game& gm) const
{
	projection_data pd = freeview_display::get_projection_data(gm);
	if (glasses_in_use) {
		pd.x = 0;
		pd.y = system::sys().get_res_y()-system::sys().get_res_x()/2;
		pd.w = system::sys().get_res_x();
		pd.h = system::sys().get_res_x()/2;
		pd.fov_x = 20.0;
	}
	return pd;
}



void sub_bridge_display::post_display(game& gm) const
{
	system::sys().prepare_2d_drawing();
	if (glasses_in_use) {
		glasses_tex->draw(0, 0, 512, 512);
		glasses_tex->draw_hm(512, 0, 512, 512);
		glBindTexture(GL_TEXTURE_2D, 0);
		glColor3ub(0, 0, 0);
		system::sys().draw_rectangle(0, 512, 1024, 256);
	}
	ui.draw_infopanel(gm);
	system::sys().unprepare_2d_drawing();
}



sub_bridge_display::sub_bridge_display(user_interface& ui_) : freeview_display(ui_), glasses_in_use(false)
{
	pos = vector3(0, 0, 6);//fixme, depends on sub
	aboard = true;
	withunderwaterweapons = false;
	drawbridge = true;
	glasses_tex = new texture(get_texture_dir() + "glasses.png", GL_LINEAR, GL_CLAMP_TO_EDGE);
}



sub_bridge_display::~sub_bridge_display()
{
	delete glasses_tex;
}



void sub_bridge_display::process_input(class game& gm, const SDL_Event& event)
{
	switch (event.type) {
	case SDL_KEYDOWN:
		switch(event.key.keysym.sym) {
		case SDLK_y: glasses_in_use = !glasses_in_use; return;
		// filter away keys NP_1...NP_9 to avoid moving viewer like in freeview mode
		case SDLK_KP8: return;
		case SDLK_KP2: return;
		case SDLK_KP4: return;
		case SDLK_KP6: return;
		case SDLK_KP1: return;
		case SDLK_KP3: return;
		default: break;
		}
	default: break;
	}
	freeview_display::process_input(gm, event);
}
