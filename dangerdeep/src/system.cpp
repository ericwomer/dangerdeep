// SDL/OpenGL based system services
// (C)+(W) by Thorsten Jordan. See LICENSE
// Parts taken and adapted from Martin Butterwecks's OOML SDL code (wws.sourceforget.net/projects/ooml/)

#include <iostream>
#include <sstream>
#include <cmath>
using namespace std;

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <SDL.h>
#include <SDL_image.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "system.h"
#include "texture.h"

class system* system::instance = 0;

system::system(double nearz_, double farz_, unsigned res, bool fullscreen) :
	res_x(res), res_y(res*3/4), nearz(nearz_), farz(farz_), is_fullscreen(fullscreen),
	show_console(false), console_font(0), console_background(0),
	draw_2d(false), keyqueue(), mouse_xrel(0), mouse_yrel(0), mouse_x(res/2),
	mouse_y(res*3/8), mouse_b(0), time_passed_while_sleeping(0), sleep_time(0),
	is_sleeping(false), maxfps(0), last_swap_time(0), screenshot_nr(0)
{
	myassert(res==640||res==800||res==1024||res==1280||res==512, "illegal resolution requested");
	myassert(!instance, "system construction: system instance already exists");

	res_x_2d = res_x;
	res_y_2d = res_y;

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
	SDL_EnableUNICODE(1);

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
	// set up some things for drawing pixels
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glReadBuffer(GL_BACK);
	glDrawBuffer(GL_BACK);
	
	// Screen resize
	screen_resize(res_x, res_y, nearz, farz);

	// check for some OpenGL Extensions
	string vendor = (char*)glGetString(GL_VENDOR);
	string renderer = (char*)glGetString(GL_RENDERER);
	string version = (char*)glGetString(GL_VERSION);
	string extensions = (char*)glGetString(GL_EXTENSIONS);
	if (extensions.length() > 0) {
		unsigned spos = 0;
		while (spos < extensions.length()) {
			unsigned pos = extensions.find(" ", spos);
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
	GLint nrtexunits = 0, nrlights = 0, nrclipplanes = 0, maxviewportdims = 0, depthbits = 0;
	glGetIntegerv(GL_MAX_TEXTURE_UNITS, &nrtexunits);
	glGetIntegerv(GL_MAX_LIGHTS, &nrlights);
	glGetIntegerv(GL_MAX_CLIP_PLANES, &nrclipplanes);
	glGetIntegerv(GL_MAX_VIEWPORT_DIMS, &maxviewportdims);
	glGetIntegerv(GL_DEPTH_BITS, &depthbits);

	cerr << "OpenGL vendor : " << vendor << "\n"
		<< "GL renderer : " << renderer << "\n"
		<< "GL version : " << version << "\n"
		<< "GL max texture size : " << texture::get_max_size() << "\n"
		<< "GL number of texture units : " << nrtexunits << "\n"
		<< "GL number of lights : " << nrlights << "\n"
		<< "GL number of clip planes : " << nrclipplanes << "\n"
		<< "GL maximum viewport dimensions : " << maxviewportdims << "\n"
		<< "GL depth bits (current) : " << depthbits << "\n"
		<< "Supported GL extensions :\n" << extensions << "\n";
		
	instance = this;
}

system::~system()
{
	myassert(instance != 0, "system destruction: system instance doesn't exist");
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
	myassert(draw_2d == false, "2d drawing already turned on");
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
	glPixelZoom(float(res_x)/res_x_2d, -float(res_y)/res_y_2d);	// flip images
}

void system::unprepare_2d_drawing()
{
	myassert(draw_2d == true, "2d drawing already turned off");
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

unsigned long system::millisec(void)
{
	return SDL_GetTicks() - time_passed_while_sleeping;
}

void system::myassert(bool cond, const string& msg)
{
	if (this == 0 && !cond) {
		cerr << msg << "\n";
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
		exit(0);
	}
}

void system::swap_buffers(void)
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

unsigned int system::poll_event_queue(void)
{
	SDL_Event event;
	SDL_keysym keysym;
	unsigned int eventcount = 0;
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
						eventcount++;
					}
					break;
				
				case SDL_KEYDOWN:		// Keyboard event - key down
					keysym = event.key.keysym;
					keyqueue.push(keysym);
					eventcount++;
					break;
				
				case SDL_KEYUP:			// Keyboard event - key up
					keysym = event.key.keysym;
					// nothing to do.
					break;
		
				case SDL_MOUSEMOTION:		// Mouse motion event
					mouse_xrel += event.motion.xrel;
					mouse_yrel += event.motion.yrel;
					mouse_x = event.motion.x;
					mouse_y = event.motion.y;
					eventcount++;
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
						case SDL_BUTTON_WHEELUP:
							mouse_b |= wheel_up;
							break;
						case SDL_BUTTON_WHEELDOWN:
							mouse_b |= wheel_down;
							break;
					}
					eventcount++;
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
						case SDL_BUTTON_WHEELUP:
							mouse_b &= ~wheel_up;
							break;
						case SDL_BUTTON_WHEELDOWN:
							mouse_b &= ~wheel_down;
							break;
					}
					eventcount++;
					break;
					
				default:			// Should NEVER happen !
					add_console("unknown event caught");
					break; // quick hack
					myassert(0, "Unknown Event !");
					break;
			}
		}
	} while (is_sleeping);

	return eventcount;
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

SDL_keysym system::get_key(void)
{
	SDL_keysym ks;
	if (keyqueue.empty()) {
		ks.scancode = 0;
		ks.sym = SDLK_UNKNOWN;
		ks.mod = KMOD_NONE;
		ks.unicode = 0;
		return ks;
	}
	ks = keyqueue.front();
	keyqueue.pop();
	if (ks.sym == SDLK_CARET)
		show_console = !show_console;
	return ks;
}

bool system::is_key_in_queue(void) const
{
	return !keyqueue.empty();
}
	
bool system::is_key_down(int code) const
{
	return (SDL_GetKeyState(0)[code] != 0);
}

void system::flush_key_queue(void)
{
	while (!keyqueue.empty())
		keyqueue.pop();
}

SDL_keysym system::getch(void)
{
	do {
		poll_event_queue();	// to avoid the app to run dead
	} while (keyqueue.empty());
	return get_key();
}

void system::screenshot(void)
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
