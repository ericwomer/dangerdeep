// SDL/OpenGL based system services
// (C)+(W) by Thorsten Jordan. See LICENSE
// Parts taken and adapted from Martin Butterwecks's OOML SDL code (wws.sourceforget.net/projects/ooml/)

#include <iostream>
#include <sstream>
#include <cmath>
using namespace std;
#define GL_GLEXT_LEGACY
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL.h>
#include <SDL_image.h>
#define M_PI 3.1415926535897932	// should be in math.h, but not for Windows. *sigh*
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#endif
#include "system.h"

class system* system::instance = 0;

system::system(double nearz_, double farz_, unsigned res, bool fullscreen) :
	res_x(res), res_y(res*3/4), nearz(nearz_), farz(farz_), is_fullscreen(fullscreen),
	show_console(false), console_font(0), console_background(0),
	draw_2d(false), keyqueue(), keymod(0), mouse_xrel(0), mouse_yrel(0), mouse_x(res/2),
	mouse_y(res*3/8), mouse_b(0), time_passed_while_sleeping(0), sleep_time(0),
	is_sleeping(false), screenshot_nr(0)
{
	myassert(res==640||res==800||res==1024||res==1280||res==512);
	myassert(!instance);

	res_x_2d = res_x;
	res_y_2d = res_y;

	keys.resize(SDLK_LAST - SDLK_FIRST + 1);
	
	int err = SDL_Init(SDL_INIT_VIDEO);
	myassert(err>=0, "SDL Video init failed");
	const SDL_VideoInfo *videoInfo = SDL_GetVideoInfo();
	myassert(videoInfo != 0, "Video info query failed");
	int videoFlags  = SDL_OPENGLBLIT | SDL_GL_DOUBLEBUFFER | SDL_HWPALETTE;
	videoFlags |= (videoInfo->hw_available) ? SDL_HWSURFACE : SDL_SWSURFACE;
	if (fullscreen)
		videoFlags |= SDL_FULLSCREEN;
	if (videoInfo->blit_hw)
		videoFlags |= SDL_HWACCEL;
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	int bpp = videoInfo->vfmt->BitsPerPixel;
	SDL_Surface* screen = SDL_SetVideoMode(res_x, res_y, bpp, videoFlags);
	myassert(screen != 0, "Video mode set failed");

	SDL_EventState(SDL_VIDEORESIZE, SDL_IGNORE);//fixme
	SDL_EventState(SDL_SYSWMEVENT, SDL_IGNORE);//fixme
	SDL_JoystickEventState(SDL_IGNORE);
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,SDL_DEFAULT_REPEAT_INTERVAL);//fixme
	SDL_ShowCursor(SDL_ENABLE);//fixme

	// OpenGL Init.
	glClearColor (32/255.0, 64/255.0, 192/255.0, 1.0);
	glClearDepth(1.0);
	glDepthFunc(GL_LEQUAL);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
	glEnable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glEnable(GL_COLOR_MATERIAL);
	glColor3f(1, 1, 1);
	
	// Screen resize
	screen_resize(res_x, res_y, nearz, farz);

	// check for some OpenGL Extensions
#ifdef INFO
	char* glexts = (char*)glGetString(GL_EXTENSIONS);
	cout << "Supported OpenGL extensions : " << glexts << "\n";
#endif

	instance = this;
}

system::~system()
{
	myassert(instance != 0);
	SDL_Quit();
	instance = 0;
}

void system::screen_resize(unsigned w, unsigned h, double nearz, double farz)
{
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gl_perspective_fovx (90.0, (GLdouble)w/(GLdouble)h, nearz, farz);
	float m[16];
	glGetFloatv(GL_PROJECTION_MATRIX, m);
	xscal_2d = 2*nearz/m[0];
	yscal_2d = 2*nearz/m[5];
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void system::clear_console(void)
{
	console_text.clear();
	add_console("$ffffffLog restart.");
}

void system::add_console(const string& tx)
{
	console_text.push_back(tx);
}

void system::draw_console_with(const font* fnt, const texture* background)
{
	console_font = fnt;
	console_background = background;
}

void system::draw_console(void)
{
	prepare_2d_drawing();
	if (console_background) {
		glColor4f(1,1,1,0.75);
		glBindTexture(GL_TEXTURE_2D, console_background->get_opengl_name());
		glBegin(GL_QUADS);
		glTexCoord2f(0,0);
		glVertex2i(0,0);
		glTexCoord2f(0,2);
		glVertex2i(0,res_y_2d/2);
		glTexCoord2f(4,2);
		glVertex2i(res_x_2d,res_y_2d/2);
		glTexCoord2f(4,0);
		glVertex2i(res_x_2d,0);
		glEnd();
	}
	glColor4f(1,1,1,1);
	
	unsigned fh = console_font->get_height();
	unsigned lines = res_y/(2*fh)-2;
	unsigned s = console_text.size();
	list<string>::const_iterator it = console_text.begin();
	while (s-- > lines) ++it;
	unsigned y = fh;
	for ( ; it != console_text.end(); ++it, y+=fh)
		console_font->print(fh, y, *it);
	unprepare_2d_drawing();
}

void system::write_console(bool fileonly) const
{
	FILE* f = fopen("console_log.txt", "wb");
	if (!f) return;
	const char* rtn = "\n";
	for (list<string>::const_iterator it = console_text.begin(); it != console_text.end(); ++it) {
		if (!fileonly)
			cerr << "console log: " << *it << "\n";
		fwrite(&((*it)[0]), it->length(), 1, f);
		fwrite(rtn, strlen(rtn), 1, f);
	}
	fclose(f);	
}

void system::prepare_2d_drawing()
{
	myassert(draw_2d == false);
	glFlush();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, res_x_2d, 0, res_y_2d);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(0, res_y_2d, 0);
	glScalef(1, -1, 1);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	draw_2d = true;
}

void system::unprepare_2d_drawing()
{
	myassert(draw_2d == true);
	glFlush();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	draw_2d = false;
}

unsigned long system::millisec(void)
{
	return SDL_GetTicks() - time_passed_while_sleeping;
}

void system::myassert(bool cond, const string& msg)
{
	if (this == 0) return;	// avoid call with no instance
	if (!cond) {
		add_console("!ERROR!");
		if (msg != "")
			add_console(msg);
		else
			add_console("unknown error");
		SDL_Quit();
		write_console();
		exit(0);
	}
}

void system::swap_buffers(void)
{
	if (show_console) {
		draw_console();
	}
	SDL_GL_SwapBuffers();
}

void system::poll_event_queue(void)
{
	SDL_Event event;
	int keycode = 0;
	do {
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT:			// Quit event
					SDL_Quit();	// to clean up mouse etc. after kill
					exit(0);	// fixme
				
				case SDL_ACTIVEEVENT:		// Application activation or focus event
					if (event.active.gain==0) {
						is_sleeping = true;
						sleep_time = SDL_GetTicks();
					} else {
						is_sleeping = false;
						time_passed_while_sleeping += SDL_GetTicks() - sleep_time;
					}
					break;
				
				case SDL_KEYDOWN:		// Keyboard event - key down
					keycode = event.key.keysym.sym;
					if (keycode == SDLK_LSHIFT || keycode == SDLK_RSHIFT) {
						keymod |= 1;
					} else {
						keyqueue.push(keycode);
					}
					keys[keycode - SDLK_FIRST] = true;
					break;
				
				case SDL_KEYUP:			// Keyboard event - key up
					keycode = event.key.keysym.sym;
					if (keycode == SDLK_LSHIFT || keycode == SDLK_RSHIFT) {
						keymod &= ~1;
					}
					keys[keycode - SDLK_FIRST] = false;
					break;
		
				case SDL_MOUSEMOTION:		// Mouse motion event
					mouse_xrel += event.motion.xrel;
					mouse_yrel += event.motion.yrel;
					mouse_x = event.motion.x;
					mouse_y = event.motion.y;
					break;
				
				case SDL_MOUSEBUTTONDOWN:	// Mouse button event - button down
					switch (event.button.button)
					{
						case SDL_BUTTON_LEFT:
							mouse_b |= left_button;
							break;
						case SDL_BUTTON_MIDDLE:
							mouse_b |= middle_button;
							break;
						case SDL_BUTTON_RIGHT:
							mouse_b |= right_button;
							break;
					}
					break;
				
				case SDL_MOUSEBUTTONUP:		// Mouse button event - button up
					switch (event.button.button)
					{
						case SDL_BUTTON_LEFT:
							mouse_b &= ~left_button;
							break;
						case SDL_BUTTON_MIDDLE:
							mouse_b &= ~middle_button;
							break;
						case SDL_BUTTON_RIGHT:
							mouse_b &= ~right_button;
							break;
					}
					break;
					
				default:			// Should NEVER happen !
					add_console("unknown event caught");
					break; // quick hack
					myassert(0, "Unknown Event !");
					break;
			}
		}
	} while (is_sleeping);
}

void system::get_mouse_motion(int &x, int &y)
{
	x = mouse_xrel * int(res_x_2d) / int(res_x);
	y = mouse_yrel * int(res_y_2d) / int(res_y);
	mouse_xrel = mouse_yrel = 0;
}

void system::get_mouse_position(int &x, int &y)
{
	x = mouse_x * int(res_x_2d) / int(res_x);
	y = mouse_y * int(res_y_2d) / int(res_y);
}

int system::get_key(void)
{
	if (keyqueue.empty())
		return 0;
	int result = keyqueue.front();
	keyqueue.pop();
	if (result == SDLK_TAB)
		show_console = !show_console;
	return result;
}

bool system::is_key_down(int code) const
{
	if (SDLK_FIRST <= code && code <= SDLK_LAST)
		return keys[code - SDLK_FIRST];
	return false;
}

int system::getch(void)
{
	do {
		poll_event_queue();	// to avoid the app to run dead
	} while (keyqueue.empty());
	return get_key();
}

void system::screenshot(void)
{
//fixme: use SDL's save bmp feature
	vector<unsigned char> pic(res_x*res_y*3);
	glReadPixels(0, 0, res_x, res_y, GL_RGB, GL_UNSIGNED_BYTE, &(pic[0]));
	ostringstream os;
	os << "screenshot" << screenshot_nr++ << ".ppm";
	FILE* f = fopen(os.str().c_str(), "wb");
	fprintf(f, "P6\n%u %u\n255\n", res_x, res_y);
	unsigned p = res_x * res_y * 3;
	for (unsigned y = 0; y < res_y; ++y) {
		p -= res_x * 3;
		unsigned q = p;
		for (unsigned x = 0; x < res_x; ++x) {
			fwrite(&(pic[q]), 3, 1, f);
			q += 3;
		}
	}
	fclose(f);
	add_console(string("screenshot taken as ")+os.str());
}

void system::draw_rectangle(int x, int y, int w, int h)
{
/*
//doesn't work. why? fixme
	glRecti(x, y, x+w, y+h);
*/	
	glBegin(GL_QUADS);
	glVertex2i(x, y);
	glVertex2i(x, y+h);
	glVertex2i(x+w, y+h);
	glVertex2i(x+w, y);
	glEnd();
}

void system::gl_perspective_fovx(double fovx, double aspect, double znear, double zfar)
{
	double tanfovx2 = tan(M_PI*fovx/360.0);
	double tanfovy2 = tanfovx2 / aspect;
	double r = znear * tanfovx2;
	double t = znear * tanfovy2;
	glFrustum(-r, r, -t, t, znear, zfar);
}
