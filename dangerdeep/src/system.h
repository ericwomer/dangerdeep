// SDL/OpenGL based system services
// (C)+(W) by Thorsten Jordan. See LICENSE

#ifndef SYSTEM_H
#define SYSTEM_H

#include <queue>
#include <list>
using namespace std;
#include "font.h"
#include "texture.h"

#ifdef WIN32
#ifndef M_PI
#define M_PI 3.1415926535897932	// should be in math.h, but not for Windows. *sigh*
#endif
#include <float.h>
#define isfinite(a) _finite(a)
#ifdef system
#undef system
#endif
#endif

class system
{
public:
	enum button_type { left_button=0x1, right_button=0x2, middle_button=0x4, wheel_up=0x8, wheel_down=0x10 };
	system(double nearz_, double farz_, unsigned res=640, bool fullscreen=true);
	~system();
	void swap_buffers(void);
	void poll_event_queue(void); // !must! be called periodically (once per frame)
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
	void screen_resize(unsigned w, unsigned h, double nearz, double farz);
	
	void clear_console(void);
	// fixme maybe printf-like (va_start,...) input also.
	void add_console(const string& tx);
	void draw_console_with(const font* fnt, const texture* background = 0);
	void write_console(bool fileonly = false) const;
	void prepare_2d_drawing(void);	// must be called as pair!
	void unprepare_2d_drawing(void);

	unsigned long millisec(void);	// returns time in milliseconds

	static system* sys(void) { return instance; };
	void myassert(bool cond, const string& msg = "");
	
	void screenshot(void);

	// 2d drawing must be turned on for this functions
/*	
	void draw_image(int x, int y, const texture* t) const { t->draw_image(x, y); }
	void draw_hm_image(int x, int y, const texture* t) const { t->draw_hm_image(x, y); }
	void draw_vm_image(int x, int y, const texture* t) const { t->draw_vm_image(x, y); }
	void draw_image(int x, int y, int w, int h, const texture* t) const { t->draw_image(x, y, w, h); }
	void draw_hm_image(int x, int y, int w, int h, const texture* t) const { t->draw_hm_image(x, y, w, h); }
	void draw_vm_image(int x, int y, int w, int h, const texture* t) const { t->draw_vm_image(x, y, w, h); }
	void draw_rot_image(int x, int y, double angle, const texture* t) const { t->draw_rot_image(x, y, angle); }
	void draw_tiles(int x, int y, int w, int h, unsigned tilesx, unsigned tilesy,
		const texture* t) const { t->draw_tiles(x, y, w, h, tilesx, tilesy); }
*/
	void draw_rectangle(int x, int y, int w, int h);
	void no_tex(void) const { glBindTexture(GL_TEXTURE_2D, 0); }
	
	// takes effect only after next prepare_2d_drawing()
	void set_res_2d(unsigned x, unsigned y) { res_x_2d = x; res_y_2d = y; }

	// give FOV X in degrees, aspect (w/h), znear and zfar.	
	void gl_perspective_fovx(double fovx, double aspect, double znear, double zfar);

	bool key_shift(void) const { return is_key_down(SDLK_LSHIFT) || is_key_down(SDLK_RSHIFT) || is_key_down(SDLK_CAPSLOCK); }
	unsigned get_res_x(void) const { return res_x; };
	unsigned get_res_y(void) const { return res_y; };
	
private:
	system() {};
	unsigned res_x, res_y;		// virtual resolution (fixed for project)
	double nearz, farz;
	bool is_fullscreen;
	bool show_console;
	const font* console_font;
	const texture* console_background;	
	list<string> console_text;
	
	bool draw_2d;
	unsigned res_x_2d, res_y_2d;	// real resolution (depends on user settings)
	
	queue<SDL_keysym> keyqueue;
	int mouse_xrel, mouse_yrel, mouse_x, mouse_y;
	int mouse_b;
	
	static system* instance;

	void draw_console(void);

	unsigned long time_passed_while_sleeping;	// total sleep time
	unsigned long sleep_time;	// time when system gets to sleep
	bool is_sleeping;
	
	double xscal_2d, yscal_2d;	// factors needed for 2d drawing
	
	int screenshot_nr;
};

#endif
