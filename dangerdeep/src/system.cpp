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

// SDL/OpenGL based system services
// (C)+(W) by Thorsten Jordan. See LICENSE
// Parts taken and adapted from Martin Butterwecks's OOML SDL code (wws.sourceforget.net/projects/ooml/)

#include <iostream>
#include <sstream>
#include <cmath>
#include <cstdarg>
#include <stdexcept>
using namespace std;

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "oglext/OglExt.h"
#include <SDL.h>
#include <SDL_image.h>
#include <glu.h>

#include "system.h"
#include "texture.h"
#include "font.h"

/*
standard exceptions:

exception
    logic_error
        domain_error
        invalid_argument
        length_error
        out_of_range
    runtime_error
        range_error
        overflow_error
        underflow_error
bad_alloc
bad_cast
bad_exception
bad_typeid

*/

//#define THROWERR

#ifdef THROWERR
#include <exception>
#endif

class system* system::instance = 0;

static char sprintf_tmp[1024];

system::system(double nearz_, double farz_, unsigned res_x_, unsigned res_y_, bool fullscreen) :
	res_x(res_x_), res_y(res_y_), nearz(nearz_), farz(farz_), is_fullscreen(fullscreen),
	show_console(false), console_font(0), console_background(0),
	draw_2d(false), time_passed_while_sleeping(0), sleep_time(0),
	is_sleeping(false), maxfps(0), last_swap_time(0), screenshot_nr(0)
{
	if (instance)
		throw runtime_error("system construction: system instance already exists");

	int err = SDL_Init(SDL_INIT_VIDEO);
	if (err < 0)
		throw sdl_error("video init failed");

	// request available SDL video modes
	SDL_Rect** modes = SDL_ListModes(NULL, SDL_FULLSCREEN|SDL_HWSURFACE);
	for (unsigned i = 0; modes[i]; ++i) {
		available_resolutions.push_back(vector2i(modes[i]->w, modes[i]->h));
	}

	res_x_2d = res_x;
	res_y_2d = res_y;

	try {
		set_video_mode(res_x, res_y, is_fullscreen);
	}
	catch (...) {
		SDL_Quit();
		throw;
	}

	SDL_EventState(SDL_VIDEORESIZE, SDL_IGNORE);//fixme
	SDL_EventState(SDL_SYSWMEVENT, SDL_IGNORE);//fixme
	SDL_JoystickEventState(SDL_IGNORE);
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,SDL_DEFAULT_REPEAT_INTERVAL);//fixme
	SDL_ShowCursor(SDL_ENABLE);//fixme
	SDL_EnableUNICODE(1);

	// OpenGL Init.
	glClearColor (32/255.0, 64/255.0, 192/255.0, 1.0);
	glClearDepth(1.0);
	glDepthFunc(GL_LEQUAL);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_NORMALIZE);	// fixme!!!! why was that turned on?! we don't need it! it doesn't hurt much though...
	glEnable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	glColor3f(1, 1, 1);
	// set up some things for drawing pixels
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glReadBuffer(GL_BACK);
	glDrawBuffer(GL_BACK);
	
	// Screen resize
	screen_resize(res_x, res_y, nearz, farz);

	// check for some OpenGL Extensions
	const char* vendorc = (const char*)glGetString(GL_VENDOR);
	const char* rendererc = (const char*)glGetString(GL_RENDERER);
	const char* versionc = (const char*)glGetString(GL_VERSION);
	const char* extensionsc = (const char*)glGetString(GL_EXTENSIONS);
	string vendor = vendorc ? vendorc : "???";
	string renderer = rendererc ? rendererc : "???";
	string version = versionc ? versionc : "???";
	string extensions = extensionsc ? extensionsc : "???";
	if (extensionsc) {
		unsigned spos = 0;
		while (spos < extensions.length()) {
			string::size_type pos = extensions.find(" ", spos);
			if (pos == string::npos) {
				supported_extensions.insert(extensions.substr(spos));
				spos = extensions.length();
			} else {
				extensions.replace(pos, 1, "\n");
				supported_extensions.insert(extensions.substr(spos, pos-spos));
				spos = pos+1;
			}
		}
	}
	GLint nrtexunits = 0, nrlights = 0, nrclipplanes = 0, depthbits = 0;
	GLint maxviewportdims[2] = { 0, 0 };
	glGetIntegerv(GL_MAX_TEXTURE_UNITS, &nrtexunits);
	glGetIntegerv(GL_MAX_LIGHTS, &nrlights);
	glGetIntegerv(GL_MAX_CLIP_PLANES, &nrclipplanes);
	glGetIntegerv(GL_MAX_VIEWPORT_DIMS, maxviewportdims);
	glGetIntegerv(GL_DEPTH_BITS, &depthbits);

	cerr << "OpenGL vendor : " << vendor << "\n"
	     << "GL renderer : " << renderer << "\n"
	     << "GL version : " << version << "\n"
	     << "GL max texture size : " << texture::get_max_size() << "\n"
	     << "GL number of texture units : " << nrtexunits << "\n"
	     << "GL number of lights : " << nrlights << "\n"
	     << "GL number of clip planes : " << nrclipplanes << "\n"
	     << "GL maximum viewport dimensions : " << maxviewportdims[0] << "x" << maxviewportdims[1] << "\n"
	     << "GL depth bits (current) : " << depthbits << "\n"
	     << "Supported GL extensions :\n" << extensions << "\n";
		
	instance = this;
}

system::~system()
{
	if (!instance) {
		SDL_Quit();
		throw sdl_error("system destruction: system instance doesn't exist");
	}
	SDL_Quit();
	write_console();
	instance = 0;
}



void system::set_video_mode(unsigned res_x_, unsigned res_y_, bool fullscreen)
{
	bool ok = false;
	for (list<vector2i>::const_iterator it = available_resolutions.begin(); it != available_resolutions.end(); ++it) {
		if (*it == vector2i(res_x_, res_y_)) {
			ok = true;
			break;
		}
	}
	if (!ok)
		throw invalid_argument("invalid resolution requested!");
	const SDL_VideoInfo *videoInfo = SDL_GetVideoInfo();
	if (!videoInfo)
		throw sdl_error("Video info query failed");
	int videoFlags  = SDL_OPENGL | SDL_GL_DOUBLEBUFFER | SDL_HWPALETTE;
	videoFlags |= (videoInfo->hw_available) ? SDL_HWSURFACE : SDL_SWSURFACE;
	if (fullscreen)
		videoFlags |= SDL_FULLSCREEN;
	if (videoInfo->blit_hw)
		videoFlags |= SDL_HWACCEL;
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	int bpp = videoInfo->vfmt->BitsPerPixel;
	SDL_Surface* screen = SDL_SetVideoMode(res_x_, res_y_, bpp, videoFlags);
	if (!screen)
		throw sdl_error("Video mode set failed");
	res_x = res_x_;
	res_y = res_y_;
	is_fullscreen = fullscreen;
	screen_resize(res_x, res_y, nearz, farz);
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



void system::clear_console()
{
	console_text.clear();
	add_console("$ffffffLog restart.");
}



void system::add_console(const string& tx)
{
	if (instance == 0) {
		cerr << tx << " (SYSTEM not yet initialized)\n";
		return;
	}
	console_text.push_back(tx);
}



void system::add_console(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vsnprintf(sprintf_tmp, 1024, fmt, args);
	va_end(args);
	add_console(string(sprintf_tmp));
}



void system::draw_console_with(const font* fnt, const texture* background)
{
	console_font = fnt;
	console_background = background;
}



void system::draw_console()
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
	if (draw_2d) throw runtime_error("2d drawing already turned on");
	glFlush();
	glViewport(0, 0, res_x, res_y);
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
	glPixelZoom(float(res_x)/res_x_2d, -float(res_y)/res_y_2d);	// flip images
}

void system::unprepare_2d_drawing()
{
	if (!draw_2d) throw runtime_error("2d drawing already turned off");
	glFlush();
	glPixelZoom(1.0f, 1.0f);
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	draw_2d = false;
}

unsigned long system::millisec()
{
	return SDL_GetTicks() - time_passed_while_sleeping;
}

void system::myassert(bool cond, const string& msg)
{
	if (this == 0 && !cond) {
		cerr << msg << "\n";
#ifdef THROWERR
		throw std::exception();
#endif
		exit(0);
	}
	if (!cond) {
		add_console("!ERROR!");
		if (msg != "")
			add_console(msg);
		else
			add_console("unknown error");
		SDL_Quit();
		write_console();
#ifdef THROWERR
		throw std::exception();
#endif
		exit(0);
	}
}

void system::myassert(bool cond, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vsnprintf(sprintf_tmp, 1024, fmt, args);
	va_end(args);
	myassert(cond, string(sprintf_tmp));
}

void system::error(const string& msg)
{
	if (!this) {
		cerr << "ERROR: " << msg << "\n";
	} else {
		add_console(string("ERROR: ") + msg);
		SDL_Quit();
		write_console();
	}
	exit(0);
}

void system::error(const char* msg, ...)
{
	va_list args;
	va_start(args, msg);
	vsnprintf(sprintf_tmp, 1024, msg, args);
	va_end(args);
	error(string(sprintf_tmp));
}

void system::swap_buffers()
{
	if (show_console) {
		draw_console();
	}
	SDL_GL_SwapBuffers();
	if (maxfps > 0) {
		unsigned tm = millisec();
		unsigned d = tm - last_swap_time;
		unsigned dmax = 1000/maxfps;
		if (d < dmax) {
			SDL_Delay(dmax - d);
			last_swap_time = tm + dmax - d;
		} else {
			last_swap_time = tm;
		}
	}
}

//intermediate solution: just return list AND handle events, the app client can choose then
//what he wants (if he handles events by himself, he have to flush the key queue each frame)
list<SDL_Event> system::poll_event_queue()
{
	list<SDL_Event> events;

	SDL_Event event;
	SDL_keysym keysym;
	do {
		unsigned nr_of_events = 0;
		while (SDL_PollEvent(&event)) {
			++nr_of_events;
			events.push_back(event);
			switch (event.type) {
				case SDL_QUIT:			// Quit event
					//fixme: this is A VERY BAD IDEA!
					//threads hang on exit...
					//better pass this event to user and let him handle it!
#if 0
					events.pop_back();
					break;
#else
					SDL_Quit();	// to clean up mouse etc. after kill
					exit(0);	// fixme
#endif
				
				case SDL_ACTIVEEVENT:		// Application activation or focus event
					if (event.active.state & SDL_APPMOUSEFOCUS) {
						if (event.active.gain == 0) {
							if (!is_sleeping) {
								is_sleeping = true;
								sleep_time = SDL_GetTicks();
							}
						} else {
							if (is_sleeping) {
								is_sleeping = false;
								time_passed_while_sleeping += SDL_GetTicks() - sleep_time;
							}
						}
					}
					events.pop_back();	// filter these events.
					break;
				
				case SDL_KEYDOWN:		// Keyboard event - key down
					keysym = event.key.keysym;
					if (keysym.unicode == '^')
						show_console = !show_console;
					break;
				
				case SDL_KEYUP:			// Keyboard event - key up
					keysym = event.key.keysym;
					// nothing to do.
					break;
		
				case SDL_MOUSEMOTION:		// Mouse motion event
					// translate coordinates!
					events.back().motion.x = events.back().motion.x *
						int(res_x_2d) / int(res_x);
					events.back().motion.y = events.back().motion.y *
						int(res_y_2d) / int(res_y);
					// be careful: small motions at larger screens
					// could get lost! fixme
					events.back().motion.xrel = events.back().motion.xrel *
						int(res_x_2d) / int(res_x);
					events.back().motion.yrel = events.back().motion.yrel *
						int(res_y_2d) / int(res_y);
					break;
				
				case SDL_MOUSEBUTTONDOWN:	// Mouse button event - button down
					// translate coordinates!
					events.back().button.x = events.back().button.x *
						int(res_x_2d) / int(res_x);
					events.back().button.y = events.back().button.y *
						int(res_y_2d) / int(res_y);
					break;
				
				case SDL_MOUSEBUTTONUP:		// Mouse button event - button up
					// translate coordinates!
					events.back().button.x = events.back().button.x *
						int(res_x_2d) / int(res_x);
					events.back().button.y = events.back().button.y *
						int(res_y_2d) / int(res_y);
					break;
					
				default:			// Should NEVER happen !
					add_console("unknown event caught");
					break; // quick hack
					//throw runtime_error("Unknown Event !");
					//break;
			}
		}
		// do not waste CPU time when sleeping
		if (nr_of_events == 0 && is_sleeping)
			SDL_Delay(25);
	} while (is_sleeping);

	return events;
}

void system::screenshot()
{
	Uint32 rmask, gmask, bmask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	rmask = 0xff000000;
	gmask = 0x00ff0000;
	bmask = 0x0000ff00;
#else
	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
#endif
	vector<Uint8> pic(res_x*res_y*3);
	glReadPixels(0, 0, res_x, res_y, GL_RGB, GL_UNSIGNED_BYTE, &pic[0]);
	// flip image vertically
	vector<Uint8> flip(res_x*3);
	for (unsigned y = 0; y < res_y/2; ++y) {
		unsigned y2 = res_y-y-1;
		memcpy(&flip[0], &pic[res_x*3*y], res_x*3);
		memcpy(&pic[res_x*3*y], &pic[res_x*3*y2], res_x*3);
		memcpy(&pic[res_x*3*y2], &flip[0], res_x*3);
	}
	SDL_Surface* tmp = SDL_CreateRGBSurfaceFrom(&pic[0], res_x, res_y,
		24, res_x*3, rmask, gmask, bmask, 0);
	ostringstream os;
	os << "screenshot" << screenshot_nr++ << ".bmp";
	SDL_SaveBMP(tmp, os.str().c_str());
	SDL_FreeSurface(tmp);
	add_console(string("screenshot taken as ")+os.str());
}

void system::draw_rectangle(int x, int y, int w, int h)
{
/*
//doesn't work. why? fixme: see OpenGL redbook. coordinate offset for pixel drawing.
			//this is the reason
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

bool system::extension_supported(const string& s) const
{
	set<string>::const_iterator it = supported_extensions.find(s);
	return (it != supported_extensions.end());
}
