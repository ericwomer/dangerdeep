// user display: general map view
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "system.h"
#include "image.h"
#include "texture.h"
#include "game.h"
#include "coastmap.h"
#include "map_display.h"
#include "user_interface.h"
#include "ship.h"
#include "submarine.h"
#include "airplane.h"
#include "texts.h"
#include "keys.h"
#include "cfg.h"
#include <sstream>
using namespace std;

#define MAPGRIDSIZE 1000	// meters



void map_display::draw_vessel_symbol(const vector2& offset, sea_object* so, color c) const
{
	vector2 d = so->get_heading().direction();
	float w = so->get_width()*mapzoom/2, l = so->get_length()*mapzoom/2;
	vector2 p = (so->get_pos().xy() + offset) * mapzoom;
	p.x += 512;
	p.y = 384 - p.y;

	c.set_gl_color();
	glBindTexture(GL_TEXTURE_2D, 0);
	glBegin(GL_QUADS);
	glVertex2f(p.x - d.y*w, p.y - d.x*w);
	glVertex2f(p.x - d.x*l, p.y + d.y*l);
	glVertex2f(p.x + d.y*w, p.y + d.x*w);
	glVertex2f(p.x + d.x*l, p.y - d.y*l);
	glEnd();
	glBegin(GL_LINES);
	glVertex2f(p.x - d.x*l, p.y + d.y*l);
	glVertex2f(p.x + d.x*l, p.y - d.y*l);
	glEnd();
	glColor3f(1,1,1);
}



void map_display::draw_trail(sea_object* so, const vector2& offset) const
{
	ship* shp = dynamic_cast<ship*>(so);
	if (shp) {
		list<vector2> l = shp->get_previous_positions();
		glColor4f(1,1,1,1);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBegin(GL_LINE_STRIP);
		vector2 p = (shp->get_pos().xy() + offset)*mapzoom;
		glVertex2f(512+p.x, 384-p.y);
		float la = 1.0/float(l.size()), lc = 0;
		for (list<vector2>::const_iterator it = l.begin(); it != l.end(); ++it) {
			glColor4f(1,1,1,1-lc);
			vector2 p = (*it + offset)*mapzoom;
			glVertex2f(512+p.x, 384-p.y);
			lc += la;
		}
		glEnd();
		glColor4f(1,1,1,1);
	}
}



void map_display::draw_pings(class game& gm, const vector2& offset) const
{
	// draw pings (just an experiment, you can hear pings, locate their direction
	//	a bit fuzzy but not their origin or exact shape).
	const list<game::ping>& pings = gm.get_pings();
	for (list<game::ping>::const_iterator it = pings.begin(); it != pings.end(); ++it) {
		const game::ping& p = *it;
		// vector2 r = player->get_pos ().xy () - p.pos;
		vector2 p1 = (p.pos + offset)*mapzoom;
		vector2 p2 = p1 + (p.dir + p.ping_angle).direction() * p.range * mapzoom;
		vector2 p3 = p1 + (p.dir - p.ping_angle).direction() * p.range * mapzoom;
		glBegin(GL_TRIANGLES);
		glColor4f(0.5,0.5,0.5,1);
		glVertex2f(512+p1.x, 384-p1.y);
		glColor4f(0.5,0.5,0.5,0);
		glVertex2f(512+p2.x, 384-p2.y);
		glVertex2f(512+p3.x, 384-p3.y);
		glEnd();
		glColor4f(1,1,1,1);
	}
}



void map_display::draw_sound_contact(class game& gm, const sea_object* player,
	double max_view_dist, const vector2& offset) const
{
	// draw sound contacts
	list<ship*> ships;
	gm.sonar_ships(ships, player);
	for (list<ship*>::iterator it = ships.begin(); it != ships.end(); ++it) {
		vector2 ldir = (*it)->get_pos().xy() - player->get_pos().xy();
		ldir = ldir.normal() * 0.666666 * max_view_dist*mapzoom;
		vector2 pos = (player->get_pos().xy() + offset) * mapzoom;
		if ((*it)->get_class() == ship::MERCHANT)
			glColor3f(0,0,0);
		else if ((*it)->get_class() == ship::WARSHIP)
			glColor3f(0,0.5,0);
		else if ((*it)->get_class() == ship::ESCORT)
			glColor3f(1,0,0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBegin(GL_LINES);
		glVertex2f(512+pos.x, 384-pos.y);
		glVertex2f(512+pos.x+ldir.x, 384-pos.y-ldir.y);
		glEnd();
		glColor3f(1,1,1);
	}

	list<submarine*> submarines;
	gm.sonar_submarines ( submarines, player );
	for ( list<submarine*>::iterator it = submarines.begin ();
		it != submarines.end (); it ++ )
	{
		vector2 ldir = (*it)->get_pos().xy() - player->get_pos().xy();
		ldir = ldir.normal() * 0.666666 * max_view_dist*mapzoom;
		// Submarines are drawn in blue.
		glColor3f(0,0,1);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBegin(GL_LINES);
		glVertex2f(512,384);
		glVertex2f(512+ldir.x, 384-ldir.y);
		glEnd();
		glColor3f(1,1,1);
	}
}



void map_display::draw_visual_contacts(class game& gm,
    const sea_object* player, const vector2& offset) const
{
	// draw vessel trails and symbols (since player is submerged, he is drawn too)
	list<ship*> ships;
	gm.visible_ships(ships, player);
	list<submarine*> submarines;
	gm.visible_submarines(submarines, player);
	list<airplane*> airplanes;
	gm.visible_airplanes(airplanes, player);
	list<torpedo*> torpedoes;
	gm.visible_torpedoes(torpedoes, player);

   	// draw trails
   	for (list<ship*>::iterator it = ships.begin(); it != ships.end(); ++it)
   		draw_trail(*it, offset);
   	for (list<submarine*>::iterator it = submarines.begin(); it != submarines.end(); ++it)
   		draw_trail(*it, offset);
   	for (list<airplane*>::iterator it = airplanes.begin(); it != airplanes.end(); ++it)
   		draw_trail(*it, offset);
   	for (list<torpedo*>::iterator it = torpedoes.begin(); it != torpedoes.end(); ++it)
   		draw_trail(*it, offset);

   	// draw vessel symbols
   	for (list<ship*>::iterator it = ships.begin(); it != ships.end(); ++it)
   		draw_vessel_symbol(offset, *it, color(192,255,192));
   	for (list<submarine*>::iterator it = submarines.begin(); it != submarines.end(); ++it)
   		draw_vessel_symbol(offset, *it, color(255,255,128));
   	for (list<airplane*>::iterator it = airplanes.begin(); it != airplanes.end(); ++it)
   		draw_vessel_symbol(offset, *it, color(0,0,64));
   	for (list<torpedo*>::iterator it = torpedoes.begin(); it != torpedoes.end(); ++it)
   		draw_vessel_symbol(offset, *it, color(255,0,0));
}



void map_display::draw_square_mark ( class game& gm,
	const vector2& mark_pos, const vector2& offset, const color& c ) const
{
	c.set_gl_color ();
	glBindTexture(GL_TEXTURE_2D, 0);
	glBegin ( GL_LINE_LOOP );
	vector2 p = ( mark_pos + offset ) * mapzoom;
	int x = int ( round ( p.x ) );
	int y = int ( round ( p.y ) );
	glVertex2i ( 512-4+x,384-4-y );
	glVertex2i ( 512+4+x,384-4-y );
	glVertex2i ( 512+4+x,384+4-y );
	glVertex2i ( 512-4+x,384+4-y );
	glEnd ();
}



map_display::map_display(user_interface& ui_) :
	user_display(ui_), mapzoom(0.1), mx(0), my(0)
{
}



map_display::~map_display()
{
}



void map_display::display(class game& gm) const
{
	sea_object* player = gm.get_player ();
	bool is_day_mode = gm.is_day_mode ();

	if ( is_day_mode )
		glClearColor ( 0.0f, 0.0f, 1.0f, 1.0f );
	else
		glClearColor ( 0.0f, 0.0f, 0.75f, 1.0f );
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	double max_view_dist = gm.get_max_view_distance();

	vector2 offset = player->get_pos().xy() + mapoffset;

	sys().prepare_2d_drawing();

	float delta = MAPGRIDSIZE*mapzoom;
	float sx = myfmod(512, delta)-myfmod(offset.x, MAPGRIDSIZE)*mapzoom;
	float sy = 768.0 - (myfmod(384.0f, delta)-myfmod(offset.y, MAPGRIDSIZE)*mapzoom);
	int lx = int(1024/delta)+2, ly = int(768/delta)+2;

	// draw grid
	if (mapzoom >= 0.01) {
		glColor3f(0.5, 0.5, 1);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBegin(GL_LINES);
		for (int i = 0; i < lx; ++i) {
			glVertex2f(sx, 0);
			glVertex2f(sx, 768);
			sx += delta;
		}
		for (int i = 0; i < ly; ++i) {
			glVertex2f(0, sy);
			glVertex2f(1024, sy);
			sy -= delta;
		}
		glEnd();
	}

	glColor3f(1,1,1);
	glPushMatrix();
	glTranslatef(512, 384, 0);
	glScalef(mapzoom, mapzoom, 1);
	glScalef(1,-1,1);
	glTranslatef(-offset.x, -offset.y, 0);
	ui.get_coastmap().draw_as_map(offset, mapzoom);//, detl); // detail should depend on zoom, fixme
	glPopMatrix();
	glBindTexture(GL_TEXTURE_2D, 0);

	// draw convoy positions	fixme: should be static and fade out after some time
	glColor3f(1,1,1);
	list<vector2> convoy_pos;
	gm.convoy_positions(convoy_pos);
	for (list<vector2>::iterator it = convoy_pos.begin(); it != convoy_pos.end(); ++it) {
		draw_square_mark ( gm, (*it), -offset, color ( 0, 0, 0 ) );
	}
	glColor3f(1,1,1);

	// draw view range
	glColor3f(1,0,0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBegin(GL_LINE_LOOP);
	for (int i = 0; i < 512; ++i) {
		float a = i*2*M_PI/512;
		glVertex2f(512+(sin(a)*max_view_dist-mapoffset.x)*mapzoom,
			   384-(cos(a)*max_view_dist-mapoffset.y)*mapzoom);
	}
	glEnd();
	glColor3f(1,1,1);

	sea_object* target = ui.get_target();

	// draw vessel symbols (or noise contacts)
	submarine* sub_player = dynamic_cast<submarine*> ( player );
	if (sub_player && sub_player->is_submerged ()) {
		// draw pings
		draw_pings(gm, -offset);

		// draw sound contacts
		draw_sound_contact(gm, sub_player, max_view_dist, -offset);

		// draw player trails and player
		draw_trail(player, -offset);
		draw_vessel_symbol(-offset, sub_player, color(255,255,128));

		// Special handling for submarine player: When the submarine is
		// on periscope depth and the periscope is up the visual contact
		// must be drawn on map.
		if ((sub_player->get_depth() <= sub_player->get_periscope_depth()) &&
			sub_player->is_scope_up())
		{
			draw_visual_contacts(gm, sub_player, -offset);

			// Draw a red box around the selected target.
			if (target)
			{
				draw_square_mark ( gm, target->get_pos ().xy (), -offset,
					color ( 255, 0, 0 ) );
				glColor3f ( 1.0f, 1.0f, 1.0f );
			}
		}
	} 
	else	 	// enable drawing of all object as testing hack by commenting this, fixme
	{
		draw_visual_contacts(gm, player, -offset);

		// Draw a red box around the selected target.
		if (target)
		{
			draw_square_mark ( gm, target->get_pos ().xy (), -offset,
				color ( 255, 0, 0 ) );
			glColor3f ( 1.0f, 1.0f, 1.0f );
		}
	}

	// draw notepad sheet giving target distance, speed and course
	if (target) {
		int nx = 768, ny = 512;
		notepadsheet->draw(nx, ny);
		ostringstream os0, os1, os2;
		// fixme: use estimated values from target/tdc estimation here, make functions for that
		os0 << texts::get(3) << ": " << unsigned(target->get_pos().xy().distance(player->get_pos().xy())) << texts::get(206);
		os1 << texts::get(4) << ": " << unsigned(sea_object::ms2kts(target->get_speed())) << texts::get(208);
		os2 << texts::get(1) << ": " << unsigned(target->get_heading().value()) << texts::get(207);
		font_arial->print(nx+16, ny+40, os0.str(), color(0,0,128));
		font_arial->print(nx+16, ny+60, os1.str(), color(0,0,128));
		font_arial->print(nx+16, ny+80, os2.str(), color(0,0,128));
	}

	// draw world coordinates for mouse
	double mouserealmx = double(mx - 512) / mapzoom + offset.x;
	double mouserealmy = double(384 - my) / mapzoom + offset.y;
	unsigned degrx, degry, minutx, minuty;
	bool west, south;
	sea_object::meters2degrees(mouserealmx, mouserealmy, west, degrx, minutx, south, degry, minuty);
	ostringstream rwcoordss;
	rwcoordss	<< degry << "/" << minuty << (south ? "S" : "N") << ", "
			<< degrx << "/" << minutx << (west ? "W" : "E");
	font_arial->print(0, 0, rwcoordss.str(), color::white(), true);

	ui.draw_infopanel();
	sys().unprepare_2d_drawing();

}



void map_display::process_input(class game& gm, const SDL_Event& event)
{
	sea_object* player = gm.get_player();
	switch (event.type) {
	case SDL_MOUSEBUTTONDOWN:
		if (event.button.button == SDL_BUTTON_LEFT) {
			// set target. get visible objects and determine which is nearest to
			// mouse position. set target for player object
			vector2 mapclick(event.button.x, event.button.y);
			vector<sea_object*> objs = gm.visible_surface_objects(player);
			double mapclickdist = 1e30;
			sea_object* target = 0;
			for (vector<sea_object*>::iterator it = objs.begin(); it != objs.end(); ++it) {
				if (!(*it)->is_alive()) continue;
				vector2 p = ((*it)->get_pos().xy() - (player->get_pos().xy()
								      + mapoffset)) * mapzoom;
				p.x += 512;
				p.y = 384 - p.y;
				double clickd = mapclick.square_distance(p);
				if (clickd < mapclickdist) {
					target = *it;	// fixme: message?
					mapclickdist = clickd;
				}
			}

			ui.set_target(target);
		}
		break;
	case SDL_MOUSEMOTION:
		mx = event.motion.x;
		my = event.motion.y;
		if (event.motion.state & SDL_BUTTON_MMASK) {
			mapoffset.x += event.motion.xrel / mapzoom;
			mapoffset.y += -event.motion.yrel / mapzoom;
		}
	case SDL_KEYDOWN:
		if (cfg::instance().getkey(KEY_ZOOM_MAP).equal(event.key.keysym)) {
			if (mapzoom < 1) mapzoom *= 2;
		} else if (cfg::instance().getkey(KEY_UNZOOM_MAP).equal(event.key.keysym)) {
			if (mapzoom > 1.0/16384) mapzoom /= 2;
		}
		break;
	default:
		break;
	}
}
