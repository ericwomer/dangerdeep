// user display: submarine's UZO
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "system.h"
#include "image.h"
#include "texture.h"
#include "game.h"
#include "command.h"
#include "sub_uzo_display.h"
#include "user_interface.h"



void sub_uzo_display::pre_display(game& gm) const
{
	glClearColor(0, 0, 0, 0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
}



freeview_display::projection_data sub_uzo_display::get_projection_data(class game& gm) const
{
	projection_data pd;
	pd.x = 0;
	pd.y = system::sys().get_res_y() - system::sys().get_res_x()/2;
	pd.w = system::sys().get_res_x();
	pd.h = system::sys().get_res_x()/2;
	// with normal fov of 70 degrees, this is 1.5 / 6.0 magnification
	pd.fov_x = zoomed ? 13.31 : 50.05;	//fixme: historic values?
	pd.near_z = 1.0;
	pd.far_z = gm.get_max_view_distance();
	return pd;
}



void sub_uzo_display::post_display(game& gm) const
{
	system::sys().prepare_2d_drawing();
	uzotex->draw(0, 0, 512, 512);
	uzotex->draw_hm(512, 0, 512, 512);
	ui.draw_infopanel(gm);
	system::sys().unprepare_2d_drawing();
}



sub_uzo_display::sub_uzo_display(user_interface& ui_) : freeview_display(ui_), zoomed(false)
{
	uzotex = texturecache.ref("uzo.png");

	pos = vector3(0, 0, 6);//fixme, depends on sub
	aboard = true;
	withunderwaterweapons = false;
	drawbridge = false;
}



sub_uzo_display::~sub_uzo_display()
{
	texturecache.unref("uzo.png");
}
