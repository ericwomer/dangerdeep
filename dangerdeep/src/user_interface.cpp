// user interface common code
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL/SDL.h>
#endif
#include <sstream>
#include "user_interface.h"
#include "system.h"
#include "game.h"
#include "texts.h"
#include "sound.h"
#include "logbook.h"
#include "ships_sunk_display.h"

vector<unsigned char> user_interface::waveheights;
vector<float> user_interface::sinvec;


user_interface::user_interface() :
	player_object(0), quit(false), pause(false), time_scale(1),
	zoom_scope(false), mapzoom(0.1), viewsideang(0), viewupang(-90),
	viewpos(0, 0, 10), bearing(0), viewmode(4), target(0)
{
	init ();
}

user_interface::user_interface(sea_object* player) :
    player_object ( player ), quit(false), pause(false), time_scale(1),
    zoom_scope(false), mapzoom(0.1), viewsideang(0), viewupang(-90),
	viewpos(0, 0, 10), bearing(0), viewmode(4), target(0)
{
	init ();
}

user_interface::~user_interface ()
{
	delete captains_logbook;
	delete ships_sunk_disp;
}

void user_interface::init ()
{
	// if the constructors of these classes may ever fail, we should
	// use C++ exceptions.
	captains_logbook = new captains_logbook_display;
//	system::sys()->myassert ( captains_logbook != 0, "Error while creating captains_logbook!" );
	ships_sunk_disp = new ships_sunk_display;
//	system::sys()->myassert ( ships_sunk_disp != 0, "Error while creating ships_sunk!" );

	if (waveheights.size() == 0) init_water_data();
}

void user_interface::init_water_data(void)
{
	sinvec.resize(256);
	for (int i = 0; i < 256; ++i)
		sinvec[i] = sin(i*M_PI/128);
	waveheights.resize(WAVERND*WAVERND);
	for (int j = 0; j < WAVERND; ++j)
		for (int i = 0; i < WAVERND; ++i)
			waveheights[j*WAVERND+i] = rnd(256);
}

float user_interface::get_waterheight(float x_, float y_, int wave)	// bilinear sampling
{
	float px = x_/WAVESIZE;
	float py = y_/WAVESIZE;
	int x = int(floor(px));
	int y = int(floor(py));
	// fixme: make tide dynamic, depending on weather.
	float h0 = WAVETIDE/2 * sinvec[(get_waterheight(x  , y  ) + wave) & 255];
	float h1 = WAVETIDE/2 * sinvec[(get_waterheight(x+1, y  ) + wave) & 255];
	float h2 = WAVETIDE/2 * sinvec[(get_waterheight(x  , y+1) + wave) & 255];
	float h3 = WAVETIDE/2 * sinvec[(get_waterheight(x+1, y+1) + wave) & 255];
	float dx = (px - floor(px));
	float dy = (py - floor(py));
	float h01 = h0*(1-dx) + h1*dx;
	float h23 = h2*(1-dx) + h3*dx;
	return h01*(1-dy) + h23*dy;
}

void user_interface::draw_water(const vector3& viewpos, angle dir, unsigned wavephase,
	double max_view_dist) const
{
#define WAVESX 16	// number of waves per screen scanline

	// fixme: add "moving" water texture

	// calculate FOV and view dependent direction vectors
	GLfloat projmatrix[16];
	glGetFloatv(GL_PROJECTION_MATRIX, projmatrix);
	double tanfovx2 = projmatrix[0];
	vector2 viewdir = dir.direction();
	vector2 viewleft = viewdir.orthogonal();

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glBindTexture(GL_TEXTURE_2D, water->get_opengl_name());

	// create vertices
	vector<GLfloat> verticecoords;
	vector<GLfloat> texturecoords;
	unsigned verts = (WAVEDEPTH+1)*(WAVESX+1);
	verticecoords.reserve(3*verts+2);
	texturecoords.reserve(2*verts+2);
	double texscalefac = 1.0/double(4*WAVESIZE);
	double zdist = 0;
	for (unsigned w = 0; w <= WAVEDEPTH; ++w) {
		vector2 viewbase = viewpos.xy() + viewdir * zdist + viewleft * zdist * tanfovx2;
		double viewleftfac = -2 * zdist * tanfovx2 / WAVESX;
		for (unsigned p = 0; p <= WAVESX; ++p) {
			// outer border of water must have height 0 to match horizon face
			double height = (w < WAVEDEPTH) ? get_waterheight((float)viewbase.x, (float)viewbase.y, (int)wavephase) : 0;
			verticecoords.push_back(viewbase.x);
			verticecoords.push_back(viewbase.y);
			verticecoords.push_back(height);
			// don't take fractional part of coordinates for texture coordinates
			// or wrap around error will occour. Texture coordinates may get big
			// (real meter values around the globe/map) but OpenGL can handle this
			// (and does that right). Scale texture by adjusting this factor.
			texturecoords.push_back(texscalefac * viewbase.x);
			texturecoords.push_back(texscalefac * viewbase.y);
			viewbase += viewleft * viewleftfac;
		}
		// this could grow more than linear to reduce number of faces drawn at
		// far distance (like voxel). This would lead to bigger waves in the distance
		// which isn't right, so don't overdo that...
		zdist += WAVESIZE;
	}

	/* 2003/07/04 idea.
	   simulate earth curvature by drawing several horizon faces
	   approximating the curvature.
	   earth has medium radius of 6371km, that means 40030km around it.
	   A ship with 15m height above the waterline disappears behind
	   the horizon at ca. 13.825km distance (7.465 sm)
	   
	   exact value 40030.17359km. (u), earth radius (r)
	   
	   height difference in view: (h), distance (d). Formula:
	   
	   h = r * (1 - cos( 360deg * d / u ) )
	   
	   or
	   
	   d = arccos ( 1 - h / r ) * u / 360deg
	   
	   draw ships with height -h. so (dis)appearing of ships can be
	   simulated properly.
	   
	   highest ships are battleships (approx. 30meters), they disappear
	   at 19.551km (10.557 sm).
	   
	   That's much shorter than I thought! But there is a mistake:
	   The viewer's height is not 0 but around 6-8m for submarines,
	   so the formulas are more difficult:
	   
	   The real distance is twice the formula, once for the viewer's
	   height, once for the object:
	   
	   d = (arccos(1 - myh/r) + arccos(1 - h/r)) * u / 360deg
	   
	   or for the watched object
	   
	   h = r * (1 - cos( 360deg * (d - (arccos(1 - myh/r)) / u ) )
	   
	   so for a watcher in 6m height and other ships we have
	   arccos(1-myh/r) = 0.07863384deg
	   15m in height -> dist: 22.569km (12.186sm)
	   30m in height -> dist: 28.295km (15.278sm)
	   
	   This values are useful for computing "normal" simulation's
	   maximum visibility.
	   Waves are disturbing sight but are ignored here.
	*/	   
	
	// additional vertices for horizon face
	vector2 horizon1 = viewpos.xy() + viewdir * max_view_dist;
	vector2 horizon2 = viewleft * max_view_dist * tanfovx2;
	vector2 horizonl = horizon1 + horizon2;
	vector2 horizonr = horizon1 - horizon2;
	verticecoords.push_back(horizonl.x);
	verticecoords.push_back(horizonl.y);
	verticecoords.push_back(0);
	texturecoords.push_back(texscalefac * horizonl.x);
	texturecoords.push_back(texscalefac * horizonl.y);
	verticecoords.push_back(horizonr.x);
	verticecoords.push_back(horizonr.y);
	verticecoords.push_back(0);
	texturecoords.push_back(texscalefac * horizonr.x);
	texturecoords.push_back(texscalefac * horizonr.y);
	
	glVertexPointer(3, GL_FLOAT, 0, &verticecoords[0]);
	glTexCoordPointer(2, GL_FLOAT, 0, &texturecoords[0]);
	
	// create faces, WAVEDEPTH*WAVESX*2 in number
	glBegin(GL_TRIANGLES);
	unsigned vertexnr = 0;
	for (unsigned w = 0; w < WAVEDEPTH; ++w) {
		for (unsigned p = 0; p < WAVESX; ++p) {
			// face 1
			glArrayElement(vertexnr);
			glArrayElement(vertexnr+WAVESX+2);
			glArrayElement(vertexnr+WAVESX+1);
			// face 2
			glArrayElement(vertexnr);
			glArrayElement(vertexnr+1);
			glArrayElement(vertexnr+WAVESX+2);
			++vertexnr;
		}
		++vertexnr;
	}
	
	// horizon faces
	glArrayElement(verts-WAVESX-1);
	glArrayElement(verts+1);
	glArrayElement(verts);
	glArrayElement(verts-WAVESX-1);
	glArrayElement(verts-1);
	glArrayElement(verts+1);
	glEnd();
	
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void user_interface::draw_view(class system& sys, class game& gm, const vector3& viewpos,
	angle dir, bool withplayer, bool withunderwaterweapons)
{
	sea_object* player = get_player();
	// fixme: this wave is used double. for waves the commented version would be
	// best, but ships' movement depend on it also. ugly.
	int wave = int(fmod(gm.get_time(),86400/*WAVETIDECYCLETIME*/)*256/WAVETIDECYCLETIME);
	
	glRotatef(-90,1,0,0);
	glRotatef(dir.value(),0,0,1);
	glTranslatef(-viewpos.x, -viewpos.y, -viewpos.z);
	
	double max_view_dist = gm.get_max_view_distance();

	// ************ sky ***************************************************************
	glPushMatrix();
	glTranslatef(viewpos.x, viewpos.y, 0);
	glPushMatrix();
	glScalef(max_view_dist, max_view_dist, max_view_dist);	// fixme dynamic
	double dt = get_day_time(gm.get_time());
	color skycol1, skycol2, lightcol;
	double colscal;
	if (dt < 1) { colscal = 0; }
	else if (dt < 2) { colscal = fmod(dt,1); }
	else if (dt < 3) { colscal = 1; }
	else { colscal = 1-fmod(dt,1); }
	lightcol = color(color(64, 64, 64), color(255,255,255), colscal);
	skycol1 = color(color(8,8,32), color(165,192,247), colscal);
	skycol2 = color(color(0, 0, 16), color(74,114,236), colscal);

	// compute light source position and brightness
	GLfloat lambient[4] = {lightcol.r/255.0/2.0, lightcol.g/255.0/2.0, lightcol.b/255.0/2.0, 1};
	GLfloat ldiffuse[4] = {lightcol.r/255.0, lightcol.g/255.0, lightcol.b/255.0, 1};
	GLfloat lposition[4] = {0,0,1,0};	//fixed for now. fixme
	glLightfv(GL_LIGHT1, GL_AMBIENT, lambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, ldiffuse);
	glLightfv(GL_LIGHT1, GL_POSITION, lposition);

	glDisable(GL_LIGHTING);
	skycol2.set_gl_color();
	skyhemisphere->display(false);//, &skycol1, &skycol2);
	color::white().set_gl_color();
	glEnable(GL_LIGHTING);
	glPopMatrix();	// remove scale
	
	// ******** clouds *******
	glDisable(GL_LIGHTING);		// direct lighting turned off
	glDisable(GL_DEPTH_TEST);	// draw all clouds
	lightcol.set_gl_color();	// cloud color depends on day time
	glBindTexture(GL_TEXTURE_2D, clouds->get_opengl_name());
	glBegin(GL_QUADS);
	for (unsigned cl = gm.get_nr_of_clouds(); cl > 0; --cl) {
		game::cloud cld = gm.get_cloud(cl-1);
		glTexCoord2f(0, 1);
		glVertex3f(-cld.size+cld.pos.x,  cld.size+cld.pos.y, cld.pos.z);
		glTexCoord2f(1, 1);
		glVertex3f( cld.size+cld.pos.x,  cld.size+cld.pos.y, cld.pos.z);
		glTexCoord2f(1, 0);
		glVertex3f( cld.size+cld.pos.x, -cld.size+cld.pos.y, cld.pos.z);
		glTexCoord2f(0, 0);
		glVertex3f(-cld.size+cld.pos.x, -cld.size+cld.pos.y, cld.pos.z);
	}
	glEnd();
	glPopMatrix();	// remove translate
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	color::white().set_gl_color();

	// ******* water *********
					// fixme: program directional light caused by sun
					// or moon should be reflected by water.
	draw_water(viewpos, dir, wave, max_view_dist);
	

	// ******************** ships & subs *************************************************

	float dwave = sinvec[wave&255];
	list<ship*> ships;
	gm.visible_ships(ships, player);
	for (list<ship*>::const_iterator it = ships.begin(); it != ships.end(); ++it) {
		if (!withplayer && *it == player) continue;	// only ships or subs playable!
		glPushMatrix();
		glTranslatef((*it)->get_pos().x, (*it)->get_pos().y, (*it)->get_pos().z);
		glRotatef(-(*it)->get_heading().value(), 0, 0, 1);
		glRotatef((3*dwave-3)/4.0,1,0,0);
		glRotatef((3*dwave-3)/4.0,0,1,0);
		(*it)->display();
		glPopMatrix();

		if ((*it)->has_smoke()) {
			double view_dir = 90.0f - angle ( (*it)->get_pos ().xy () - player->get_pos ().xy () ).value ();
			(*it)->smoke_display (view_dir);
		}
	}

	list<submarine*> submarines;
	gm.visible_submarines(submarines, player);
	for (list<submarine*>::const_iterator it = submarines.begin(); it != submarines.end(); ++it) {
		if (!withplayer && *it == player) continue; // only ships or subs playable!
		glPushMatrix();
		glTranslatef((*it)->get_pos().x, (*it)->get_pos().y, (*it)->get_pos().z);
		glRotatef(-(*it)->get_heading().value(), 0, 0, 1);
		if ((*it)->get_pos().z > -15) {
			glRotatef((3*dwave-3)/4.0,1,0,0);
			glRotatef((3*dwave-3)/4.0,0,1,0);
		}
		(*it)->display();
		glPopMatrix();
	}

	list<airplane*> airplanes;
	gm.visible_airplanes(airplanes, player);
	for (list<airplane*>::const_iterator it = airplanes.begin(); it != airplanes.end(); ++it) {
		glPushMatrix();
		glTranslatef((*it)->get_pos().x, (*it)->get_pos().y, (*it)->get_pos().z);
		glRotatef(-(*it)->get_heading().value(), 0, 0, 1);	// simulate pitch, roll etc.
		(*it)->display();
		glPopMatrix();
	}

	if (withunderwaterweapons) {
		list<torpedo*> torpedoes;
		gm.visible_torpedoes(torpedoes, player);
		for (list<torpedo*>::const_iterator it = torpedoes.begin(); it != torpedoes.end(); ++it) {
			glPushMatrix();
			glTranslatef((*it)->get_pos().x, (*it)->get_pos().y, (*it)->get_pos().z);
			glRotatef(-(*it)->get_heading().value(), 0, 0, 1);
			(*it)->display();
			glPopMatrix();
		}
		list<depth_charge*> depth_charges;
		gm.visible_depth_charges(depth_charges, player);
		for (list<depth_charge*>::const_iterator it = depth_charges.begin(); it != depth_charges.end(); ++it) {
			glPushMatrix();
			glTranslatef((*it)->get_pos().x, (*it)->get_pos().y, (*it)->get_pos().z);
			glRotatef(-(*it)->get_heading().value(), 0, 0, 1);
			(*it)->display();
			glPopMatrix();
		}
	}

	list<gun_shell*> gun_shells;
	gm.visible_gun_shells(gun_shells, player);
	for (list<gun_shell*>::const_iterator it = gun_shells.begin(); it != gun_shells.end(); ++it) {
		glPushMatrix();
		glTranslatef((*it)->get_pos().x, (*it)->get_pos().y, (*it)->get_pos().z);
		glRotatef(-(*it)->get_heading().value(), 0, 0, 1);
		glScalef(100,100,100);//fixme: to control functionality for now
		(*it)->display();
		glPopMatrix();
	}

	list<water_splash*> water_splashs;
	gm.visible_water_splashes ( water_splashs, player );
	for ( list<water_splash*>::const_iterator it = water_splashs.begin ();
		it != water_splashs.end (); it ++ )
	{
		double view_dir = 90.0f - angle ( (*it)->get_pos ().xy () - player->get_pos ().xy () ).value ();
		glPushMatrix ();
		glTranslatef ( (*it)->get_pos ().x, (*it)->get_pos ().y, (*it)->get_pos ().z );
		glRotatef ( view_dir, 0.0f, 0.0f, 1.0f );
		(*it)->display ();
		glPopMatrix ();
	}
	
	glColor3f(1,1,1);
}

bool user_interface::time_scale_up(void)
{
	if (time_scale < 4096) {
		time_scale *= 2;
		return true;
	}
	return false;
}

bool user_interface::time_scale_down(void)
{
	if (time_scale > 1) {
		time_scale /= 2;
		return true;
	}
	return false;
}

void user_interface::draw_infopanel(class system& sys, class game& gm) const
{
	glBindTexture(GL_TEXTURE_2D, panelbackgr->get_opengl_name());
	glColor3f ( 1.0f, 1.0f, 1.0f );
	glBegin(GL_QUADS);
	glTexCoord2i(0,0);
	glVertex2i(0,640);
	glTexCoord2i(0,1);
	glVertex2i(0,768);
	glTexCoord2i(8,1);
	glVertex2i(1024,768);
	glTexCoord2i(8,0);
	glVertex2i(1024,640);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
	ostringstream os;
	os << texts::get(1) << ": " << get_player()->get_heading().ui_value()
		<< "   " << texts::get(4) << ": "
		<< unsigned(fabs(round(sea_object::ms2kts(get_player()->get_speed()))))
		<< "   " << texts::get(5) << ": "
		<< unsigned(round(-get_player()->get_pos().z))
		<< "   " << texts::get(2) << ": "
		<< bearing.ui_value()
		<< "   " << texts::get(98) << ": "
		<< time_scale;
	font_panel->print(0, 648, os.str().c_str());
	int y = 768 - 24;
	for (list<string>::const_reverse_iterator it = panel_texts.rbegin(); 
             it != panel_texts.rend(); ++it) {
		font_panel->print(0, y, it->c_str());
		y -= 24;	// font_panel's height is 24.
	}
}


texture* user_interface::torptex(unsigned type)
{
	switch (type) {
		case torpedo::T1: return torpt1;
		case torpedo::T3: return torpt3;
		case torpedo::T5: return torpt5;
		case torpedo::T3FAT: return torpt3fat;
		case torpedo::T6LUT: return torpt6lut;
		case torpedo::T11: return torpt11;
	}
	return torpempty;
}

void user_interface::draw_gauge(class system& sys, class game& gm,
	unsigned nr, int x, int y, unsigned wh, angle a, const string& text, angle a2) const
{
	set_display_color ( gm );
	switch (nr) {
		case 1:	sys.draw_image(x, y, wh, wh, gauge1); break;
		case 2:	sys.draw_image(x, y, wh, wh, gauge2); break;
		case 3:	sys.draw_image(x, y, wh, wh, gauge3); break;
		case 4:	sys.draw_image(x, y, wh, wh, gauge4); break;
		default: return;
	}
	vector2 d = a.direction();
	int xx = x+wh/2, yy = y+wh/2;
	pair<unsigned, unsigned> twh = font_arial2->get_size(text.c_str());

	color font_color ( 255, 255, 255 );
	if ( !gm.is_day_mode () )
		font_color = color ( 255, 127, 127 );

	font_arial2->print(xx-twh.first/2, yy-twh.second/2, text.c_str(), font_color);
	glBindTexture(GL_TEXTURE_2D, 0);
	if (a2 != a) {
		vector2 d2 = a2.direction();
		glColor3f(0.2,0.8,1);
		glBegin(GL_LINES);
		glVertex2i(xx, yy);
		glVertex2i(xx + int(d2.x*wh*3/8),yy - int(d2.y*wh*3/8));
		glEnd();
	}
	glColor3f(1,0,0);
	glBegin(GL_TRIANGLES);
	glVertex2i(xx - int(d.y*4),yy - int(d.x*4));
	glVertex2i(xx + int(d.y*4),yy + int(d.x*4));
	glVertex2i(xx + int(d.x*wh*3/8),yy - int(d.y*wh*3/8));
	glEnd();
	glColor3f(1,1,1);
}

void user_interface::draw_clock(class system& sys, class game& gm,
	int x, int y, unsigned wh, double t, const string& text) const
{
	unsigned seconds = unsigned(fmod(t, 86400));
	unsigned minutes = seconds / 60;
	bool is_day_mode = gm.is_day_mode ();

	set_display_color ( gm );
	if (minutes < 12*60)
		sys.draw_image(x, y, wh, wh, clock12);
	else
		sys.draw_image(x, y, wh, wh, clock24);
	minutes %= 12*60;
	int xx = x+wh/2, yy = y+wh/2;
	pair<unsigned, unsigned> twh = font_arial2->get_size(text.c_str());

	color font_color ( 255, 255, 255 );
	if ( !is_day_mode )
		font_color = color ( 255, 127, 127 );

	font_arial2->print(xx-twh.first/2, yy-twh.second/2, text.c_str(), font_color);
	vector2 d;
	int l;

	glBindTexture(GL_TEXTURE_2D, 0);
	glBegin(GL_TRIANGLES);

	d = (angle(minutes * 360 / (12*60))).direction();
	l = wh/4;
	if ( is_day_mode )
		glColor3f(0,0,0.5);
	else
		glColor3f ( 0.5f, 0.0f, 0.5f );
	glVertex2i(xx - int(d.y*4),yy - int(d.x*4));
	glVertex2i(xx + int(d.y*4),yy + int(d.x*4));
	glVertex2i(xx + int(d.x*l),yy - int(d.y*l));

	d = (angle((minutes%60) * 360 / 60)).direction();
	l = wh*3/8;
	if ( is_day_mode )
		glColor3f(0,0,1);
	else
		glColor3f ( 0.5f, 0.0f, 1.0f );
	glVertex2i(xx - int(d.y*4),yy - int(d.x*4));
	glVertex2i(xx + int(d.y*4),yy + int(d.x*4));
	glVertex2i(xx + int(d.x*l),yy - int(d.y*l));

	d = (angle((seconds%60) * 360 / 60)).direction();
	l = wh*7/16;
	glColor3f(1,0,0);
	glVertex2i(xx - int(d.y*4),yy - int(d.x*4));
	glVertex2i(xx + int(d.y*4),yy + int(d.x*4));
	glVertex2i(xx + int(d.x*l),yy - int(d.y*l));

	glEnd();
	glColor3f(1,1,1);
}

void user_interface::draw_vessel_symbol(class system& sys,
	const vector2& offset, const sea_object* so, color c) const
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

void user_interface::draw_trail(sea_object* so, const vector2& offset)
{
	list<vector2> l = so->get_previous_positions();
	glColor4f(1,1,1,1);
	glBegin(GL_LINE_STRIP);
	vector2 p = (so->get_pos().xy() + offset)*mapzoom;
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

void user_interface::display_gauges(class system& sys, game& gm)
{
	sea_object* player = get_player ();
	sys.prepare_2d_drawing();
	set_display_color ( gm );
	for (int y = 0; y < 3; ++y)	// fixme: replace with gauges
		for (int x = 0; x < 4; ++x)
			sys.draw_image(x*256, y*256, 256, 256, psbackgr);
	angle player_speed = player->get_speed()*360.0/sea_object::kts2ms(36);
	angle player_depth = -player->get_pos().z;
	draw_gauge(sys, gm, 1, 0, 0, 256, player->get_heading(), texts::get(1),
		player->get_head_to());
	draw_gauge(sys, gm, 2, 256, 0, 256, player_speed, texts::get(4));
	draw_gauge(sys, gm, 4, 2*256, 0, 256, player_depth, texts::get(5));
	draw_clock(sys, gm, 3*256, 0, 256, gm.get_time(), texts::get(61));

	draw_infopanel(sys, gm);
	sys.unprepare_2d_drawing();

	// mouse handling
	int mx, my, mb = sys.get_mouse_buttons();
	sys.get_mouse_position(mx, my);

	if (mb & 1) {
		int marea = (my/256)*4+(mx/256);
		int mareax = (mx/256)*256+128;
		int mareay = (my/256)*256+128;
		angle mang(vector2(mx - mareax, mareay - my));
		if ( marea == 0 )
		{
			player->head_to_ang(mang, mang.is_cw_nearer(
				player->get_heading()));
		}
		else if ( marea == 1 )
		{}
		else if ( marea == 2 )
		{
			submarine* sub = dynamic_cast<submarine*> ( player );
			if ( sub )
			{
				sub->dive_to_depth(mang.ui_value());
			}
		}
	}

	// keyboard processing
	int key = sys.get_key();
	while (key != 0) {
		if (!keyboard_common(key, sys, gm)) {
			// specific keyboard processing
		}
		key = sys.get_key();
	}
}

void user_interface::display_bridge(class system& sys, game& gm)
{
	sea_object* player = get_player();
    
	glClear(GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	vector2 phd = player->get_heading().direction();
	vector3 viewpos = player->get_pos() + vector3(0, 0, 6) + phd.xy0();
	// no torpedoes, no DCs, with player
	draw_view(sys, gm, viewpos, player->get_heading()+bearing, true, false);

	sys.prepare_2d_drawing();
	draw_infopanel(sys, gm);
	sys.unprepare_2d_drawing();

	// keyboard processing
	int key = sys.get_key();
	while (key != 0) {
		if (!keyboard_common(key, sys, gm)) {
			// specific keyboard processing
			switch ( key )
			{
				// Zoom view
				case SDLK_y:
					zoom_scope = true;
					break;
			}
		}
		key = sys.get_key();
	}
}

void user_interface::draw_pings(class game& gm, const vector2& offset)
{
	// draw pings (just an experiment, you can hear pings, locate their direction
	//	a bit fuzzy but not their origin or exact shape).
	const list<game::ping>& pings = gm.get_pings();
	for (list<game::ping>::const_iterator it = pings.begin(); it != pings.end(); ++it) {
		const game::ping& p = *it;
		vector2 r = player_object->get_pos ().xy () - p.pos;
		vector2 p1 = (p.pos + offset)*mapzoom;
		vector2 p2 = p1 + (p.dir + p.pingAngle).direction() * p.range * mapzoom;
		vector2 p3 = p1 + (p.dir - p.pingAngle).direction() * p.range * mapzoom;
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

void user_interface::draw_sound_contact(class game& gm, const sea_object* player,
	double max_view_dist)
{
    // draw sound contacts
	list<ship*> ships;
	gm.sonar_ships(ships, player);
	for (list<ship*>::iterator it = ships.begin(); it != ships.end(); ++it) {
		vector2 ldir = (*it)->get_pos().xy() - player->get_pos().xy();
		ldir = ldir.normal() * 0.666666 * max_view_dist*mapzoom;
		if ((*it)->is_merchant())
			glColor3f(0,0,0);
		else if ((*it)->is_warship())
			glColor3f(0,0.5,0);
		else if ((*it)->is_escort())
			glColor3f(1,0,0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBegin(GL_LINES);
		glVertex2f(512,384);
		glVertex2f(512+ldir.x, 384-ldir.y);
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

void user_interface::draw_visual_contacts(class system& sys, class game& gm,
    const sea_object* player, const vector2& offset)
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
   		draw_vessel_symbol(sys, offset, *it, color(192,255,192));
   	for (list<submarine*>::iterator it = submarines.begin(); it != submarines.end(); ++it)
   		draw_vessel_symbol(sys, offset, *it, color(255,255,128));
   	for (list<airplane*>::iterator it = airplanes.begin(); it != airplanes.end(); ++it)
   		draw_vessel_symbol(sys, offset, *it, color(0,0,64));
   	for (list<torpedo*>::iterator it = torpedoes.begin(); it != torpedoes.end(); ++it)
   		draw_vessel_symbol(sys, offset, *it, color(255,0,0));
}

void user_interface::draw_square_mark ( class system& sys, class game& gm,
	const vector2& mark_pos, const vector2& offset, const color& c )
{
	c.set_gl_color ();
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

void user_interface::display_map(class system& sys, game& gm)
{
	sea_object* player = get_player ();
	bool is_day_mode = gm.is_day_mode ();

	if ( is_day_mode )
		glClearColor ( 0.0f, 0.0f, 1.0f, 1.0f );
	else
		glClearColor ( 0.0f, 0.0f, 0.75f, 1.0f );
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	double max_view_dist = gm.get_max_view_distance();

	vector2 offset = -player->get_pos().xy();

	sys.prepare_2d_drawing();

	float delta = MAPGRIDSIZE*mapzoom;
	float sx = myfmod(512, delta)-myfmod(-offset.x, MAPGRIDSIZE)*mapzoom;
	float sy = 768.0 - (myfmod(384.0, delta)-myfmod(-offset.y, MAPGRIDSIZE)*mapzoom);
	int lx = int(1024/delta)+2, ly = int(768/delta)+2;

	// draw grid
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
	glColor3f(1,1,1);

	// draw convoy positions	fixme: should be static and fade out after some time
	list<vector2> convoy_pos;
	gm.convoy_positions(convoy_pos);
	glBegin(GL_LINE_LOOP);
	for (list<vector2>::iterator it = convoy_pos.begin(); it != convoy_pos.end(); ++it) {
		draw_square_mark ( sys, gm, (*it), offset, color ( 0, 0, 0 ) );
	}
	glEnd();
	glColor3f(1,1,1);

	// draw view range
	glColor3f(1,0,0);
	float range = max_view_dist*mapzoom;
	glBegin(GL_LINE_LOOP);
	for (int i = 0; i < range/4; ++i) {
		float a = i*8*M_PI/range;
		glVertex2f(512+sin(a)*range, 384-cos(a)*range);
	}
	glEnd();
	glColor3f(1,1,1);

	// draw vessel symbols (or noise contacts)
	submarine* sub_player = dynamic_cast<submarine*> ( player );
	if (sub_player && sub_player->is_submerged ()) {
		// draw pings
		draw_pings(gm, offset);

		// draw sound contacts
		draw_sound_contact(gm, sub_player, max_view_dist);

		// draw player trails and player
		draw_trail(player, offset);
		draw_vessel_symbol(sys, offset, sub_player, color(255,255,128));

		// Special handling for submarine player: When the submarine is
		// on periscope depth and the periscope is up the visual contact
		// must be drawn on map.
		if ((sub_player->get_depth() <= sub_player->get_periscope_depth()) &&
			sub_player->is_scope_up())
		{
			draw_visual_contacts(sys, gm, sub_player, offset);

			// Draw a red box around the selected target.
			if ( target )
			{
				draw_square_mark ( sys, gm, target->get_pos ().xy (), offset,
					color ( 255, 0, 0 ) );
				glColor3f ( 1.0f, 1.0f, 1.0f );
			}
		}
	} 
	else	 	// enable drawing of all object as testing hack by commenting this, fixme
	{
		draw_visual_contacts(sys, gm, player, offset);

		// Draw a red box around the selected target.
		if ( target )
		{
			draw_square_mark ( sys, gm, target->get_pos ().xy (), offset,
				color ( 255, 0, 0 ) );
			glColor3f ( 1.0f, 1.0f, 1.0f );
		}
	}

	draw_infopanel(sys, gm);
	sys.unprepare_2d_drawing();

	// Mouse handling
	int mx, my, mb = sys.get_mouse_buttons();
	sys.get_mouse_position(mx, my);

	if ( mb & sys.left_button )
	{
		list<ship*> ships;
		gm.visible_ships(ships, player);
		list<submarine*> submarines;
		gm.visible_submarines(submarines, player);
	}

	// keyboard processing
	int key = sys.get_key();
	while (key != 0) {
		if (!keyboard_common(key, sys, gm)) {
			// specific keyboard processing
			switch(key) {
				case SDLK_PLUS : if (mapzoom < 1) mapzoom *= 1.5; break;
				case SDLK_MINUS : if (mapzoom > 0.01) mapzoom /= 1.5; break;
			}
		}
		key = sys.get_key();
	}
}

void user_interface::display_logbook(class system& sys, game& gm)
{
	// glClearColor ( 0.5f, 0.25f, 0.25f, 0 );
	glClearColor ( 0, 0, 0, 0 );
	glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	sys.prepare_2d_drawing ();
	captains_logbook->display ( sys, gm );
	draw_infopanel ( sys, gm );
	sys.unprepare_2d_drawing ();

	// mouse processing;
	int mx;
	int my;
	int mb = sys.get_mouse_buttons();
	sys.get_mouse_position(mx, my);
	if ( mb & sys.left_button )
		captains_logbook->check_mouse ( mx, my, mb );

	// keyboard processing
	int key = sys.get_key();
	while (key != 0) {
		if (!keyboard_common(key, sys, gm)) {
			// specific keyboard processing
			captains_logbook->check_key ( key, sys, gm );
		}
		key = sys.get_key();
	}
}

void user_interface::display_successes(class system& sys, game& gm)
{
	// glClearColor ( 0, 0, 0, 0 );
	// glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	sys.prepare_2d_drawing ();
	ships_sunk_disp->display ( sys, gm );
	draw_infopanel ( sys, gm );
	sys.unprepare_2d_drawing ();

	// keyboard processing
	int key = sys.get_key ();
	while ( key != 0 )
	{
		if ( !keyboard_common ( key, sys, gm ) )
		{
			// specific keyboard processing
			ships_sunk_disp->check_key ( key, sys, gm );
		}
		key = sys.get_key ();
	}
}

#ifdef OLD
void user_interface::display_successes(class system& sys, game& gm)
{
	glClearColor(0.25, 0.25, 0.25, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	sys.prepare_2d_drawing();
	font_arial2->print(0, 0, "success records - fixme");
	font_arial2->print(0, 100, "Ships sunk\n----------\n");
	unsigned ships = 0, tons = 0;
	for (list<unsigned>::const_iterator it = tonnage_sunk.begin(); it != tonnage_sunk.end(); ++it) {
		++ships;
		char tmp[20];
		sprintf(tmp, "%u BRT", *it);
		font_arial2->print(0, 100+(ships+2)*font_arial2->get_height(), tmp);
		tons += *it;
	}
	char tmp[40];
	sprintf(tmp, "total: %u BRT", tons);
	font_arial2->print(0, 100+(ships+4)*font_arial2->get_height(), tmp);
	sys.unprepare_2d_drawing();

	// keyboard processing
	int key = sys.get_key();
	while (key != 0) {
		if (!keyboard_common(key, sys, gm)) {
			// specific keyboard processing
		}
		key = sys.get_key();
	}
}
#endif // OLD

void user_interface::display_freeview(class system& sys, game& gm)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glRotatef(viewupang,1,0,0);
	glRotatef(viewsideang,0,1,0);
	float viewmatrix[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, viewmatrix);
	vector3 sidestep(viewmatrix[0], viewmatrix[4], viewmatrix[8]);
	vector3 upward(viewmatrix[1], viewmatrix[5], viewmatrix[9]);
	vector3 forward(viewmatrix[2], viewmatrix[6], viewmatrix[10]);
	glTranslatef(-viewpos.x, -viewpos.y, -viewpos.z);

	// draw everything
	draw_view(sys, gm, viewpos, 0, true, true);

	int mx, my;
	sys.get_mouse_motion(mx, my);
	viewsideang += mx*0.5;
	viewupang -= my*0.5;

	sys.prepare_2d_drawing();
	draw_infopanel(sys, gm);
	sys.unprepare_2d_drawing();

	// keyboard processing
	int key = sys.get_key();
	while (key != 0) {
		if (!keyboard_common(key, sys, gm)) {
			// specific keyboard processing
			switch(key) {
				case SDLK_w: viewpos -= forward * 5; break;
				case SDLK_x: viewpos += forward * 5; break;
				case SDLK_a: viewpos -= sidestep * 5; break;
				case SDLK_d: viewpos += sidestep * 5; break;
				case SDLK_q: viewpos -= upward * 5; break;
				case SDLK_e: viewpos += upward * 5; break;
			}
		}
		key = sys.get_key();
	}
}

void user_interface::add_message(const string& s)
{
	panel_texts.push_back(s);
	if (panel_texts.size() > 4)	// (128-8)/24-1 ;-)
		panel_texts.pop_front();
}

void user_interface::display_glasses(class system& sys, class game& gm)
{
	sea_object* player = get_player();

	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	unsigned res_x = sys.get_res_x(), res_y = sys.get_res_y();

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	sys.gl_perspective_fovx (5.0, 2.0/1.0, 2.0, gm.get_max_view_distance());
	glViewport(0, res_y/3, res_x, res_x/2);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	vector3 viewpos = player->get_pos() + vector3(0, 0, 6);
	// no torpedoes, no DCs, no player
	draw_view(sys, gm, viewpos, player->get_heading()+bearing, false, false);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glViewport(0, 0, res_x, res_y);
	glMatrixMode(GL_MODELVIEW);

	sys.prepare_2d_drawing();
	sys.draw_image(0, 0, 512, 512, glasses);
	sys.draw_hm_image(512, 0, 512, 512, glasses);
	draw_infopanel(sys, gm);
	sys.unprepare_2d_drawing();

	// keyboard processing
	int key = sys.get_key();
	while (key != 0) {
		if (!keyboard_common(key, sys, gm)) {
			// specific keyboard processing
			switch ( key )
			{
				case SDLK_y:
					zoom_scope = false;
					break;
			}
		}
		key = sys.get_key();
	}
}

void user_interface::add_rudder_message()
{
    switch (player_object->get_rudder())
    {
        case player_object->rudderfullleft:
            add_message(texts::get(35));
            break;
        case player_object->rudderleft:
            add_message(texts::get(33));
            break;
        case player_object->ruddermid:
            add_message(texts::get(42));
            break;
        case player_object->rudderright:
            add_message(texts::get(34));
            break;
        case player_object->rudderfullright:
            add_message(texts::get(36));
            break;
    }
}

#define DAY_MODE_COLOR() glColor3f ( 1.0f, 1.0f, 1.0f )

#define NIGHT_MODE_COLOR() glColor3f ( 1.0f, 0.4f, 0.4f )

void user_interface::set_display_color ( color_mode mode ) const
{
	switch ( mode )
	{
		case night_color_mode:
			NIGHT_MODE_COLOR ();
			break;
		default:
			DAY_MODE_COLOR ();
			break;
	}
}

void user_interface::set_display_color ( const class game& gm ) const
{
	if ( gm.is_day_mode () )
		DAY_MODE_COLOR ();
	else
		NIGHT_MODE_COLOR ();
}

sound* user_interface::get_sound_effect ( sound_effect se ) const
{
	sound* s = 0;

	switch ( se )
	{
		case se_submarine_torpedo_launch:
			s = torpedo_launch_sound;
			break;
		case se_torpedo_detonation:
			{
				submarine* sub = dynamic_cast<submarine*>( player_object );

				if ( sub && sub->is_submerged () )
				{
					double sid = rnd ( 2 );
					if ( sid < 1.0f )
						s = torpedo_detonation_submerged[0];
					else if ( sid < 2.0f )
						s = torpedo_detonation_submerged[1];
				}
				else
				{
					double sid = rnd ( 2 );
					if ( sid < 1.0f )
						s = torpedo_detonation_surfaced[0];
					else if ( sid < 2.0f )
						s = torpedo_detonation_surfaced[1];
				}
			}
			break;
	}

	return s;
}

void user_interface::play_sound_effect ( sound_effect se, double volume ) const
{
	sound* s = get_sound_effect ( se );

	if ( s )
		s->play ( volume );
}

void user_interface::play_sound_effect_distance ( sound_effect se, double distance ) const
{
	sound* s = get_sound_effect ( se );

	if ( s )
		s->play ( ( 1.0f - player_object->get_noise_factor () ) * exp ( - distance / 3000.0f ) );
}

void user_interface::add_captains_log_entry ( class game& gm, const string& s)
{
	date d;
	get_date ( gm.get_time (), d );

	if ( captains_logbook )
		captains_logbook->add_entry( d, s );
}

inline void user_interface::record_sunk_ship ( const ship* so )
{
	ships_sunk_disp->add_sunk_ship ( so );
}

void user_interface::draw_manometer_gauge ( class system& sys, class game& gm,
	unsigned nr, int x, int y, unsigned wh, float value, const string& text) const
{
	set_display_color ( gm );
	switch (nr)
	{
		case 1:
			sys.draw_image ( x, y, wh, wh / 2, gauge5 );
			break;
		default:
			return;
	}
	angle a ( 292.5f + 135.0f * value );
	vector2 d = a.direction ();
	int xx = x + wh / 2, yy = y + wh / 2;
	pair<unsigned, unsigned> twh = font_arial2->get_size(text.c_str());

	// Draw text.
	color font_color ( 0, 0, 0 );
	font_arial2->print ( xx - twh.first / 2, yy - twh.second / 2 - wh / 6,
		text.c_str(), font_color );

	// Draw pointer.
	glColor3f ( 0.0f, 0.0f, 0.0f );
	glBindTexture ( GL_TEXTURE_2D, 0 );
	glBegin ( GL_LINES );
	glVertex2i ( xx + int ( d.x * wh / 16 ), yy - int ( d.y * wh / 16 ) );
	glVertex2i ( xx + int ( d.x * wh * 3 / 8 ), yy - int ( d.y * wh * 3 / 8 ) );
	glEnd ();
	glColor3f ( 1.0f, 1.0f, 1.0f );
}
