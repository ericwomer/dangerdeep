// user display: submarine's bridge
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "system.h"
#include "image.h"
#include "texture.h"
#include "game.h"
#include "command.h"
#include "sub_bridge_display.h"
#include "user_interface.h"



void sub_bridge_display::pre_display(game& gm) const
{
	glClear(GL_DEPTH_BUFFER_BIT);
}



void sub_bridge_display::post_display(game& gm) const
{
	system::sys().prepare_2d_drawing();
	ui.draw_infopanel(gm);
	system::sys().unprepare_2d_drawing();
}



sub_bridge_display::sub_bridge_display(user_interface& ui_) : freeview_display(ui_)
{
	pos = vector3(0, 0, 6);//fixme, depends on sub
	aboard = true;
	withunderwaterweapons = false;
	drawbridge = true;
}



sub_bridge_display::~sub_bridge_display()
{
}




#if 0 //old
void user_interface::display_glasses(class game& gm)
{
	class system& sys = system::sys();
	sea_object* player = get_player();

	unsigned res_x = system::sys().get_res_x();
	unsigned res_y = system::sys().get_res_y();

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	sys.gl_perspective_fovx (10.0, 2.0/1.0, 2.0, gm.get_max_view_distance());
	glViewport(0, res_y-res_x/2, res_x, res_x/2);
	glClear(GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glRotatef(-90, 1,0,0);	// swap y and z coordinates (camera looks at +y axis)

	vector3 viewpos = player->get_pos() + vector3(0, 0, 6);
	// no torpedoes, no DCs, no player
	draw_view(gm, viewpos, 0,res_y-res_x/2,res_x,res_x/2, player->get_heading()+bearing, 0, false/*true*/, false, false);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glViewport(0, 0, res_x, res_y);
	glMatrixMode(GL_MODELVIEW);

	sys.prepare_2d_drawing();
	glasses->draw(0, 0, 512, 512);
	glasses->draw_hm(512, 0, 512, 512);
	glBindTexture(GL_TEXTURE_2D, 0);
	color::black().set_gl_color();
	sys.draw_rectangle(0, 512, 1024, 256);
	draw_infopanel(gm);
	sys.unprepare_2d_drawing();

	// keyboard processing
	int key = sys.get_key().sym;
	while (key != 0) {
		if (!keyboard_common(key, gm)) {
			// specific keyboard processing
			switch ( key )
			{
				case SDLK_y:
					zoom_scope = false;
					break;
			}
		}
		key = sys.get_key().sym;
	}
}

#endif
