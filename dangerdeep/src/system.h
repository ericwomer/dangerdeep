// SDL/OpenGL based system services
// (C)+(W) by Thorsten Jordan. See LICENSE

#ifndef SYSTEM_H
#define SYSTEM_H

#include <queue>
#include <list>
#include <set>
#include <string>
using namespace std;
#include "font.h"
#include "texture.h"

#ifdef WIN32
#ifndef M_PI
#define M_PI 3.1415926535897932	// should be in math.h, but not for Windows. *sigh*
#endif
#include <float.h>
#ifndef isfinite
#define isfinite(a) _finite(a)
#endif
#endif

#ifdef WIN32
#define system System
#endif

class system
{
public:
	enum button_type { left_button=0x1, right_button=0x2, middle_button=0x4, wheel_up=0x8, wheel_down=0x10 };
	system(double nearz_, double farz_, unsigned res=640, bool fullscreen=true);
	~system();
	void swap_buffers(void);

	// must be called once per frame (or the OS will think your app is dead)
	// the events are also bypassed to the application.
	// if you want to interpret the events yourself, just call flush_key_queue()
	// after poll_event_queue to avoid filling the queue.
	list<SDL_Event> poll_event_queue(void);
	//fixme: mouse position translation is now missing!!!!
	//but if the position is translated here, e.g. from high to a low resolution
	//many small movements may get mapped to < 1.0 movements, they are zero ->
	//mouse movement is NEVER noticed.
	// We have to translate the events and handle screen res, mouse pos/movement
	//must be correct also for subpixel cases! big fixme!

//these functions are useless with new poll event queue
	void get_mouse_motion(int &x, int &y);
	void get_mouse_position(int &x, int &y);
	// get mouse button state as mask of button_type
	int get_mouse_buttons(void) const { return mouse_b; }
	// get SDL code of next key (0 if no key in queue)
	SDL_keysym get_key(void);
	bool is_key_in_queue(void) const;
	bool is_key_down(int code) const;
	void flush_key_queue(void);
	// wait for keypress
	SDL_keysym getch(void);
///-------

	void screen_resize(unsigned w, unsigned h, double nearz, double farz);
	
	void clear_console(void);
	// fixme maybe printf-like (va_start,...) input also.
	//void con_printf(const char* fmt, ...);
	void add_console(const string& tx);
	void draw_console_with(const font* fnt, const texture* background = 0);
	void write_console(bool fileonly = false) const;
	void prepare_2d_drawing(void);	// must be called as pair!
	void unprepare_2d_drawing(void);

	unsigned long millisec(void);	// returns time in milliseconds

	static inline system& sys(void) { return *instance; };
	void myassert(bool cond, const string& msg = "");
	
	void set_screenshot_directory(const string& s) { screenshot_dir = s; }
	void screenshot(void);

	void draw_rectangle(int x, int y, int w, int h);
	
	// takes effect only after next prepare_2d_drawing()
	void set_res_2d(unsigned x, unsigned y) { res_x_2d = x; res_y_2d = y; }

	// set maximum fps rate (0 for unlimited)
	void set_max_fps(unsigned fps) { maxfps = fps; }

	// give FOV X in degrees, aspect (w/h), znear and zfar.	
	void gl_perspective_fovx(double fovx, double aspect, double znear, double zfar);

	//fixme: is useless with new poll event queue
	bool key_shift(void) const { return is_key_down(SDLK_LSHIFT) || is_key_down(SDLK_RSHIFT) || is_key_down(SDLK_CAPSLOCK); }

	unsigned get_res_x(void) const { return res_x; };
	unsigned get_res_y(void) const { return res_y; };
	unsigned get_res_x_2d(void) const { return res_x_2d; };
	unsigned get_res_y_2d(void) const { return res_y_2d; };
	
	// is a given OpenGL extension supported?
	bool extension_supported(const string& s) const;
	
private:
	system() {};
	unsigned res_x, res_y;		// virtual resolution (fixed for project)
	double nearz, farz;
	bool is_fullscreen;
	bool show_console;
	const font* console_font;
	const texture* console_background;	
	list<string> console_text;
	
	set<string> supported_extensions;	// memory supported OpenGL extensions
	
	bool draw_2d;
	unsigned res_x_2d, res_y_2d;	// real resolution (depends on user settings)
	
	// these state variables are uselss with new poll event q
	queue<SDL_keysym> keyqueue;
	int mouse_xrel, mouse_yrel, mouse_x, mouse_y;
	int mouse_b;
	
	static system* instance;

	void draw_console(void);

	unsigned long time_passed_while_sleeping;	// total sleep time
	unsigned long sleep_time;	// time when system gets to sleep
	bool is_sleeping;
	
	unsigned maxfps;		// limit fps, give 0 for no limit.
	unsigned long last_swap_time;	// time of last buffer swap
	
	double xscal_2d, yscal_2d;	// factors needed for 2d drawing
	
	int screenshot_nr;
	string screenshot_dir;
};

// to make user's code even shorter
inline class system& sys(void) { return system::sys(); }

#endif
