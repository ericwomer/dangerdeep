// user display: free 3d view
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "system.h"
#include "image.h"
#include "texture.h"
#include "game.h"
#include "user_interface.h"
#include "freeview_display.h"
#include "ship.h"
#include "submarine.h"
#include "airplane.h"
#include "torpedo.h"
#include "depth_charge.h"
#include "gun_shell.h"
#include "particle.h"
#include "sky.h"
#include "water.h"
#include "global_data.h"
#include "model.h"
#include <fstream>



void freeview_display::pre_display(game& gm) const
{
}



freeview_display::projection_data freeview_display::get_projection_data(game& gm) const
{
	projection_data pd;
	pd.x = 0;
	pd.y = 0;
	pd.w = system::sys().get_res_x();
	pd.h = system::sys().get_res_y();
	pd.fov_x = 70.0;
	pd.near_z = 1.0;
	pd.far_z = gm.get_max_view_distance();
	return pd;
}



void freeview_display::set_modelview_matrix(game& gm) const
{
	glLoadIdentity();

	glRotated(-get_elevation().value(),1,0,0);

	// This should be a negative angle, but nautical view dir is clockwise,
	// OpenGL uses ccw values, so this is a double negation
	glRotated(get_bearing().value(),0,0,1);

	// if we're aboard the player's vessel move the world instead of the ship
	if (aboard) {
		double rollfac = (dynamic_cast<ship*>(gm.get_player()))->get_roll_factor();
		ui.rotate_by_pos_and_wave(gm.get_player()->get_pos(), rollfac, true);
	}

	glTranslated(-pos.x, -pos.y, -pos.z);
}



void freeview_display::post_display(game& gm) const
{
/*
	system::sys().prepare_2d_drawing();
	ui.draw_infopanel(gm);
	system::sys().unprepare_2d_drawing();
*/
}



freeview_display::freeview_display(user_interface& ui_) :
	user_display(ui_), aboard(false), withunderwaterweapons(true), drawbridge(false)
{
	pos = vector3(20,0,10);
}



freeview_display::~freeview_display()
{
}



void freeview_display::display(class game& gm) const
{
	// glClear or not, background drawing
	pre_display(gm);

	// render scene
	draw_view(gm);

	// e.g. drawing of infopanel or 2d effects, background mask etc.
	post_display(gm);
}



void freeview_display::process_input(class game& gm, const SDL_Event& event)
{
	submarine* sub = dynamic_cast<submarine*>(gm.get_player());

	glPushMatrix();
	glLoadIdentity();
	set_modelview_matrix(gm);
	matrix4 viewmatrix = matrix4::get_gl(GL_MODELVIEW_MATRIX);
	glPopMatrix();
	vector3 sidestep = viewmatrix.row(0);
	vector3 upward = viewmatrix.row(1);
	vector3 forward = -viewmatrix.row(2);

	switch (event.type) {
	case SDL_KEYDOWN:
		switch(event.key.keysym.sym) {
		case SDLK_KP8: pos -= forward * 5; break;
		case SDLK_KP2: pos += forward * 5; break;
		case SDLK_KP4: pos -= sidestep * 5; break;
		case SDLK_KP6: pos += sidestep * 5; break;
		case SDLK_KP1: pos -= upward * 5; break;
		case SDLK_KP3: pos += upward * 5; break;
		default: break;
		}
		break;
	case SDL_MOUSEMOTION:
		if (event.motion.state & SDL_BUTTON_LMASK) {
			set_bearing(get_bearing() + event.motion.xrel*0.5);
			set_elevation(get_elevation() - event.motion.yrel*0.5);
			//fixme handle clamping of elevation at +-90deg
		}
		break;
	default:
		break;
	}
}



void freeview_display::draw_objects(game& gm, const vector3& viewpos) const
{
	// simulate horizon: d is distance to object (on perimeter of earth)
	// z is additional height (negative!), r is earth radius
	// z = r*sin(PI/2 - d/r) - r
	// d = PI/2*r - r*arcsin(z/r+1), fixme implement

	sea_object* player = gm.get_player();

	glColor3f(1,1,1);

	list<ship*> ships;
	gm.visible_ships(ships, player);
	for (list<ship*>::const_iterator it = ships.begin(); it != ships.end(); ++it) {
		if (aboard && *it == player) continue;
		glPushMatrix();
		// fixme: z translate according to water height here
		vector3 pos = (*it)->get_pos() - viewpos;
//		pos.z += EARTH_RADIUS * (sin(M_PI/2 - pos.xy().length()/EARTH_RADIUS) - 1.0);
		glTranslated(pos.x, pos.y, pos.z);
		glRotatef(-(*it)->get_heading().value(), 0, 0, 1);
		ui.rotate_by_pos_and_wave((*it)->get_pos(), (*it)->get_roll_factor());
		(*it)->display();

		glPopMatrix();
	}

	list<submarine*> submarines;
	gm.visible_submarines(submarines, player);
	for (list<submarine*>::const_iterator it = submarines.begin(); it != submarines.end(); ++it) {
		if (aboard && *it == player) continue;
		glPushMatrix();
		// fixme: z translate according to water height here
		vector3 pos = (*it)->get_pos() - viewpos;
		glTranslated(pos.x, pos.y, pos.z);
		glRotatef(-(*it)->get_heading().value(), 0, 0, 1);
		if ((*it)->get_pos().z > -15) {
			ui.rotate_by_pos_and_wave((*it)->get_pos(), (*it)->get_roll_factor());
		}
		(*it)->display();
		glPopMatrix();
	}

	list<airplane*> airplanes;
	gm.visible_airplanes(airplanes, player);
	for (list<airplane*>::const_iterator it = airplanes.begin(); it != airplanes.end(); ++it) {
		if (aboard && *it == player) continue;
		glPushMatrix();
		vector3 pos = (*it)->get_pos() - viewpos;
		glTranslated(pos.x, pos.y, pos.z);
		glRotatef(-(*it)->get_heading().value(), 0, 0, 1);	// simulate pitch, roll etc.
		(*it)->display();
		glPopMatrix();
	}

	if (withunderwaterweapons) {
		list<torpedo*> torpedoes;
		gm.visible_torpedoes(torpedoes, player);
		for (list<torpedo*>::const_iterator it = torpedoes.begin(); it != torpedoes.end(); ++it) {
			glPushMatrix();
			vector3 pos = (*it)->get_pos() - viewpos;
			glTranslated(pos.x, pos.y, pos.z);
			glRotatef(-(*it)->get_heading().value(), 0, 0, 1);
			(*it)->display();
			glPopMatrix();
		}
		list<depth_charge*> depth_charges;
		gm.visible_depth_charges(depth_charges, player);
		for (list<depth_charge*>::const_iterator it = depth_charges.begin(); it != depth_charges.end(); ++it) {
			glPushMatrix();
			vector3 pos = (*it)->get_pos() - viewpos;
			glTranslated(pos.x, pos.y, pos.z);
			glRotatef(-(*it)->get_heading().value(), 0, 0, 1);
			(*it)->display();
			glPopMatrix();
		}
	}

	list<gun_shell*> gun_shells;
	gm.visible_gun_shells(gun_shells, player);
	for (list<gun_shell*>::const_iterator it = gun_shells.begin(); it != gun_shells.end(); ++it) {
		glPushMatrix();
		vector3 pos = (*it)->get_pos() - viewpos;
		glTranslated(pos.x, pos.y, pos.z);
		glRotatef(-(*it)->get_heading().value(), 0, 0, 1);
		(*it)->display();
		glPopMatrix();
	}

	list<particle*> particles;
	gm.visible_particles(particles, player);
	particle::display_all(particles, viewpos, gm);
}








void freeview_display::draw_view(game& gm) const
{

	//fixme: viewpos plus additional player pos leads to wrong water drawing.
	//just add up both positions, let mv matrix have zero translation.

	double max_view_dist = gm.get_max_view_distance();

	// the modelview matrix is set around the player's object position.
	// i.e. it has an translation part of zero if the viewer is exactly at the player's object position.
	// this means all objects have to be drawn with an offset of (-player->get_pos())
	// this is done because the position's values can be rather large (global coordinates!) which leads
	// to rounding errors when storing them in the OpenGL matrix. So we have to compute them partly manually
	// with high(er) precision (double).
	// the real viewing position (global coordinates) is stored in viewpos.

	sea_object* player = gm.get_player();

	projection_data pd = get_projection_data(gm);

	// *************** compute and set player pos ****************************************
	set_modelview_matrix(gm);

	vector3 camerapos = matrix4::get_gl(GL_MODELVIEW_MATRIX).inverse().column(3);
	vector3 viewpos = player->get_pos() + camerapos;

	// **************** prepare drawing ***************************************************

	GLfloat horizon_color[4];
	ui.get_sky().get_horizon_color(gm, viewpos).store_rgba(horizon_color);

	color lightcol = gm.compute_light_color(viewpos);

	// compute light source position and brightness (must be set AFTER modelview matrix)
	vector3 sundir = gm.compute_sun_pos(viewpos).normal();
	GLfloat lposition[4] = { sundir.x, sundir.y, sundir.z, 1.0f };
	GLfloat lambient[4] = { 0.05f, 0.05f, 0.05f };
	GLfloat ldiffuse[4] = {lightcol.r/255.0f, lightcol.g/255.0f, lightcol.b/255.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, lambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, ldiffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, ldiffuse);

	// ********************* draw mirrored scene
	
	matrix4 reflection_projmvmat;	// given later to water::display to set up reflection

	// ************ compute water reflection ******************************************
	// save projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	// store the projection matrix
	system::sys().gl_perspective_fovx(pd.fov_x * 1.0, 1.0 /*aspect*/, pd.near_z, pd.far_z);
	reflection_projmvmat = matrix4::get_gl(GL_PROJECTION_MATRIX) * matrix4::get_gl(GL_MODELVIEW_MATRIX);
	glLoadIdentity();
	// set up projection matrix (new width/height of course) with a bit larger fov
	system::sys().gl_perspective_fovx(pd.fov_x * 1.0, 1.0 /*aspect*/, pd.near_z, pd.far_z);
	// set up new viewport size s*s with s<=max_texure_size and s<=w,h of old viewport
	unsigned vps = ui.get_water().get_reflectiontex_size();
	glViewport(0, 0, vps, vps);
	// clear depth buffer (not needed for sky drawing, but this color can be seen as mirror
	// image when looking from below the water surface (scope) fixme: clear color with upwelling color!)
	glClearColor(0, 1.0f/16, 1.0f/8, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// shear one clip plane to match world space z=0 plane
	//fixme

	// flip geometry at z=0 plane
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glScalef(1.0f, 1.0f, -1.0f);

	// flip light!
	glLightfv(GL_LIGHT0, GL_POSITION, lposition);

	// draw all parts of the scene that are (partly) above the water:
	//   sky
	glCullFace(GL_FRONT);

	//fixme: viewpos/pos z translation is not computed correctly here... so mirrored image of sky is wrong
	glPushMatrix();
	// compensate camera height. should also be done for other objects...fixme
	glTranslated(0, 0, -2*camerapos.z);
	ui.get_sky().display(gm, viewpos, max_view_dist, true);
	glPopMatrix();

	//   terrain
	glColor4f(1,1,1,1);//fixme: fog is missing
	ui.draw_terrain(viewpos, get_bearing(), max_view_dist);
	//fixme
	//   models/smoke
	//fixme
	glCullFace(GL_BACK);
	// copy viewport pixel data to reflection texture

	glBindTexture(GL_TEXTURE_2D, ui.get_water().get_reflectiontex());
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, vps, vps, 0);

#if 0
	vector<Uint8> scrn(vps*vps*3);
	//unsigned t0 = SDL_GetTicks();
	glReadPixels(0, 0, vps, vps, GL_RGB, GL_UNSIGNED_BYTE, &scrn[0]);
	//unsigned t1 = SDL_GetTicks();
	//cout << "time for " << vps << " size tex: " << t1-t0 << "\n"; // 14-20 ms each, ~1-2ms for a 128pixel texture
	scrn[0] = 255;//test to see where is up
	ofstream oss("mirror.ppm");
	oss << "P6\n" << vps << " " << vps << "\n255\n";
	oss.write((const char*)(&scrn[0]), vps*vps*3);
#endif

	// clean up
	glPopMatrix();

	// ************************** draw the real scene ********************************

	glViewport(pd.x, pd.y, pd.w, pd.h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	system::sys().gl_perspective_fovx(pd.fov_x, double(pd.w)/double(pd.h), pd.near_z, pd.far_z);
	glMatrixMode(GL_MODELVIEW);

	//fixme: water reflections are brighter than the sky, so there must be a difference between sky drawing
	//and mirrored sky drawing...
	//yes, because sky is blended into background, and that color is different.
	//mirror scene background color is horizon color, so that in the mirrored image the background pixels
	//don't bleed into the sky/horizon pixels...
	//shouldn't matter anyway because of water fog... test this

	glClearColor(0, 0, 0, 0);
	// this color clear eats ~ 2 frames (52 to 50 on a gf4mx), but is needed for star sky drawing
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	// set light! fixme the models are dark now. maybe we have to use the same modelview matrix that we used when creating the initial pos.?!
	glLightfv(GL_LIGHT0, GL_POSITION, lposition);


	// ************ sky ***************************************************************
	ui.get_sky().display(gm, viewpos, max_view_dist, false);

	// ********* set fog for scene ****************************************************
	glFogi(GL_FOG_MODE, GL_LINEAR );
	glFogfv(GL_FOG_COLOR, horizon_color);
	glFogf(GL_FOG_DENSITY, 1.0);	// not used in linear mode
	glHint(GL_FOG_HINT, GL_NICEST /*GL_FASTEST*/ /*GL_DONT_CARE*/);
	glFogf(GL_FOG_START, max_view_dist*0.75);	// ships disappear earlier :-(
	glFogf(GL_FOG_END, max_view_dist);
	glEnable(GL_FOG);	

	// ******* water ***************************************************************
	//ui.get_water().update_foam(1.0/25.0);  //fixme: deltat needed here
	//ui.get_water().spawn_foam(vector2(myfmod(gm.get_time(),256.0),0));
	//fixme: viewpos should be submarine pos, not already viewer trans of mv mat added...
	ui.get_water().display(viewpos, get_bearing(), max_view_dist, reflection_projmvmat);

	// ******** terrain/land ********************************************************
//	glDisable(GL_FOG);	//testing with new 2d bspline terrain.
	ui.draw_terrain(viewpos, get_bearing(), max_view_dist);
//	glEnable(GL_FOG);	

/* test hack, remove
	glCullFace(GL_FRONT);
	glPushMatrix();
	glScalef(1,1,-1);
	draw_terrain(viewpos, dir, max_view_dist);
	glCullFace(GL_BACK);
	glPopMatrix();
*/

	// ******************** ships & subs *************************************************
//	cout << "mv trans pos " << matrix4::get_gl(GL_MODELVIEW_MATRIX).column(3) << "\n";
	// substract player pos.
	draw_objects(gm, player->get_pos());

	// ******************** draw the bridge in higher detail
	if (aboard && drawbridge) {
		// after everything was drawn, draw conning tower (new projection matrix needed)
//		glClear(GL_DEPTH_BUFFER_BIT);
//		glDisable(GL_DEPTH_TEST);
//		glMatrixMode(GL_PROJECTION);
//		glPushMatrix();
//		glLoadIdentity();
//		system::sys().gl_perspective_fovx (90.0, 4.0/3.0 /* fixme may change */, 0.5, 100.0);
//		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glTranslated(0, 0, 6);
//		glLoadIdentity();
//		glRotatef(-elevation.value()-90,1,0,0);
		glRotatef(-player->get_heading().value(),0,0,1);
//fixme handle different! with modelcache!
		conning_tower_typeVII->display();
//		glPopMatrix();
//		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
//		glMatrixMode(GL_MODELVIEW);
//		glEnable(GL_DEPTH_TEST);
	}

	glDisable(GL_FOG);	
	glColor4f(1,1,1,1);

	ui.draw_weather_effects(gm);
}



angle freeview_display::get_bearing(void) const
{
	return ui.get_bearing();
}



angle freeview_display::get_elevation(void) const
{
	return ui.get_elevation();
}



void freeview_display::set_bearing(const angle& a)
{
	ui.set_bearing(a);
}



void freeview_display::set_elevation(const angle& a)
{
	ui.set_elevation(a);
}
