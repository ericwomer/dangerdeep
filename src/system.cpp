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
#include "log.h"
#include "primitives.h"
#include "shader.h"

#include <iostream>
#include <sstream>
#include <cmath>
#include <cstdarg>
#include <stdexcept>
#include <fstream>
#include <vector>
using namespace std;

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

system::parameters::parameters() :
	near_z(1.0),
	far_z(1000.0),
	resolution_x(1680),
	resolution_y(1050),
	fullscreen(false),
	use_multisampling(false),
	hint_multisampling(0),
	multisample_level(0),
	hint_fog(0),
	hint_mipmap(0),
	hint_texture_compression(0),
	vertical_sync(true)
{
}
	


system::parameters::parameters(double near_z_, double far_z_, unsigned res_x, unsigned res_y, bool fullscreen_) :
	near_z(near_z_),
	far_z(far_z_),
	resolution_x(res_x),
	resolution_y(res_y),
	fullscreen(fullscreen_),
	use_multisampling(false),
	hint_multisampling(0),
	multisample_level(0),
	hint_fog(0),
	hint_mipmap(0),
	hint_texture_compression(0),
	vertical_sync(true)
{
}



system::system(const parameters& params_) :
	params(params_),
	screen(0),
	show_console(false),
	console_font(0),
	console_background(0),
	draw_2d(false),
	time_passed_while_sleeping(0),
	sleep_time(0),
	is_sleeping(false),
	maxfps(0),
	last_swap_time(0),
	screenshot_nr(0)
{

	int err = SDL_Init(SDL_INIT_VIDEO);
	if (err < 0)
		throw sdl_error("video init failed");

//	if (params.window_caption.length() > 0) { // Quesiont earlyer was answered.
//		SDL_WM_SetCaption(params.window_caption.c_str(), NULL);
//	}

	// request available SDL video modes
	// !Rake: Not necessary in SDL2 for
	// initial runnint since we use
	// SDL_FULLSCREEN_DESKTOP when
	// creating the window
	// ...hopefully.

	// SDL_DisplayMode* modes = new(SDL_DisplayMode);
	int numdisplays = 0, displayindex = 0; // displayindex can always be 0 here since its only for testing on one monitor/display

	numdisplays = SDL_GetNumVideoDisplays();

	int numdisplaymode = SDL_GetNumDisplayModes(displayindex);
	if (numdisplaymode == 0)
		throw sdl_error("Failed to query number of display modes");

	std::vector<SDL_DisplayMode> modes; //
	SDL_DisplayMode* dummy = new SDL_DisplayMode;


	for (int i = 0; i < SDL_GetNumDisplayModes(0); ++i) {
		if(SDL_GetDisplayMode(displayindex,i,dummy) != 0) {
			throw sdl_error("Failed to get display mode."); // !Rake: for lack of a better error message.
		}

#ifdef DEBUG // !Rake: some debug cruft for your pleasure
		cout << "Available Display Modes: " << dummy->w << "x" << dummy->h << " @ " << dummy->refresh_rate << std::endl;
#endif // DEBUG
		modes.push_back(*dummy);
		available_resolutions.push_back(vector2i(dummy->w, dummy->h));
	}
	delete dummy;

	try {
		set_video_mode(params.resolution_x, params.resolution_y, params.fullscreen,false);
	}
	catch (...) {
		SDL_Quit();
		throw;
	}

	SDL_EventState(SDL_WINDOWEVENT_SIZE_CHANGED, SDL_IGNORE);//fixme
	SDL_EventState(SDL_SYSWMEVENT, SDL_IGNORE);//fixme
	SDL_JoystickEventState(SDL_IGNORE);
	// SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,SDL_DEFAULT_REPEAT_INTERVAL);//fixme !Rake: Fixed?
	SDL_ShowCursor(SDL_ENABLE);//fixme
	// SDL_EnableUNICODE(1); // !Rake: removed in SDL2

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
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &nrtexunits);
	glGetIntegerv(GL_MAX_LIGHTS, &nrlights);
	glGetIntegerv(GL_MAX_CLIP_PLANES, &nrclipplanes);
	glGetIntegerv(GL_MAX_VIEWPORT_DIMS, maxviewportdims);
	glGetIntegerv(GL_DEPTH_BITS, &depthbits);

	log_info("***** OpenGL Information *****\n\n\n"
		 << "OpenGL vendor : " << vendor << "\n"
		 << "GL renderer : " << renderer << "\n"
		 << "GL version : " << version << "\n"
		 << "GL max texture size : " << texture::get_max_size() << "\n"
		 << "GL number of texture units : " << nrtexunits << "\n"
		 << "GL number of lights : " << nrlights << "\n"
		 << "GL number of clip planes : " << nrclipplanes << "\n"
		 << "GL maximum viewport dimensions : " << maxviewportdims[0] << "x" << maxviewportdims[1] << "\n"
		 << "GL depth bits (current) : " << depthbits << "\n"
		 << "Supported GL extensions :\n" << extensions << "\n");

	bool glsl_supported = extension_supported("GL_ARB_fragment_shader") &&
		extension_supported("GL_ARB_shader_objects") &&
		/* sys().extension_supported("GL_ARB_shading_language_100") && */
		extension_supported("GL_ARB_vertex_shader");
	if (!glsl_supported) {
		throw std::runtime_error("GLSL shaders are not supported!");
	}
	if (vendor.find("NVIDIA") != std::string::npos) {
		// we have an Nvidia card (most probably)
		glsl_shader::is_nvidia_card = true;
	}

}



system::system()
{
	throw std::runtime_error("default constructor of system class forbidden!");
}



system::~system()
{
	glsl_shader_setup::default_deinit();
	SDL_Quit();
}



void system::set_video_mode(unsigned& res_x_, unsigned& res_y_, bool fullscreen, bool resize)
{

	// only limit possible mode when using fullscreen.
	// windows can have any sizes.
	// !Rake: Add switch/case for fullscreen fullscreen_desktop and window
	// !Rake: This will get a rewrite completely to utilize SDL_WINDOW_FULLSCREEN_DESKTOP only since
	// SDL_WINDOW_FULLSCREEN causes so many issues.

	if (fullscreen) {
		bool ok = false;
		unsigned max_mode_x = 0, max_mode_y = 0;
		for (list<vector2i>::const_iterator it = available_resolutions.begin(); it != available_resolutions.end(); ++it) {
			if (*it == vector2i(res_x_, res_y_)) {
				ok = true;
				break;
			}
			if (it->x >= int(max_mode_x)) {
				max_mode_x = it->x;
				if (it->y >= int(max_mode_y)) {
					max_mode_y = it->y;
				}
			}
		}

		if (res_x_ == 0 || res_y_ == 0) {
			res_x_ = max_mode_x;
			res_y_ = max_mode_y;
		} else if (!ok) {
			throw invalid_argument("invalid resolution requested!");
		}
	}

//	!Rake SDL_VideoInfo was removed from SDL2
//	const SDL_VideoInfo *videoInfo = SDL_GetVideoInfo();
//	if (!videoInfo)
//		throw sdl_error("Video info query failed");

	// Note: the SDL_GL_DOUBLEBUFFER flag is ignored with OpenGL modes.
	// the flags SDL_HWPALETTE, SDL_HWSURFACE and SDL_HWACCEL
	// are not needed for OpenGL mode.
	Uint32 videoFlags  = SDL_WINDOW_OPENGL;
	 if (fullscreen)
		videoFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP; // !Rake: Its a nice safety mesure to use SDL_WINDOW_FULLSCREEN_DESKTOP
													 // and let the players change it later.
	if (SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1) < 0)
		throw sdl_error("setting double buffer mode failed");

	/* Sometimes when setting SDL_GL_ACCELERATED_VISUAL (0 or 1 !?) multisampling is borked
	 * if (SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 0) < 0)
	 *	throw sdl_error("setting accelerated visual failed");
	 */

	if (SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, params.use_multisampling) < 0)
		throw sdl_error("setting multisampling failed");
	if (SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, params.multisample_level) < 0)
		throw sdl_error("setting multisamplelevel failed");


	if(!resize) {
		// SDL_Surface* screen = SDL_SetVideoMode(res_x_, res_y_, bpp, videoFlags);
		screen = SDL_CreateWindow(params.window_caption.c_str(),
										  SDL_WINDOWPOS_UNDEFINED,
										  SDL_WINDOWPOS_UNDEFINED,
										  res_x_,res_y_,
										  videoFlags);
		if (!screen)
			throw sdl_error("Video mode set failed");

		glcontext = SDL_GL_CreateContext(screen);
		if (!glcontext) {
			std::string error = "Couldn't create context: ";
			error += SDL_GetError();
			throw sdl_error(error.c_str());
		}

	} else { // must be window resize
		SDL_SetWindowSize(screen,res_x_,res_y_);
	}
	// enable VSync, but doesn't work on Linux/Nvidia/SDL 1.2.11 (?!)
	// works with Linux/Nvidia/SDL 1.2.12
	if (SDL_GL_SetSwapInterval( params.vertical_sync) < 0){
		std::cout << "SDL_GL_SetSwapInterval: " << SDL_GetError() << std::endl;
		throw sdl_error("setting VSync failed");
	}

	params.resolution_x = res_x_;
	params.resolution_y = res_y_;
	params.fullscreen = fullscreen;

	// OpenGL Init.
	glClearColor (32/255.0, 64/255.0, 192/255.0, 1.0);
	glClearDepth(1.0);
	glDepthFunc(GL_LEQUAL);
	glShadeModel(GL_SMOOTH);
	glDisable(GL_LIGHTING); // we use shaders for everything
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_NORMALIZE);
	glEnable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE); // should be obsolete
	// set up some things for drawing pixels
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glReadBuffer(GL_BACK);
	glDrawBuffer(GL_BACK);
	
	// screen resize
	glViewport(0, 0, params.resolution_x, params.resolution_y);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gl_perspective_fovx (90.0, (GLdouble)params.resolution_x/(GLdouble)params.resolution_y, params.near_z, params.far_z);
	float m[16];
	glGetFloatv(GL_PROJECTION_MATRIX, m);
	xscal_2d = 2*params.near_z/m[0];
	yscal_2d = 2*params.near_z/m[5];
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// enable texturing on all units
	GLint nrtexunits = 0;
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &nrtexunits);
	for (unsigned i = 0; i < unsigned(nrtexunits); ++i) {
		glActiveTexture(GL_TEXTURE0 + i);
		glEnable(GL_TEXTURE_2D);
	}
	
	if (params.use_multisampling) {
		glEnable(GL_MULTISAMPLE);
		switch(params.hint_multisampling) {
			case 1: 
				glHint(GL_MULTISAMPLE_FILTER_HINT_NV, GL_NICEST);
			break;
			case 2:
				glHint(GL_MULTISAMPLE_FILTER_HINT_NV, GL_FASTEST);
			break;
			default:
				glHint(GL_MULTISAMPLE_FILTER_HINT_NV, GL_DONT_CARE);
			break;
		}
	}
	switch (params.hint_fog) {
		case 1: 
			glHint(GL_FOG_HINT, GL_NICEST);
		break;
		case 2:
			glHint(GL_FOG_HINT, GL_FASTEST);
		break;
		default:
			glHint(GL_FOG_HINT, GL_DONT_CARE);
		break;
	}
	switch (params.hint_mipmap) {
		case 1: 
			glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
		break;
		case 2:
			glHint(GL_GENERATE_MIPMAP_HINT, GL_FASTEST);
		break;
		default:
			glHint(GL_GENERATE_MIPMAP_HINT, GL_DONT_CARE);
		break;
	}
	switch (params.hint_texture_compression) {
		case 1: 
			glHint(GL_TEXTURE_COMPRESSION_HINT, GL_NICEST);
		break;
		case 2:
			glHint(GL_TEXTURE_COMPRESSION_HINT, GL_FASTEST);
		break;
		default:
			glHint(GL_TEXTURE_COMPRESSION_HINT, GL_DONT_CARE);
		break;
	}
	// since we use vertex arrays for every primitive, we can enable it
	// here and leave it enabled forever
	glEnableClientState(GL_VERTEX_ARRAY);

	glsl_shader_setup::default_init();

	// compute 2d area and resolution. it must be 4:3 always.
	if (params.resolution_x * 3 >= params.resolution_y * 4) {
		// screen is wider than high
		res_area_2d_w = params.resolution_y * 4 / 3;
		res_area_2d_h = params.resolution_y;
		res_area_2d_x = (params.resolution_x - res_area_2d_w) / 2;
		res_area_2d_y = 0;
	} else {
		// screen is higher than wide
		res_area_2d_w = params.resolution_x;
		res_area_2d_h = params.resolution_x * 3 / 4;
		res_area_2d_x = 0;
		res_area_2d_y = (params.resolution_y - res_area_2d_h) / 2;
		// maybe limit y to even lines for interlaced displays?
		//res_area_2d_y &= ~1U;
	}
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
		primitives::textured_quad(vector2f(0,0), vector2f(res_x_2d,res_y_2d/2),
					  *console_background,
					  vector2f(0,0), vector2f(4, 2),
					  colorf(1,1,1,0.75)).render();
	}
	
	unsigned fh = console_font->get_height();
	unsigned lines = res_y_2d/(2*fh)-2;
	console_font->print(fh, fh, log::instance().get_last_n_lines(lines));
	unprepare_2d_drawing();
}



int system::transform_2d_x(int x)
{
	x -= int(res_area_2d_x);
	if (x < 0) x = 0;
	if (x >= int(res_area_2d_w)) x = int(res_area_2d_w)-1;
	return x * res_x_2d / res_area_2d_w;
}



int system::transform_2d_y(int y)
{
	y -= int(res_area_2d_y);
	if (y < 0) y = 0;
	if (y >= int(res_area_2d_h)) y = int(res_area_2d_h)-1;
	return y * res_y_2d / res_area_2d_h;
}



void system::prepare_2d_drawing()
{
	if (draw_2d) throw runtime_error("2d drawing already turned on");
	glFlush();
	glViewport(res_area_2d_x, res_area_2d_y, res_area_2d_w, res_area_2d_h);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, res_x_2d, 0, res_y_2d);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(0, res_y_2d, 0);
	glScalef(1, -1, 1);
	glDisable(GL_DEPTH_TEST);
	glCullFace(GL_FRONT);
	draw_2d = true;
	glPixelZoom(float(res_area_2d_w)/res_x_2d, -float(res_area_2d_h)/res_y_2d);	// flip images
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
	glEnable(GL_DEPTH_TEST);
	glCullFace(GL_BACK);
	draw_2d = false;
}



unsigned long system::millisec()
{
	return SDL_GetTicks() - time_passed_while_sleeping;
}



void system::swap_buffers(SDL_Window *window)
{
	if (show_console) {
		draw_console();
	}
	SDL_GL_SwapWindow(window);
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

	Uint32 window_id = SDL_GetWindowID(sys().get_sdl_window()); // !Rake: Should only need to call this once

	SDL_Event event;
	do {
		unsigned nr_of_events = 0;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT:			// Quit event
					log_info("---------- immediate exit ----------");
					log::instance().write(std::cerr, log::LOG_SYSINFO);
 					{
 						std::ofstream f("log.txt");
 						log::instance().write(f, log::LOG_SYSINFO);
					}
					throw quit_exception(0);
				// !Rake: an attemp to rewrite the window focus event
				case SDL_WINDOWEVENT:
				if(event.window.windowID == window_id) {
					if(event.window.event == SDL_WINDOWEVENT_FOCUS_LOST){
							if(!is_sleeping) {
								is_sleeping = true;
								sleep_time = SDL_GetTicks();
							}
					}else if(event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
						if(is_sleeping) {
								is_sleeping = false;
								time_passed_while_sleeping += SDL_GetTicks() - sleep_time;
							}
					}
				}
				continue; // filter these events
				/* !Rake: replaced with above ^
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
					continue; // filter these events
				*/ // !Rake:

				case SDL_KEYDOWN:		// Keyboard event - key down
					if (event.key.keysym.scancode == SDL_SCANCODE_KP_POWER)
						show_console = !show_console;
					break;
				
				case SDL_KEYUP: // pass known events through
				case SDL_MOUSEMOTION:
				case SDL_MOUSEBUTTONDOWN:
				case SDL_MOUSEBUTTONUP:
					break;
				
				default: // by default don't pass though unknown events
					continue;
			}

			++nr_of_events;
			events.push_back(event);
		}
		// do not waste CPU time when sleeping
		if (nr_of_events == 0 && is_sleeping)
			SDL_Delay(25);
	} while (is_sleeping);

	return events;
}



double system::translate_motion_x(const SDL_Event& event)
{
	if (event.type == SDL_MOUSEMOTION)
		return event.motion.xrel * double(res_x_2d) / res_area_2d_w;
	else
		return 0.0;
}



double system::translate_motion_y(const SDL_Event& event)
{
	if (event.type == SDL_MOUSEMOTION)
		return event.motion.yrel * double(res_y_2d) / res_area_2d_h;
	else
		return 0.0;
}



vector2 system::translate_motion(const SDL_Event& event)
{
	return vector2(translate_motion_x(event), translate_motion_y(event));
}



int system::translate_position_x(const SDL_Event& event)
{
	if (event.type == SDL_MOUSEMOTION)
		return transform_2d_x(event.motion.x);
	else
		return transform_2d_x(event.button.x);
}



int system::translate_position_y(const SDL_Event& event)
{
	if (event.type == SDL_MOUSEMOTION)
		return transform_2d_y(event.motion.y);
	else
		return transform_2d_y(event.button.y);
}



vector2i system::translate_position(const SDL_Event& event)
{
	return vector2i(translate_position_x(event), translate_position_y(event));
}



void system::screenshot(const std::string& filename)
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
	vector<Uint8> pic(params.resolution_x*params.resolution_y*3);
	glReadPixels(0, 0, params.resolution_x, params.resolution_y, GL_RGB, GL_UNSIGNED_BYTE, &pic[0]);
	// flip image vertically
	vector<Uint8> flip(params.resolution_x*3);
	for (unsigned y = 0; y < params.resolution_y/2; ++y) {
		unsigned y2 = params.resolution_y-y-1;
		memcpy(&flip[0], &pic[params.resolution_x*3*y], params.resolution_x*3);
		memcpy(&pic[params.resolution_x*3*y], &pic[params.resolution_x*3*y2], params.resolution_x*3);
		memcpy(&pic[params.resolution_x*3*y2], &flip[0], params.resolution_x*3);
	}
	SDL_Surface* tmp = SDL_CreateRGBSurfaceFrom(&pic[0], params.resolution_x, params.resolution_y,
		24, params.resolution_x*3, rmask, gmask, bmask, 0);
	std::string fn;
	if (filename.empty()) {
		ostringstream os;
		os << screenshot_dir << "screenshot" << screenshot_nr++ << ".bmp";
		fn = os.str();
	} else {
		fn = filename + ".bmp";
	}
	SDL_SaveBMP(tmp, fn.c_str());
	SDL_FreeSurface(tmp);
	log_info("screenshot taken as " << fn);
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



font& system::register_font(const std::string& basedir, const std::string& basefilename, unsigned char_spacing)
{
	std::pair<std::map<std::string, font*>::iterator, bool> ir = fonts.insert(std::make_pair(basefilename, (font*)0));
	if (!ir.second)
		throw std::runtime_error("tried to register font twice!");
	ir.first->second = new font(basedir + basefilename, char_spacing);
	return *(ir.first->second);
}



font& system::get_font(const std::string& basefilename) const
{
	std::map<std::string, font*>::const_iterator it = fonts.find(basefilename);
	if (it == fonts.end())
		throw std::runtime_error("font unknown");
	return *it->second;
}



bool system::unregister_font(const std::string& basefilename)
{
	std::map<std::string, font*>::iterator it = fonts.find(basefilename);
	if (it == fonts.end())
		return false;
	delete it->second;
	fonts.erase(it);
	return true;
}
