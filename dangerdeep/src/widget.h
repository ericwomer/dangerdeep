// OpenGL based widgets
// (C)+(W) by Thorsten Jordan. See LICENSE

#ifndef WIDGET_H
#define WIDGET_H

#include <list>
#include <string>
using namespace std;
#include "system.h"
#include "color.h"
#include "image.h"
#include "font.h"
#include "vector2.h"

// fixme: add image-widget
// fixme: implement checkbox widget

// maybe give mouse pos / movement as arguments of the on_click/on_drag functions.

// fixme: add header for menu widget (like old dftd menues)

// fixme: when editing in a widget_edit you have to click twice on a button to lose focus AND click the button .. bad

// fixme: make yes/no, ok, yes/no/cancel dialogue
// when a dialogue opens another one, both should be drawn, and the parent
// should pass draw()/process_input() calls to its child
// make a special flag: widget* wait_for?
// process_input(){if (wait_for)wait_for->process_input();else ...old...;} ?
// ein widget.close fehlt. close:= parent.remove(this), wenn parent==0 dann globale liste nach this
// durchsuchen. run() läuft dann bis globale liste leer ist.

// more widgets: progress bar

// theme files:
// two images, one for elements, one for icons.
// each image is one row of square elements.
// elements:
// 2 for background/sunken background
// 2*8 for borders (clockwise, starting top left), normal and inverse border
// square length is determined by height of image, so width has to be (2+2*8)*h = 18*h
// icons:
// arrow up, arrow down, unchecked box, checked box

class widget
{
protected:
	vector2i pos, size;
	string text;
	widget* parent;
	const image* background;
	bool enabled;
	list<widget*> children;
	int retval;
	bool closeme;	// is set to true by close(), stops run() the next turn
	
	widget();
	widget(const widget& );
	widget& operator= (const widget& );
	static void draw_frame(int x, int y, int w, int h, bool out);
	static void draw_rect(int x, int y, int w, int h, bool out);
	void draw_area(int x, int y, int w, int h, bool out) const;
public:
	class theme {
		theme();
		theme(const theme& );
		theme& operator= (const theme& );
	public:
		class texture *backg, *skbackg, *frame[8], *frameinv[8], *icons[4];
		const font* myfont;
		color textcol, textselectcol, textdisabledcol;
		int frame_size(void) const;
		int icon_size(void) const;
		theme(const char* elements_filename, const char* icons_filename, const font* fnt,
			color tc, color tsc, color tdc);
		~theme();
	};

protected:
	static class widget::theme* globaltheme;
	static widget* focussed;	// which widget has the focus
	static widget* mouseover;	// which widget the mouse is over
	
	static int oldmx, oldmy, oldmb;	// used for input calculation

public:	
	static void set_theme(theme* t);
	static theme* get_theme(void) { return globaltheme; }
	static theme* replace_theme(theme* t);
	widget(int x, int y, int w, int h, const string& text_, widget* parent_ = 0, const image* backgr = 0);
	virtual ~widget();
	virtual void add_child(widget* w);
	virtual void remove_child(widget* w);
	virtual void remove_children(void);
	virtual bool is_mouse_over(int mx, int my) const;
	virtual void draw(void) const;
	virtual bool compute_focus(int mx, int my);
	virtual bool compute_mouseover(int mx, int my);
	virtual vector2i get_pos(void) const { return pos; }
	virtual void set_pos(const vector2i& p) { move_pos(p - pos); }
	virtual void move_pos(const vector2i& p);
	virtual void align(int h, int v);	// give <0,0,>0 for left,center,right
	virtual vector2i get_size(void) const { return size; }
	virtual void set_size(const vector2i& s) { size = s; }
	virtual widget* get_parent(void) const { return parent; }
	virtual void set_parent(widget* w) { parent = w; }
	virtual string get_text(void) const { return text; }
	virtual void set_text(const string& s) { text = s; }
	virtual const image* get_background(void) const { return background; }
	virtual void set_background(const image* b) { background = b; }
	virtual void set_return_value(int rv) { retval = rv; }
	virtual bool is_enabled(void) const;
	virtual void enable(void);
	virtual void disable(void);

	// called for every key in queue
	virtual void on_char(const SDL_keysym& ks);
	// called on mouse button down
	virtual void on_click(int mx, int my, int mb) {}
	// called on mouse wheel action, mb is 1 for up, 2 for down
	virtual void on_wheel(int wd) {}
	// called on mouse button up
	virtual void on_release(void) {}
	// called on mouse move while button is down
	virtual void on_drag(int mx, int my, int rx, int ry, int mb) {}

	// determine type of input, fetch it to on_* functions
	virtual void process_input(const SDL_Event& event);
	// just calls the previous function repeatedly
	virtual void process_input(const list<SDL_Event>& events);

	// run() always returns 1    - fixme: make own widget classes for them?
	static widget* create_dialogue_ok(widget* parent_, const string& title, const string& text = "");
	widget* create_dialogue_ok(const string& title, const string& text = "") { return create_dialogue_ok(this, title, text); }
	// run() returns 1 for ok, 0 for cancel
	static widget* create_dialogue_ok_cancel(widget* parent_, const string& title, const string& text = "");
	widget* create_dialogue_ok_cancel(const string& title, const string& text = "") { return create_dialogue_ok_cancel(this, title, text); }

	// show & exec. widget, automatically disable widgets below
	// run() runs for "time" milliseconds (or forever if time == 0), then returns
	// if do_stacking is false only this widget is drawn, but none of its parents
	virtual int run(unsigned timeout = 0, bool do_stacking = true);
	virtual void close(int val);	// close this widget (stops run() on next turn, returns val)
	
	static list<widget*> widgets;	// stack of dialogues, topmost is back
};

class widget_text : public widget
{
protected:
	bool sunken;

	widget_text();
	widget_text(const widget_text& );
	widget_text& operator= (const widget_text& );
public:
	widget_text(int x, int y, int w, int h, const string& text_, widget* parent_ = 0, bool sunken_ = false)
		: widget(x, y, w, h, text_, parent_), sunken(sunken_) {}
	~widget_text() {}
	void draw(void) const;
};

class widget_checkbox : public widget
{
protected:
	bool checked;

	widget_checkbox();
	widget_checkbox(const widget_checkbox& );
	widget_checkbox& operator= (const widget_checkbox& );
public:
	widget_checkbox(int x, int y, int w, int h, const string& text_, widget* parent_ = 0)
		: widget(x, y, w, h, text_, parent_) {}
	~widget_checkbox() {}
	void draw(void) const;
	void on_click(int mx, int my, int mb);
	bool is_checked(void) const { return checked; }
	virtual void on_change(void) {}
};

class widget_button : public widget
{
protected:
	bool pressed;

	widget_button();
	widget_button(const widget_button& );
	widget_button& operator= (const widget_button& );
public:
	widget_button(int x, int y, int w, int h, const string& text_,
		widget* parent_ = 0, const image* backgr = 0) : widget(x, y, w, h, text_, parent_, backgr), pressed(false) {}
	~widget_button() {}
	void draw(void) const;
	void on_click(int mx, int my, int mb);
	void on_release(void);
	bool is_pressed(void) const { return pressed; }
	virtual void on_change(void) {}
};

template<class Obj, class Func>
class widget_caller_button : public widget_button
{
	Obj* obj;
	Func func;
public:
	widget_caller_button(Obj* obj_, Func func_, int x = 0, int y = 0, int w = 0, int h = 0,
		const string& text = "", widget* parent = 0)
		: widget_button(x, y, w, h, text, parent), obj(obj_), func(func_) {}
	~widget_caller_button() {}
	void on_release(void) { widget_button::on_release(); (obj->*func)(); }
};

template<class Obj, class Func, class Arg>
class widget_caller_arg_button : public widget_button
{
	Obj* obj;
	Func func;
	Arg arg;
public:
	widget_caller_arg_button(Obj* obj_, Func func_, Arg arg_, int x = 0, int y = 0, int w = 0, int h = 0,
		const string& text = "", widget* parent = 0)
		: widget_button(x, y, w, h, text, parent), obj(obj_), func(func_), arg(arg_) {}
	~widget_caller_arg_button() {}
	void on_release(void) { widget_button::on_release(); (obj->*func)(arg); }
};

template<class Func>
class widget_func_button : public widget_button
{
	Func func;
public:
	widget_func_button(Func func_, int x = 0, int y = 0, int w = 0, int h = 0,
		const string& text = "", widget* parent = 0)
		: widget_button(x, y, w, h, text, parent), func(func_) {}
	~widget_func_button() {}
	void on_release(void) { widget_button::on_release(); func(); }
};

template<class Func, class Arg>
class widget_func_arg_button : public widget_button
{
	Func func;
	Arg arg;
public:
	widget_func_arg_button(Func func_, Arg arg_, int x = 0, int y = 0, int w = 0, int h = 0,
		const string& text = "", widget* parent = 0)
		: widget_button(x, y, w, h, text, parent), func(func_), arg(arg_) {}
	~widget_func_arg_button() {}
	void on_release(void) { widget_button::on_release(); func(arg); }
};

template<class Obj>
class widget_set_button : public widget_button
{
	Obj& obj;
	Obj value;
public:
	widget_set_button(Obj& obj_, const Obj& val, int x = 0, int y = 0, int w = 0, int h = 0,
		const string& text = "", widget* parent = 0)
		: widget_button(x, y, w, h, text, parent), obj(obj_), value(val) {}
	~widget_set_button() {}
	void on_release(void) { widget_button::on_release(); obj = value; }
};

class widget_menu : public widget
{
protected:
	bool horizontal;
	int entryw, entryh;
	int entryspacing;

	widget_menu() {}
	widget_menu(const widget_menu& );
	widget_menu& operator= (const widget_menu& );
	
	void add_child(widget* w) { widget::add_child(w); };	// clients must use add_entry
	
public:
	widget_menu(int x, int y, int w, int h, const string& text_, bool horizontal_ = false,
		    widget* parent_ = 0);
	void set_entry_spacing(int spc) { entryspacing = spc; }
	void adjust_buttons(unsigned totalsize);	// width or height
	widget_button* add_entry(const string& s, widget_button* wb = 0); // wb's text is always set to s
	int get_selected(void) const;
	~widget_menu() {};
	void draw(void) const;
};

class widget_scrollbar : public widget
{
protected:
	unsigned scrollbarpixelpos;	// current pixel position
	unsigned scrollbarpos;		// current position
	unsigned scrollbarmaxpos;	// maximum number of positions

	unsigned get_max_scrollbarsize(void) const;	// total height of bar in pixels
	unsigned get_scrollbarsize(void) const;	// height of slider bar in pixels
	void compute_scrollbarpixelpos(void);	// recompute value from pos values

	widget_scrollbar();
	widget_scrollbar(const widget_scrollbar& );
	widget_scrollbar& operator= (const widget_scrollbar& );
public:
	widget_scrollbar(int x, int y, int w, int h, widget* parent_ = 0);
	~widget_scrollbar();
	void set_nr_of_positions(unsigned s);
	unsigned get_current_position(void) const;
	void set_current_position(unsigned p);
	void draw(void) const;
	void on_click(int mx, int my, int mb);
	void on_drag(int mx, int my, int rx, int ry, int mb);
	void on_wheel(int wd);
	virtual void on_scroll(void) {}
};

class widget_list : public widget
{
protected:
	list<string> entries;
	unsigned listpos;
	int selected;
	widget_scrollbar* myscrollbar;	// stored also as child

	list<string>::iterator ith(unsigned i);
	list<string>::const_iterator ith(unsigned i) const;

	widget_list();
	widget_list(const widget_list& );
	widget_list& operator= (const widget_list& );
public:
	widget_list(int x, int y, int w, int h, widget* parent_ = 0);
	~widget_list() {};
	void delete_entry(unsigned n);
	void insert_entry(unsigned n, const string& s);
	void append_entry(const string& s);
	void sort_entries(void);
	void make_entries_unique(void);
	string get_entry(unsigned n) const;
	unsigned get_listsize(void) const;
	int get_selected(void) const;
	void set_selected(unsigned n);
	string get_selected_entry(void) const;
	unsigned get_nr_of_visible_entries(void) const;
	void clear(void);
	void draw(void) const;
	void on_click(int mx, int my, int mb);
	void on_drag(int mx, int my, int rx, int ry, int mb);
	virtual void on_sel_change(void) {}
};

class widget_edit : public widget
{
protected:
	unsigned cursorpos;

	widget_edit();
	widget_edit(const widget_edit& );
	widget_edit& operator= (const widget_edit& );
public:
	widget_edit(int x, int y, int w, int h, const string& text_, widget* parent_ = 0)
		: widget(x, y, w, h, text_, parent_), cursorpos(0) {}
	~widget_edit() {}
	void set_text(const string& s) { widget::set_text(s); cursorpos = s.length(); }
	void draw(void) const;
	void on_char(const SDL_keysym& ks);
	virtual void on_enter(void) {}	// run on pressed ENTER-key
	virtual void on_change(void) {}
};

class widget_fileselector : public widget
{
protected:
	widget_list* current_dir;
	widget_edit* current_filename;
	widget_text* current_path;

	void read_current_dir(void);
	unsigned nr_dirs, nr_files;
	
	struct filelist : public widget_list
	{
		void on_click(int mx, int my, int mb) {
			widget_list::on_click(mx, my, mb);
			dynamic_cast<widget_fileselector*>(parent)->listclick();
		}
		filelist(int x, int y, int w, int h) : widget_list(x, y, w, h) {}
		~filelist() {}
	};

	widget_fileselector();
	widget_fileselector(const widget_fileselector& );
	widget_fileselector& operator= (const widget_fileselector& );
public:
	widget_fileselector(int x, int y, int w, int h, const string& text_, widget* parent_ = 0);
	~widget_fileselector() {}
	string get_filename(void) const { return current_path->get_text() + current_filename->get_text(); }
	void listclick(void);
};

#endif
