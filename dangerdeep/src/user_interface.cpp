// user interface common code
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL/SDL.h>
#include <sstream>
#include "user_interface.h"
#include "system.h"
#include "game.h"
#include "texts.h"

#define SKYSEGS 16

vector<float> user_interface::allwaveheights;


user_interface::user_interface() :
        player_object(0), quit(false), pause(false), time_scale(1),
        zoom_scope(false), mapzoom(0.1), viewsideang(0), viewupang(-90),
    	viewpos(0, 0, 10), bearing(0), viewmode(4), target(0)
{
	if (allwaveheights.size() == 0) init_water_data();
}

user_interface::user_interface(sea_object* player) :
    	player_object ( player ), quit(false), pause(false), time_scale(1),
        zoom_scope(false), mapzoom(0.1), viewsideang(0), viewupang(-90),
    	viewpos(0, 0, 10), bearing(0), viewmode(4), target(0)
{
    	if (allwaveheights.size() == 0) init_water_data();
}

void user_interface::init_water_data(void)
{
	vector<float> dwave(WAVES);
	for (int i = 0; i < WAVES; ++i)
		dwave[i] = WAVETIDEHEIGHT*sin(i*2*M_PI/WAVES);
	vector<unsigned char> waterheight(WATERSIZE*WATERSIZE);
	for (int j = 0; j < WATERSIZE; ++j)
		for (int i = 0; i < WATERSIZE; ++i)
			waterheight[j*WATERSIZE+i] = rand()%256;
	allwaveheights.resize(WAVES*WATERSIZE*WATERSIZE);
	vector<float>::iterator it = allwaveheights.begin();
	for (int k = 0; k < WAVES; ++k) {
		for (int j = 0; j < WATERSIZE; ++j) {
			for (int i = 0; i < WATERSIZE; ++i) {
				*it++ = dwave[(int(waterheight[j*WATERSIZE+i])+k)%WAVES];
			}
		}
	}
}

inline float user_interface::get_waterheight(int x, int y, int wave)
{
	return allwaveheights[((wave&(WAVES-1))*WATERSIZE+(y&(WATERSIZE-1)))*WATERSIZE+(x&(WATERSIZE-1))];
}

float user_interface::get_waterheight(float x_, float y_, int wave)	// bilinear sampling
{
	float px = x_/WAVESIZE;
	float py = y_/WAVESIZE;
	int x = int(floor(px));
	int y = int(floor(py));
	float h0 = get_waterheight(x, y, wave);
	float h1 = get_waterheight(x+1, y, wave);
	float h2 = get_waterheight(x, y+1, wave);
	float h3 = get_waterheight(x+1, y+1, wave);
	float dx = (px - floor(px));
	float dy = (py - floor(py));
	float h01 = h0*(1-dx) + h1*dx;
	float h23 = h2*(1-dx) + h3*dx;
	return h01*(1-dy) + h23*dy;
}

void user_interface::draw_view(class system& sys, class game& gm, const vector3& viewpos,
	angle direction, bool withplayer, bool withunderwaterweapons)
{
	sea_object* player = get_player();
	int wave = int(fmod(gm.get_time(),86400)*WAVES/WAVETIDECYCLETIME);
	
	glRotatef(-90,1,0,0);
	glRotatef(direction.value(),0,0,1);
	glTranslatef(-viewpos.x, -viewpos.y, -viewpos.z);
	
	double max_view_dist = gm.get_max_view_distance();

	// ************ sky ***************************************************************
	glDisable(GL_LIGHTING);
	glPushMatrix();
	glTranslatef(viewpos.x, viewpos.y, 0);
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
	skycol2 = color(color(0, 0, 16), color(24,47,244), colscal);

	skyhemisphere->display(false, &skycol1, &skycol2);
	glBindTexture(GL_TEXTURE_2D, clouds->get_opengl_name());
	float skysin[SKYSEGS], skycos[SKYSEGS];
	for (int i = 0; i < SKYSEGS; ++i) {
		float t = i*2*M_PI/SKYSEGS;
		skycos[i] = cos(t);
		skysin[i] = sin(t);
	}
	double cloudrot = fmod(gm.get_time(), 60)/60;
	glBegin(GL_QUADS);	// fixme: quad strips!
	float rl = 0.95, ru = 0.91;
	float hl = 0.1, hu = 0.4;
	for (int j = 0; j < SKYSEGS; ++j) {
		int t = (j+1) % SKYSEGS;
		lightcol.set_gl_color(0);
		glTexCoord2f((j+1)*0.5+cloudrot, 0);
		glVertex3f(rl * skycos[t], rl * skysin[t], hl);
		glTexCoord2f((j  )*0.5+cloudrot, 0);
		glVertex3f(rl * skycos[j], rl * skysin[j], hl);
		lightcol.set_gl_color(255);
		glTexCoord2f((j  )*0.5+cloudrot, 1);
		glVertex3f(ru * skycos[j], ru * skysin[j], hu);
		glTexCoord2f((j+1)*0.5+cloudrot, 1);
		glVertex3f(ru * skycos[t], ru * skysin[t], hu);
	}
	glEnd();
	glPopMatrix();
	glEnable(GL_LIGHTING);

	// ************ water *************************************************************
	glDisable(GL_LIGHTING);
	glPushMatrix();
	int wx = int(floor(viewpos.x/WAVESIZE)) & (WATERSIZE-1);
	int wy = int(floor(viewpos.y/WAVESIZE)) & (WATERSIZE-1);
	glTranslatef(ceil(viewpos.x/WAVESIZE)*WAVESIZE-WATERRANGE, ceil(viewpos.y/WAVESIZE)*WAVESIZE-WATERRANGE, 0);

	float wd = (wave%(8*WAVES))/float(8*WAVES);
	float t0 = wd;
	float t1 = wd + (max_view_dist - WATERRANGE)/64;
	float t2 = wd + (max_view_dist + WATERRANGE)/64;
	float t3 = wd + 2*max_view_dist/64;
	float c0 = -max_view_dist+WATERRANGE;
	float c1 = 0;
	float c2 = 2*WATERRANGE;
	float c3 = max_view_dist+WATERRANGE;

	// fixme: glclearcolor depends on daytime, too

	// color of water depends on daytime
	lightcol.set_gl_color();

	// fixme: with swimming the missing anisotropic filtering causes
	// the water to shine unnatural. a special distant_water texture doesn't help
	// just looks worse
	// fixme: while moving the distant water texture coordinates jump wildly.
	// we have to adjust its texture coordinates by remainder of viewpos.xy/WAVESIZE
	// fixme use multitexturing for distant water with various moving around the
	// texture for more realism?
	
	glBindTexture(GL_TEXTURE_2D, water->get_opengl_name());
	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2f(t0,t3);
	glVertex3f(c0,c3,0);
	glTexCoord2f(t1,t2);
	glVertex3f(c1,c2,0);
	glTexCoord2f(t3,t3);
	glVertex3f(c3,c3,0);
	glTexCoord2f(t2,t2);
	glVertex3f(c2,c2,0);
	glTexCoord2f(t3,t0);
	glVertex3f(c3,c0,0);
	glTexCoord2f(t2,t1);
	glVertex3f(c2,c1,0);
	glTexCoord2f(t0,t0);
	glVertex3f(c0,c0,0);
	glTexCoord2f(t1,t1);
	glVertex3f(c1,c1,0);
	glTexCoord2f(t0,t3);
	glVertex3f(c0,c3,0);
	glTexCoord2f(t1,t2);
	glVertex3f(c1,c2,0);
	glEnd();
	
	//fixme waterheight of äußerstem rand des allwaveheight-gemachten wassers auf 0
	//damit keine lücken zu obigem wasser da sind SCHNELLER machen
	//fixme vertex lists
	//fixme visibility detection: 75% of the water are never seen but drawn

	glBindTexture(GL_TEXTURE_2D, water->get_opengl_name());
	glBegin(GL_QUADS);
	int y = wy;
	for (int j = 0; j < WATERSIZE; ++j) {
		int x = wx;
		int j2 = y%4;
		float j3 = j*WAVESIZE;
		for (int i = 0; i < WATERSIZE; ++i) {
			int i2 = x%4;
			float i3 = i*WAVESIZE;
			// fixme vertex lists
			glTexCoord2f(i2/4.0, j2/4.0);
			glVertex3f(i3,j3,(i == 0 || j == 0) ? 0 : get_waterheight(x, y, wave));
			glTexCoord2f((i2+1)/4.0, j2/4.0);
			glVertex3f(i3+WAVESIZE,j3,(i == WATERSIZE-1 || j == 0) ? 0 : get_waterheight(x+1, y, wave));
			glTexCoord2f((i2+1)/4.0, (j2+1)/4.0);
			glVertex3f(i3+WAVESIZE,j3+WAVESIZE,(i == WATERSIZE-1 || j == WATERSIZE-1) ? 0 : get_waterheight(x+1, y+1, wave));
			glTexCoord2f(i2/4.0, (j2+1)/4.0);
			glVertex3f(i3,j3+WAVESIZE,(i == 0 || j == WATERSIZE-1) ? 0 : get_waterheight(x, y+1, wave));
			x = (x + 1) & (WATERSIZE-1);
		}
		y = (y + 1) & (WATERSIZE-1);
	}
	glEnd();
	glPopMatrix();
	glEnable(GL_LIGHTING);
//	glColor3f(1,1,1);

	// ******************** ships & subs *************************************************

	float dwave = sin((wave%WAVES)*2*M_PI/WAVES);
	list<ship*> ships = gm.visible_ships(player->get_pos());
	for (list<ship*>::const_iterator it = ships.begin(); it != ships.end(); ++it) {
		if (!withplayer && *it == player) continue;	// only ships or subs playable!
		glPushMatrix();
		glTranslatef((*it)->get_pos().x, (*it)->get_pos().y, (*it)->get_pos().z);
		glRotatef(-(*it)->get_heading().value(), 0, 0, 1);
		glRotatef((3*dwave-3)/4.0,1,0,0);
		glRotatef((3*dwave-3)/4.0,0,1,0);
		(*it)->display();
		glPopMatrix();
	}

	list<submarine*> submarines = gm.visible_submarines(player->get_pos());
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

	list<airplane*> airplanes = gm.visible_airplanes(player->get_pos());
	for (list<airplane*>::const_iterator it = airplanes.begin(); it != airplanes.end(); ++it) {
		glPushMatrix();
		glTranslatef((*it)->get_pos().x, (*it)->get_pos().y, (*it)->get_pos().z);
		glRotatef(-(*it)->get_heading().value(), 0, 0, 1);	// simulate pitch, roll etc.
		(*it)->display();
		glPopMatrix();
	}

	if (withunderwaterweapons) {
		list<torpedo*> torpedoes = gm.visible_torpedoes(player->get_pos());
		for (list<torpedo*>::const_iterator it = torpedoes.begin(); it != torpedoes.end(); ++it) {
			glPushMatrix();
			glTranslatef((*it)->get_pos().x, (*it)->get_pos().y, (*it)->get_pos().z);
			glRotatef(-(*it)->get_heading().value(), 0, 0, 1);
			(*it)->display();
			glPopMatrix();
		}
		list<depth_charge*> depth_charges = gm.visible_depth_charges(player->get_pos());
		for (list<depth_charge*>::const_iterator it = depth_charges.begin(); it != depth_charges.end(); ++it) {
			glPushMatrix();
			glTranslatef((*it)->get_pos().x, (*it)->get_pos().y, (*it)->get_pos().z);
			glRotatef(-(*it)->get_heading().value(), 0, 0, 1);
			(*it)->display();
			glPopMatrix();
		}
	}

	list<gun_shell*> gun_shells = gm.visible_gun_shells(player->get_pos());
	for (list<gun_shell*>::const_iterator it = gun_shells.begin(); it != gun_shells.end(); ++it) {
		glPushMatrix();
		glTranslatef((*it)->get_pos().x, (*it)->get_pos().y, (*it)->get_pos().z);
		glRotatef(-(*it)->get_heading().value(), 0, 0, 1);
	glScalef(100,100,100);//fixme: to control functionality for now
		(*it)->display();
		glPopMatrix();
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

void user_interface::draw_infopanel(class system& sys) const
{
	glBindTexture(GL_TEXTURE_2D, panelbackgr->get_opengl_name());
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
	os << TXT_Heading[language] << ": " << get_player()->get_heading().ui_value()
		<< "   " << TXT_Speed[language] << ": "
		<< unsigned(fabs(round(sea_object::ms2kts(get_player()->get_speed()))))
		<< "   " << TXT_Depth[language] << ": "
		<< unsigned(round(-get_player()->get_pos().z))
		<< "   " << TXT_Bearing[language] << ": "
		<< bearing.ui_value();
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

void user_interface::draw_gauge(class system& sys, unsigned nr, int x, int y,
	unsigned wh, angle a, const char* text) const
{
	switch (nr) {
		case 1:	sys.draw_image(x, y, wh, wh, gauge1); break;
		case 2:	sys.draw_image(x, y, wh, wh, gauge2); break;
		case 3:	sys.draw_image(x, y, wh, wh, gauge3); break;
		case 4:	sys.draw_image(x, y, wh, wh, gauge4); break;
		default: return;
	}
	vector2 d = a.direction();
	int xx = x+wh/2, yy = y+wh/2;
	pair<unsigned, unsigned> twh = font_arial2->get_size(text);
	font_arial2->print(xx-twh.first/2, yy-twh.second/2, text);
	glColor3f(1,0,0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBegin(GL_TRIANGLES);
	glVertex2i(xx - int(d.y*4),yy - int(d.x*4));
	glVertex2i(xx + int(d.y*4),yy + int(d.x*4));
	glVertex2i(xx + int(d.x*wh*3/8),yy - int(d.y*wh*3/8));
	glEnd();
	glColor3f(1,1,1);
}

void user_interface::draw_clock(class system& sys, int x, int y, unsigned wh, double t,
	const char* text) const
{
	unsigned seconds = unsigned(fmod(t, 86400));
	unsigned minutes = seconds / 60;
	if (minutes < 12*60)
		sys.draw_image(x, y, wh, wh, clock12);
	else
		sys.draw_image(x, y, wh, wh, clock24);
	minutes %= 12*60;
	int xx = x+wh/2, yy = y+wh/2;
	pair<unsigned, unsigned> twh = font_arial2->get_size(text);
	font_arial2->print(xx-twh.first/2, yy-twh.second/2, text);
	vector2 d;
	int l;

	glBindTexture(GL_TEXTURE_2D, 0);
	glBegin(GL_TRIANGLES);

	d = (angle(minutes * 360 / (12*60))).direction();
	l = wh/4;
	glColor3f(0,0,0.5);
	glVertex2i(xx - int(d.y*4),yy - int(d.x*4));
	glVertex2i(xx + int(d.y*4),yy + int(d.x*4));
	glVertex2i(xx + int(d.x*l),yy - int(d.y*l));

	d = (angle((minutes%60) * 360 / 60)).direction();
	l = wh*3/8;
	glColor3f(0,0,1);
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
	for (int y = 0; y < 3; ++y)	// fixme: replace with gauges
		for (int x = 0; x < 4; ++x)
			sys.draw_image(x*256, y*256, 256, 256, psbackgr);
	angle player_speed = player->get_speed()*360.0/sea_object::kts2ms(36);
	angle player_depth = -player->get_pos().z;
	draw_gauge(sys, 1, 0, 0, 256, player->get_heading(), TXT_Heading[language]);
	draw_gauge(sys, 2, 256, 0, 256, player_speed, TXT_Speed[language]);
	draw_gauge(sys, 4, 2*256, 0, 256, player_depth, TXT_Depth[language]);
	draw_clock(sys, 3*256, 0, 256, gm.get_time(), TXT_Time[language]);

	draw_infopanel(sys);
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
            if ( player->get_submarine_ptr() )
            {
                player->get_submarine_ptr()->dive_to_depth(mang.ui_value());
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
	draw_infopanel(sys);
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

void user_interface::draw_pings(class game& gm, const vector2& offset)
{
  	// draw pings (just an experiment, you can hear pings, locate their direction
  	//	a bit fuzzy but not their origin or exact shape).
  	const list<game::ping>& pings = gm.get_pings();
  	for (list<game::ping>::const_iterator it = pings.begin(); it != pings.end(); ++it) {
  		const game::ping& p = *it;
  		vector2 p1 = (p.pos + offset)*mapzoom;
  		vector2 p2 = p1 + (p.dir + angle(PINGANGLE/2)).direction() * PINGLENGTH * mapzoom;
  		vector2 p3 = p1 + (p.dir - angle(PINGANGLE/2)).direction() * PINGLENGTH * mapzoom;
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
    const double& max_view_dist)
{
    // draw sound contacts
   	if (player->get_throttle_speed()/player->get_max_speed() < 0.51) {
   		list<ship*> ships = gm.hearable_ships(player->get_pos());
   		// fixme: sub's missing
   		// fixme: differences between ship types missing
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
   	}
}

void user_interface::draw_visual_contacts(class system& sys, class game& gm,
    const sea_object* player, const vector2& offset)
{
    // draw vessel trails and symbols (since player is submerged, he is drawn too)
   	list<ship*> ships = gm.visible_ships(player->get_pos());
   	list<submarine*> submarines = gm.visible_submarines(player->get_pos());
   	list<airplane*> airplanes = gm.visible_airplanes(player->get_pos());
   	list<torpedo*> torpedoes = gm.visible_torpedoes(player->get_pos());

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

void user_interface::display_map(class system& sys, game& gm)
{
    sea_object* player = get_player ();
    
	glClearColor(0, 0, 1, 1);	// fixme
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
	list<vector2> convoy_pos = gm.convoy_positions();
	glColor3f(0,0,0);
	glBegin(GL_LINE_LOOP);
	for (list<vector2>::iterator it = convoy_pos.begin(); it != convoy_pos.end(); ++it) {
		vector2 p = (*it + offset)*mapzoom;
		int x = int(round(p.x));
		int y = int(round(p.y));
		glVertex2i(512-4+x,384-4-y);
		glVertex2i(512+4+x,384-4-y);
		glVertex2i(512+4+x,384+4-y);
		glVertex2i(512-4+x,384+4-y);
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
    submarine* sub_player = player->get_submarine_ptr();
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
        }
	} 
	else	 	// enable drawing of all object as testing hack by commenting this, fixme
	{	// sub is surfaced
        draw_visual_contacts(sys, gm, player, offset);
	}
	
	draw_infopanel(sys);
	sys.unprepare_2d_drawing();
	
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

void user_interface::display_damagecontrol(class system& sys, game& gm)
{
	glClearColor(0.25, 0.25, 0.25, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	sys.prepare_2d_drawing();
	font_arial2->print(0, 0, "damage control - fixme");
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

void user_interface::display_logbook(class system& sys, game& gm)
{
	glClearColor(0.25, 0.25, 0.25, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	sys.prepare_2d_drawing();
	font_arial2->print(0, 0, "logbook - fixme");
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
	draw_infopanel(sys);
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
