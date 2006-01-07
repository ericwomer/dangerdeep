/*
Danger from the Deep - Open source submarine simulation
Copyright (C) 2003-2006  Thorsten Jordan, Luis Barrancos and others.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

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
	pd.w = sys().get_res_x();
	pd.h = sys().get_res_y();
	pd.fov_x = 70.0;
	pd.near_z = 0.2;	// fixme: should be 1.0, but new conning tower needs 0.1 or so
	pd.far_z = gm.get_max_view_distance();
	return pd;
}



void freeview_display::set_modelview_matrix(game& gm, const vector3& viewpos) const
{
	glLoadIdentity();

	// set up rotation (player's view direction)
	glRotated(-ui.get_elevation().value(),1,0,0);

	// This should be a negative angle, but nautical view dir is clockwise,
	// OpenGL uses ccw values, so this is a double negation
	glRotated(ui.get_absolute_bearing().value(),0,0,1);

	// if we're aboard the player's vessel move the world instead of the ship
	if (aboard) {
		const sea_object* pl = gm.get_player();
		double rollfac = (dynamic_cast<const ship*>(pl))->get_roll_factor();
		ui.rotate_by_pos_and_wave(pl->get_pos(), pl->get_heading(),
					  pl->get_length(), pl->get_width(), rollfac, true);
	}

	// set up modelview matrix as if player is at position (0, 0, 0), so do NOT set a translational part.
	// This is done to avoid rounding errors caused by large x/y values (modelview matrix seems to store floats,
	// but coordinates are in real meters, so float is not precise enough).
}



void freeview_display::post_display(game& gm) const
{
/*
	sys().prepare_2d_drawing();
	ui.draw_infopanel(gm);
	sys().unprepare_2d_drawing();
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
	vector3 viewpos = gm.get_player()->get_pos() + pos;
//	cout << "pos is " << pos << " playerpos " << gm.get_player()->get_pos() << " viewpos " << viewpos << " aboard: " << aboard << "\n";
	draw_view(gm, viewpos);

	// e.g. drawing of infopanel or 2d effects, background mask etc.
	post_display(gm);
}



void freeview_display::process_input(class game& gm, const SDL_Event& event)
{
	submarine* sub = dynamic_cast<submarine*>(gm.get_player());

	glPushMatrix();
	glLoadIdentity();
	set_modelview_matrix(gm, vector3());	// position doesn't matter, only direction.
	matrix4 viewmatrix = matrix4::get_gl(GL_MODELVIEW_MATRIX);
	glPopMatrix();
	vector3 sidestep = viewmatrix.row3(0);
	vector3 upward = viewmatrix.row3(1);
	vector3 forward = -viewmatrix.row3(2);

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
			ui.add_bearing(event.motion.xrel*0.5);
			ui.add_elevation(-event.motion.yrel*0.5);
			//fixme handle clamping of elevation at +-90deg
		}
		break;
	default:
		break;
	}
}



void freeview_display::draw_objects(game& gm, const vector3& viewpos,
				    const vector<ship*>& ships,
				    const vector<submarine*>& submarines,
				    const vector<torpedo*>& torpedoes, bool mirrorclip) const
{
	// simulate horizon: d is distance to object (on perimeter of earth)
	// z is additional height (negative!), r is earth radius
	// z = r*sin(PI/2 - d/r) - r
	// d = PI/2*r - r*arcsin(z/r+1), fixme implement

	sea_object* player = gm.get_player();

	glColor3f(1,1,1);

	for (vector<ship*>::const_iterator it = ships.begin(); it != ships.end(); ++it) {
		if (aboard && *it == player) continue;
		glPushMatrix();
		// fixme: z translate according to water height here
		if (mirrorclip) {
			vector3 pos = (*it)->get_pos();
			glTranslated(pos.x - viewpos.x, pos.y - viewpos.y, -viewpos.z);
			glActiveTexture(GL_TEXTURE1);
			glMatrixMode(GL_TEXTURE);
			glLoadIdentity();
			glTranslated(0, 0, pos.z);
		} else {
			vector3 pos = (*it)->get_pos() - viewpos;
			//pos.z += EARTH_RADIUS * (sin(M_PI/2 - pos.xy().length()/EARTH_RADIUS) - 1.0);
			glTranslated(pos.x, pos.y, pos.z);
		}
		glRotatef(-(*it)->get_heading().value(), 0, 0, 1);
		ui.rotate_by_pos_and_wave((*it)->get_pos(), (*it)->get_heading(),
					  (*it)->get_length(), (*it)->get_width(),
					  (*it)->get_roll_factor());
		if (mirrorclip) {
			glActiveTexture(GL_TEXTURE0);
			glMatrixMode(GL_MODELVIEW);
			(*it)->display_mirror_clip();
			glPopMatrix();
		} else {
			(*it)->display();
			glPopMatrix();
		}
	}

	for (vector<submarine*>::const_iterator it = submarines.begin(); it != submarines.end(); ++it) {
		if (aboard && *it == player) continue;
		glPushMatrix();
		// fixme: z translate according to water height here
		if (mirrorclip) {
			vector3 pos = (*it)->get_pos();
			glTranslated(pos.x - viewpos.x, pos.y - viewpos.y, -viewpos.z);
			glActiveTexture(GL_TEXTURE1);
			glMatrixMode(GL_TEXTURE);
			glLoadIdentity();
			glTranslated(0, 0, pos.z);
		} else {
			vector3 pos = (*it)->get_pos() - viewpos;
			glTranslated(pos.x, pos.y, pos.z);
		}
		glRotatef(-(*it)->get_heading().value(), 0, 0, 1);
		if ((*it)->get_pos().z > -15) {
			ui.rotate_by_pos_and_wave((*it)->get_pos(), (*it)->get_heading(),
						  (*it)->get_length(), (*it)->get_width(),
						  (*it)->get_roll_factor());
		}
		if (mirrorclip) {
			glActiveTexture(GL_TEXTURE0);
			glMatrixMode(GL_MODELVIEW);
			(*it)->display_mirror_clip();
			glPopMatrix();
		} else {
			(*it)->display();
			glPopMatrix();
		}
	}

	vector<airplane*> airplanes = gm.visible_airplanes(player);
	for (vector<airplane*>::const_iterator it = airplanes.begin(); it != airplanes.end(); ++it) {
		if (aboard && *it == player) continue;
		glPushMatrix();
		vector3 pos = (*it)->get_pos() - viewpos;
		glTranslated(pos.x, pos.y, pos.z);
		glRotatef(-(*it)->get_heading().value(), 0, 0, 1);	// simulate pitch, roll etc.
		(*it)->display();
		glPopMatrix();
	}

	if (withunderwaterweapons) {
		for (vector<torpedo*>::const_iterator it = torpedoes.begin(); it != torpedoes.end(); ++it) {
			glPushMatrix();
			vector3 pos = (*it)->get_pos() - viewpos;
			glTranslated(pos.x, pos.y, pos.z);
			glRotatef(-(*it)->get_heading().value(), 0, 0, 1);
			(*it)->display();
			glPopMatrix();
		}
		vector<depth_charge*> depth_charges = gm.visible_depth_charges(player);
		for (vector<depth_charge*>::const_iterator it = depth_charges.begin(); it != depth_charges.end(); ++it) {
			glPushMatrix();
			vector3 pos = (*it)->get_pos() - viewpos;
			glTranslated(pos.x, pos.y, pos.z);
			glRotatef(-(*it)->get_heading().value(), 0, 0, 1);
			(*it)->display();
			glPopMatrix();
		}
	}

	vector<gun_shell*> gun_shells = gm.visible_gun_shells(player);
	for (vector<gun_shell*>::const_iterator it = gun_shells.begin(); it != gun_shells.end(); ++it) {
		glPushMatrix();
		vector3 pos = (*it)->get_pos() - viewpos;
		glTranslated(pos.x, pos.y, pos.z);
		glRotatef(-(*it)->get_heading().value(), 0, 0, 1);
		(*it)->display();
		glPopMatrix();
	}

	vector<particle*> particles = gm.visible_particles(player);
	particle::display_all(particles, viewpos, gm);
}



void freeview_display::draw_view(game& gm, const vector3& viewpos) const
{
	double max_view_dist = gm.get_max_view_distance();

	// the modelview matrix is set around the player's viewing position.
	// i.e. it has an translation part of zero.
	// this means all objects have to be drawn with an offset of (-viewpos)
	// this is done because the position's values can be rather large (global coordinates!) which leads
	// to rounding errors when storing them in the OpenGL matrix. So we have to compute them partly manually
	// with high(er) precision (double).
	// the real viewing position (global coordinates) is stored in viewpos.

	sea_object* player = gm.get_player();

	projection_data pd = get_projection_data(gm);

	// *************** compute and set player pos ****************************************
	set_modelview_matrix(gm, viewpos);

	// **************** prepare drawing ***************************************************

	GLfloat horizon_color[4];
	ui.get_sky().get_horizon_color(gm, viewpos).store_rgba(horizon_color);

	color lightcol = gm.compute_light_color(viewpos);

	// compute light source position and brightness (must be set AFTER modelview matrix)
	vector3 sundir = gm.compute_sun_pos(viewpos).normal();
	GLfloat lposition[4] = { sundir.x, sundir.y, sundir.z, 0.0f };
	GLfloat lambient[4] = { 0.1f, 0.1f, 0.1f }; // { 0.05f, 0.05f, 0.05f };
	GLfloat ldiffuse[4] = {lightcol.r/255.0f, lightcol.g/255.0f, lightcol.b/255.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, lambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, ldiffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, ldiffuse);

	// ************************* compute visible surface objects *******************

	// compute visble ships/subs, needed for draw_objects and amount of foam computation
	//fixme: the lookout sensor must give all ships seens around, not cull away ships
	//out of the frustum, or their foam is lost as well, even it would be visible...
	vector<ship*> ships = gm.visible_ships(player);
	vector<submarine*> submarines = gm.visible_submarines(player);
	vector<torpedo*> torpedoes = gm.visible_torpedoes(player);

	// ********************* draw mirrored scene
	
	// ************ compute water reflection ******************************************
	// in theory we have to set up a projection matrix with a slightly larger FOV than the scene projection matrix,
	// because the mirrored scene can show parts that are not seen in normal view.
	// However we use the reflection texture coordinates also for foam, which should match the scene projection,
	// so we do NOT change the projection matrix here.
	// the (old) code for change is shown here:
	// --------- old begin ----------
	// save projection matrix
	//glMatrixMode(GL_PROJECTION);
	//glLoadIdentity();
	// store the projection matrix
	//sys().gl_perspective_fovx(pd.fov_x * 1.0, 1.0 /*aspect*/, pd.near_z, pd.far_z);
	//reflection_projmvmat = matrix4::get_gl(GL_PROJECTION_MATRIX) * matrix4::get_gl(GL_MODELVIEW_MATRIX);
	//glLoadIdentity();
	// set up projection matrix (new width/height of course) with a bit larger fov
	//sys().gl_perspective_fovx(pd.fov_x * 1.0, 1.0 /*aspect*/, pd.near_z, pd.far_z);
	// -------- old end ------------

	// set up new viewport size s*s with s<=max_texure_size and s<=w,h of old viewport
	const texture* refltex = ui.get_water().get_reflectiontex();
	glViewport(0, 0, refltex->get_width(), refltex->get_height());
	// clear depth buffer (not needed for sky drawing, but this color can be seen as mirror
	// image when looking from below the water surface (scope) fixme: clear color with upwelling color!)
	glClearColor(0, 1.0f/16, 1.0f/8, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// shear one clip plane to match world space z=0 plane
	//fixme, use shaders for that, Clip planes are often computed in software and are too slow
	//the question is how they interfere with shaders?
	//yes they are damn slow.
	//give height of object (pos.z) to shader, it then clips at -z.
	//no, instead compute plane to clip to and clip in shader.
	//either clip to arbitrary plane (z=0 water plane in world space is not z=0 in eye space)
	//or transform object space to world space, then clip at z=0 and then transform to eye space.

	{
		matrix_pusher mp(GL_MODELVIEW);
		// flip geometry at z=0 plane
		glScalef(1.0f, 1.0f, -1.0f);
		glCullFace(GL_FRONT);

		// viewpos for drawing mirrored objects should/must be changed
		// to (vp.x, vp.y, -vp.z) (z coordinate negated).
		vector3 viewpos_mirror = vector3(viewpos.x, viewpos.y, -viewpos.z);

		// flip light!
		glLightfv(GL_LIGHT0, GL_POSITION, lposition);

		// draw all parts of the scene that are (partly) above the water:
		//   sky
		ui.get_sky().display(gm, viewpos_mirror, max_view_dist, true);

		//   terrain
		glColor4f(1,1,1,1);//fixme: fog is missing
		ui.draw_terrain(viewpos_mirror, ui.get_absolute_bearing(), max_view_dist);
		//fixme
		//   models/smoke
		// test hack. limit mirror effect only to near objects?
		// the drawn water is nearly a flat plane in the distance so mirroring
		// would be perfect which is highly unrealistic.
		// so remove entries that are too far away. Torpedoes can't be seen
		// so they don't need to get rendered.
		vector<ship*> ships_mirror;
		vector<submarine*> submarines_mirror;
		vector<torpedo*> torpedoes_mirror; // empty
		ships_mirror.reserve(ships.size());
		submarines_mirror.reserve(submarines.size());
		const double MIRROR_DIST = 1000.0;	// 1km or so...
		for (vector<ship*>::iterator it = ships.begin(); it != ships.end(); ++it) {
			if ((*it)->get_pos().xy().square_distance(viewpos.xy()) < MIRROR_DIST*MIRROR_DIST) {
				ships_mirror.push_back(*it);
			}
		}
		for (vector<submarine*>::iterator it = submarines.begin(); it != submarines.end(); ++it) {
			if ((*it)->get_pos().xy().square_distance(viewpos.xy()) < MIRROR_DIST*MIRROR_DIST) {
				submarines_mirror.push_back(*it);
			}
		}
		draw_objects(gm, viewpos_mirror, ships_mirror, submarines_mirror, torpedoes_mirror,
			     true /* mirror */);

		//fixme
		glCullFace(GL_BACK);

		// copy viewport pixel data to reflection texture
		refltex->set_gl_texture();
		glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, refltex->get_width(), refltex->get_height(), 0);
		//fixme: ^ glCopyTexSubImage may be faster!

	}

	// *************************** compute amount of foam for water display *****************

	// compute foam values for water.
	// give pointers to all visible ships for foam calculation, that are all ships, subs
	// and torpedoes. Gun shell impacts/dc explosions will be added later...
	//fixme: foam generated depends on depth of sub and type of torpedo etc.
	vector<ship*> allships;
	allships.reserve(ships.size() + submarines.size() + torpedoes.size());
	for (vector<ship*>::const_iterator it = ships.begin(); it != ships.end(); ++it)
		allships.push_back(*it);
	for (vector<submarine*>::const_iterator it = submarines.begin(); it != submarines.end(); ++it)
		allships.push_back(*it);
	for (vector<torpedo*>::const_iterator it = torpedoes.begin(); it != torpedoes.end(); ++it)
		allships.push_back(*it);
	ui.get_water().compute_amount_of_foam_texture(gm, viewpos, allships);

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

	// ************************** draw the real scene ********************************

	glViewport(pd.x, pd.y, pd.w, pd.h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	sys().gl_perspective_fovx(pd.fov_x, double(pd.w)/double(pd.h), pd.near_z, pd.far_z);
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

	// set light!
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
	ui.get_water().display(viewpos, ui.get_absolute_bearing(), max_view_dist);

	// ******** terrain/land ********************************************************
//	glDisable(GL_FOG);	//testing with new 2d bspline terrain.
	ui.draw_terrain(viewpos, ui.get_absolute_bearing(), max_view_dist);
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
	draw_objects(gm, viewpos, ships, submarines, torpedoes);

	// ******************** draw the bridge in higher detail
	if (aboard && drawbridge) {
		// after everything was drawn, draw conning tower
		vector3 conntowerpos = player->get_pos() - viewpos;
		matrix_pusher mp(GL_MODELVIEW);
		// we would have to translate the conning tower, but the current model is centered arount the player's view
		// already, fixme.
		//glTranslated(conntowerpos.x, conntowerpos.y, conntowerpos.z);
		glRotatef(-player->get_heading().value(),0,0,1);
		conning_tower_typeVII->display();
	}

	glDisable(GL_FOG);	
	glColor4f(1,1,1,1);

	ui.draw_weather_effects();
}
