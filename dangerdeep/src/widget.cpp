// OpenGL based widgets
// (C)+(W) by Thorsten Jordan. See LICENSE

#include "oglext/OglExt.h"
#include <SDL_image.h>

#include "widget.h"
#include "global_data.h"
#include "filehelper.h"
#include "system.h"
#include "texture.h"
#include <set>

widget::theme* widget::globaltheme = 0;
widget* widget::focussed = 0;
int widget::oldmx = 0;
int widget::oldmy = 0;
int widget::oldmb = 0;

list<widget*> widget::widgets;

int widget::theme::frame_size(void) const
{
	return frame[0]->get_height();
}

int widget::theme::icon_size(void) const
{
	return icons[0]->get_height();
}

widget::theme::theme(const char* elements_filename, const char* icons_filename, const font* fnt,
	color tc, color tsc, color tdc) : myfont(fnt), textcol(tc), textselectcol(tsc), textdisabledcol(tdc)
{
	int fw;
	SDL_Surface* tmp;
	tmp = IMG_Load((get_texture_dir() + elements_filename).c_str());
	system::sys().myassert(tmp != 0, string("Unable to load widget theme elements file: ") + get_texture_dir() + elements_filename);
	fw = tmp->h;
	backg = new texture(tmp, 0, 0, fw, fw);
	skbackg = new texture(tmp, fw, 0, fw, fw);
	for (int i = 0; i < 8; ++i)
		frame[i] = new texture(tmp, (i+2)*fw, 0, fw, fw);
	for (int i = 0; i < 8; ++i)
		frameinv[i] = new texture(tmp, (i+10)*fw, 0, fw, fw);
	SDL_FreeSurface(tmp);
	tmp = IMG_Load((get_texture_dir() + icons_filename).c_str());
	system::sys().myassert(tmp != 0, "Unable to load widget theme icons file");
	fw = tmp->h;
	for (int i = 0; i < 4; ++i)
		icons[i] = new texture(tmp, i*fw, 0, fw, fw);
	SDL_FreeSurface(tmp);
}

widget::theme::~theme()
{
	delete backg;
	delete skbackg;
	for (int i = 0; i < 8; ++i) {
		delete frame[i];
		delete frameinv[i];
	}
	for (int i = 0; i < 4; ++i) {
		delete icons[i];
	}
}

void widget::set_theme(widget::theme* t)
{
	delete globaltheme;
	globaltheme = t;
}

widget::theme* widget::replace_theme(widget::theme* t)
{
	widget::theme* tmp = globaltheme;
	globaltheme = t;
	return tmp;
}

widget::widget(int x, int y, int w, int h, const string& text_, widget* parent_, const image* backgr)
	: pos(x, y), size(w, h), text(text_), parent(parent_), background(backgr), enabled(true), retval(-1), closeme(false)
{
}

widget::~widget()
{
	for (list<widget*>::iterator it = children.begin(); it != children.end(); ++it)
		delete *it;
	if (this == focussed) focussed = parent;
}

void widget::add_child(widget* w)
{
	w->set_parent(this);
	w->move_pos(pos);
	children.push_back(w);
}

void widget::remove_child(widget* w)
{
	for (list<widget*>::iterator it = children.begin(); it != children.end(); ++it) {
		if (*it == w) {
			children.erase(it);
			break;
		}
	}
	delete w;
}

void widget::move_pos(const vector2i& p)
{
	pos += p;
	for (list<widget*>::iterator it = children.begin(); it != children.end(); ++it)
		(*it)->move_pos(p);
}

void widget::align(int h, int v)
{
	vector2i sz;
	if (parent) {
		sz = parent->get_size();
	} else {
		sz.x = int(system::sys().get_res_x_2d());
		sz.y = int(system::sys().get_res_y_2d());
	}
	set_pos(vector2i(
		(h < 0) ? 0 : ((h > 0) ? (sz.x-size.x) : ((sz.x-size.x)/2)),
		(v < 0) ? 0 : ((v > 0) ? (sz.y-size.y) : ((sz.y-size.y)/2)) ));
}

void widget::draw(void) const
{
	vector2i p = get_pos();
	draw_area(p.x, p.y, size.x, size.y, /*fixme: replace by property?*/true);
	int fw = globaltheme->frame_size();
	// draw titlebar only when there is a title
	if (text.length() > 0) {
		draw_rect(p.x+fw, p.y+fw, size.x-2*fw, globaltheme->myfont->get_height(), false);
		color tcol = is_enabled() ? globaltheme->textcol : globaltheme->textdisabledcol;
		globaltheme->myfont->print_hc(
			p.x+size.x/2, p.y+globaltheme->frame_size(), text,
			tcol, true);
	}
	for (list<widget*>::const_iterator it = children.begin(); it != children.end(); ++it)
		(*it)->draw();
}

bool widget::compute_focus(void)
{
	focussed = 0;
	if (is_mouse_over()) {
		for (list<widget*>::const_iterator it = children.begin(); it != children.end(); ++it)
			if ((*it)->compute_focus()) return true;
		focussed = this;
		return true;
	}
	return false;
}

bool widget::is_enabled(void) const
{
	bool e = enabled;
	if (parent)
		e = e && parent->is_enabled();
	return e;
}

void widget::enable(void)
{
	enabled = true;
}

void widget::disable(void)
{
	enabled = false;
}

void widget::on_char(void)
{
	system::sys().get_key();	// just ignore it
}

void widget::draw_frame(int x, int y, int w, int h, bool out)
{
	glColor4f(1,1,1,1);
	texture** frelem = (out ? globaltheme->frame : globaltheme->frameinv);
	int fw = globaltheme->frame_size();
	frelem[0]->draw(x, y);
	frelem[1]->draw(x+fw, y, w-2*fw, fw);
	frelem[2]->draw(x+w-fw, y);
	frelem[3]->draw(x+w-fw, y+fw, fw, h-2*fw);
	frelem[4]->draw(x+w-fw, y+h-fw);
	frelem[5]->draw(x+fw, y+h-fw, w-2*fw, fw);
	frelem[6]->draw(x, y+h-fw);
	frelem[7]->draw(x, y+fw, fw, h-2*fw);
}

void widget::prepare_input(void)
{
	class system& sys = system::sys();
	sys.get_mouse_position(oldmx, oldmy);
	oldmb = sys.get_mouse_buttons() & 1;
}

void widget::process_input(bool ignorekeys)
{
	class system& sys = system::sys();
	int mx, my;		
	sys.get_mouse_position(mx, my);
	int mb = sys.get_mouse_buttons() & 1;
	int mclick = mb & ~oldmb;
	int mrelease = oldmb & ~mb;
	oldmb = mb;
	int dx = mx - oldmx;
	int dy = my - oldmy;
	oldmx = mx;
	oldmy = my;

	if (mclick & 1)
		compute_focus();
	if (focussed) {
		if (!ignorekeys) {
			if (!focussed->is_enabled()) { sys.flush_key_queue(); return; }
			if (sys.is_key_in_queue()) focussed->on_char();
		}
		if (mclick & 1) focussed->on_click();
		if (mrelease & 1) focussed->on_release();
		if (mb & 0x18) focussed->on_wheel();
		if ((dx != 0 || dy != 0) && (mb & 1)) focussed->on_drag();
	}
}

void widget::draw_rect(int x, int y, int w, int h, bool out)
{
	glColor4f(1,1,1,1);
	if (out)
		globaltheme->backg->draw(x, y, w, h);
	else
		globaltheme->skbackg->draw(x, y, w, h);
}

void widget::draw_area(int x, int y, int w, int h, bool out) const
{
	int fw = globaltheme->frame_size();
	draw_rect(x+fw, y+fw, w-2*fw, h-2*fw, out);
	if (background) {
		int bw = int(background->get_width());
		int bh = int(background->get_height());
		background->draw(x + (w-bw)/2, y + (h-bh)/2);
	}
	draw_frame(x, y, w, h, out);
}

bool widget::is_mouse_over(void) const
{
	int mx, my;
	system::sys().get_mouse_position(mx, my);
	vector2i p = get_pos();
	return (mx >= p.x && my >= p.y && mx < p.x+size.x && my < p.y + size.y);
}

widget* widget::create_dialogue_ok(widget* parent_, const string& title, const string& text)
{
	unsigned res_x = system::sys().get_res_x_2d();
	unsigned res_y = system::sys().get_res_y_2d();
	widget* w = new widget(res_x/4, res_y/4, res_x/2, res_y/2, title, parent_);
	w->add_child(new widget_text(32, 64, res_x/2-64, res_y/2-128, text));
	int fw = globaltheme->frame_size();
	int fh = int(globaltheme->myfont->get_height());
	int butw = 4*fh+2*fw;
	w->add_child(new widget_caller_arg_button<widget, void (widget::*)(int), int>(w, &widget::close, 1, res_x/4-butw/2, res_y/2-64, butw, fh+4*fw, "Ok"));
	return w;
}

widget* widget::create_dialogue_ok_cancel(widget* parent_, const string& title, const string& text)
{
	unsigned res_x = system::sys().get_res_x_2d();
	unsigned res_y = system::sys().get_res_y_2d();
	widget* w = new widget(res_x/4, res_y/4, res_x/2, res_y/2, title, parent_);
	w->add_child(new widget_text(32, 64, res_x/2-64, res_y/2-128, text));
	int fw = globaltheme->frame_size();
	int fh = int(globaltheme->myfont->get_height());
	int butw = 4*fh+2*fw;
	w->add_child(new widget_caller_arg_button<widget, void (widget::*)(int), int>(w, &widget::close, 1, res_x/8-butw/2, res_y/2-64, butw, fh+4*fw, "Ok"));
	w->add_child(new widget_caller_arg_button<widget, void (widget::*)(int), int>(w, &widget::close, 0, 3*res_x/8-butw/2, res_y/2-64, butw, fh+4*fw, "Cancel"));
	return w;
}

int widget::run(unsigned timeout)
{
	glClearColor(0, 0, 0, 0);
	widget* myparent = parent;	// store parent info and unlink chain to parent
	parent = 0;
	if (myparent) myparent->disable();
	closeme = false;
	widgets.push_back(this);
	class system& sys = system::sys();
	unsigned endtime = sys.millisec() + timeout;
	while (!closeme) {
		unsigned time = sys.millisec();
		if (timeout != 0 && time > endtime) break;
		
		sys.poll_event_queue();
		glClear(GL_COLOR_BUFFER_BIT);
		sys.prepare_2d_drawing();
		glColor4f(1,1,1,1);
		for (list<widget*>::iterator it = widgets.begin(); it != widgets.end(); ++it)
			(*it)->draw();
		sys.unprepare_2d_drawing();
		process_input();
		sys.swap_buffers();
	}
	widgets.pop_back();
	if (myparent) myparent->enable();
	parent = myparent;
	return retval;
}

void widget::close(int val)
{
	retval = val;
	closeme = true;
}

widget_button* widget_menu::add_entry(const string& s, widget_button* wb)
{
	int x, y, w, h;
	if (horizontal) {
		x = children.size() * (entryw + entryspacing);
		y = 0;
		w = entryw;
		h = entryh;
		size.x += entryw;
		size.y = entryh;
		if (children.size() > 0) size.x += entryspacing;
	} else {
		x = 0;
		y = children.size() * (entryh + entryspacing);
		w = entryw;
		h = entryh;
		size.x = entryw;
		size.y += entryh;
		if (children.size() > 0) size.y += entryspacing;
	}
	if (wb) {
		wb->set_size(vector2i(w, h));
		wb->set_pos(vector2i(x, y));
		wb->set_text(s);
		wb->set_parent(this);
	} else {
		wb = new widget_button(x, y, w, h, s, this);
	}
	add_child(wb);
	return wb;
}

int widget_menu::get_selected(void) const
{
	int sel = 0;
	for (list<widget*>::const_iterator it = children.begin(); it != children.end(); ++it, ++sel)
		if ((dynamic_cast<widget_button*>(*it))->is_pressed())
			return sel;
	return -1;
}

void widget_menu::draw(void) const
{
	vector2i p = get_pos();
	for (list<widget*>::const_iterator it = children.begin(); it != children.end(); ++it)
		(*it)->draw();
}

void widget_menu::adjust_buttons(unsigned totalsize)
{
	// fixme: if there's not enough space for all buttons, nothing is adjusted.
	// thats bad - but one can't do anything anyway
	if (horizontal) {
		int textw = 0;
		int fw = globaltheme->frame_size();
		int nrbut = int(children.size());
		int longest = 0;
		for (list<widget*>::const_iterator it = children.begin(); it != children.end(); ++it) {
			int w = int(globaltheme->myfont->get_size((*it)->get_text()).first);
			textw += w;
			if (w > longest) longest = w;
		}
		int framew = 2*fw*nrbut;
		int spaceleft = int(totalsize) - ((longest+2*fw)*nrbut + framew + (nrbut-1)*entryspacing);
		if (spaceleft > 0) {	// equi distant buttons
			size.x = int(totalsize);
			int spc = spaceleft / nrbut;
			int runpos = 0;
			for (list<widget*>::const_iterator it = children.begin(); it != children.end(); ++it) {
				int mytextw = longest+2*fw;
				(*it)->set_pos(pos + vector2i(runpos, 0));
				(*it)->set_size(vector2i(mytextw+2*fw+spc, entryh));
				runpos += mytextw+2*fw+spc + entryspacing;
			}
		} else {
			spaceleft = int(totalsize) - (textw + framew + (nrbut-1)*entryspacing);
			if (spaceleft > 0) {	// space left?
				size.x = int(totalsize);
				int spc = spaceleft / nrbut;
				int runpos = 0;
				for (list<widget*>::const_iterator it = children.begin(); it != children.end(); ++it) {
					int mytextw = int(globaltheme->myfont->get_size((*it)->get_text()).first);
					(*it)->set_pos(pos + vector2i(runpos, 0));
					(*it)->set_size(vector2i(mytextw+2*fw+spc, entryh));
					runpos += mytextw+2*fw+spc + entryspacing;
				}
			}
		}
	} else {
		// fixme: todo
	}
}

void widget_text::draw(void) const
{
	vector2i p = get_pos();
	class system& sys = system::sys();
	if (sunken) {
		draw_area(p.x, p.y, size.x, size.y, false);
		int fw = globaltheme->frame_size();
		globaltheme->myfont->print_wrapped(p.x+2*fw, p.y+2*fw, size.x-4*fw, 0, text, globaltheme->textcol, true);
	} else {
		globaltheme->myfont->print_wrapped(p.x, p.y, size.x, 0, text, globaltheme->textcol, true);
	}
}

void widget_button::draw(void) const
{
	vector2i p = get_pos();
	bool mover = is_enabled() && is_mouse_over();
	draw_area(p.x, p.y, size.x, size.y, !mover);
	color col = (is_enabled() ? (is_mouse_over() ? globaltheme->textselectcol : globaltheme->textcol) : globaltheme->textdisabledcol);
	globaltheme->myfont->print_c(p.x+size.x/2, p.y+size.y/2, text, col, true);
}

void widget_button::on_click(void)
{
	pressed = true;
	on_change();
}

void widget_button::on_release(void)
{
	pressed = false;
	on_change();
}

widget_scrollbar::widget_scrollbar(int x, int y, int w, int h, widget* parent_)
	: widget(x, y, w, h, "", parent_), scrollbarpos(0), scrollbarmaxpos(0)
{
}

widget_scrollbar::~widget_scrollbar()
{
}

void widget_scrollbar::set_nr_of_positions(unsigned s)
{
	scrollbarmaxpos = s;
	if (scrollbarmaxpos == 0)
		scrollbarpos = 0;
	else if (scrollbarpos >= scrollbarmaxpos)
		scrollbarpos = scrollbarmaxpos - 1;
	compute_scrollbarpixelpos();
}

unsigned widget_scrollbar::get_current_position(void) const
{
	return scrollbarpos;
}

void widget_scrollbar::set_current_position(unsigned p)
{
	if (p < scrollbarmaxpos) {
		scrollbarpos = p;
		compute_scrollbarpixelpos();
	}
}

unsigned widget_scrollbar::get_max_scrollbarsize(void) const
{
	return size.y-globaltheme->icons[0]->get_height()-globaltheme->icons[1]->get_height()-4*globaltheme->frame_size();
}

unsigned widget_scrollbar::get_scrollbarsize(void) const
{
	unsigned msbs = get_max_scrollbarsize();
	if (scrollbarmaxpos == 0)
		return msbs;
	else
		return msbs/2 + msbs/(1+scrollbarmaxpos);
}

void widget_scrollbar::compute_scrollbarpixelpos(void)
{
	if (scrollbarmaxpos <= 1) scrollbarpixelpos = 0;
	else scrollbarpixelpos = (get_max_scrollbarsize() - get_scrollbarsize()) * scrollbarpos/(scrollbarmaxpos-1);
}

void widget_scrollbar::draw(void) const
{
	vector2i p = get_pos();
	int fw = globaltheme->frame_size();
	draw_frame(p.x, p.y, globaltheme->icons[0]->get_width()+2*fw, globaltheme->icons[0]->get_height()+2*fw, true);
	draw_frame(p.x, p.y+size.y-globaltheme->icons[1]->get_height()-2*fw, globaltheme->icons[1]->get_width()+2*fw, globaltheme->icons[1]->get_height()+2*fw, true);
	globaltheme->icons[0]->draw(p.x+fw, p.y+fw);
	globaltheme->icons[1]->draw(p.x+fw, p.y+size.y-globaltheme->icons[1]->get_height()-fw);
	draw_area(p.x, p.y + globaltheme->icons[0]->get_height() + 2*fw, globaltheme->icons[0]->get_width()+2*fw, get_max_scrollbarsize(), false);
	draw_area(p.x, p.y + globaltheme->icons[0]->get_height() + 2*fw + scrollbarpixelpos, globaltheme->icons[0]->get_width()+2*fw, get_scrollbarsize(), true);
}

void widget_scrollbar::on_click(void)
{
	unsigned oldpos = scrollbarpos;
	vector2i p = get_pos();
	int mx, my, mb;
	system::sys().get_mouse_position(mx, my);
	mb = system::sys().get_mouse_buttons();
	if (my < int(p.y + globaltheme->icons[0]->get_height() + 4)) {
		if (mb != 0) {
			if (scrollbarpos > 0) {
				--scrollbarpos;
				compute_scrollbarpixelpos();
			}
		}
	} else if (my >= int(p.y + size.y - globaltheme->icons[1]->get_height() - 4)) {
		if (mb != 0) {
			if (scrollbarpos+1 < scrollbarmaxpos) {
				++scrollbarpos;
				compute_scrollbarpixelpos();
			}
		}
	} else {
		int rx, ry;
		system::sys().get_mouse_motion(rx, ry);
		if (mb != 0 && ry != 0) {
			if (scrollbarmaxpos > 1) {
				int msbp = get_max_scrollbarsize() - get_scrollbarsize();
				int sbpp = scrollbarpixelpos;
				sbpp += ry;
				if (sbpp < 0) sbpp = 0;
				else if (sbpp > msbp) sbpp = msbp;
				scrollbarpixelpos = sbpp;
				scrollbarpos = scrollbarpixelpos * (scrollbarmaxpos-1) / msbp;
			}
		}
	}
	if (oldpos != scrollbarpos)
		on_scroll();
}

void widget_scrollbar::on_drag(void)
{
	on_click();
}

void widget_scrollbar::on_wheel(void)
{
	widget::process_input();
	if (!is_enabled()) return;
	unsigned oldpos = scrollbarpos;
	int mb = system::sys().get_mouse_buttons();
	if (mb & 0x8) {
		if (scrollbarpos > 0) {
			--scrollbarpos;
			compute_scrollbarpixelpos();
		}
	}
	if (mb != 0x10) {
		if (scrollbarpos+1 < scrollbarmaxpos) {
			++scrollbarpos;
			compute_scrollbarpixelpos();
		}
	}
	if (oldpos != scrollbarpos)
		on_scroll();
}

widget_list::widget_list(int x, int y, int w, int h, widget* parent_)
	: widget(x, y, w, h, "", parent_), listpos(0), selected(-1)
{
	struct wls : public widget_scrollbar
	{
		unsigned& p;
		void on_scroll(void) { p = get_current_position(); }
		wls(unsigned& p_, int x, int y, int w, int h, widget* parent) : widget_scrollbar(x,y,w,h,parent), p(p_) {}
		~wls() {};
	};
	int fw = globaltheme->frame_size();
	myscrollbar = new wls(listpos, size.x-3*fw-globaltheme->icons[0]->get_width(), fw, globaltheme->icons[0]->get_width()+2*fw, size.y-2*fw, this);
	add_child(myscrollbar);
}

list<string>::iterator widget_list::ith(unsigned i)
{
	list<string>::iterator it = entries.begin();
	while (it != entries.end() && i > 0) {
		--i;
		++it;
	}
	return it;
}

list<string>::const_iterator widget_list::ith(unsigned i) const
{
	list<string>::const_iterator it = entries.begin();
	while (it != entries.end() && i > 0) {
		--i;
		++it;
	}
	return it;
}

void widget_list::delete_entry(unsigned n)
{
	list<string>::iterator it = ith(n);
	if (it != entries.end())
		entries.erase(it);
	unsigned es = entries.size();
	if (es == 0) selected = -1;	// remove selection
	else if (es == 1) set_selected(0);	// set to first entry
	else on_sel_change();
	unsigned ve = get_nr_of_visible_entries();
	if (es > ve)
		myscrollbar->set_nr_of_positions(es - ve + 1);
}

void widget_list::insert_entry(unsigned n, const string& s)
{
	list<string>::iterator it = ith(n);
	if (it != entries.end())
		entries.insert(it, s);
	else
		entries.push_back(s);
	unsigned es = entries.size();
	if (es == 1) set_selected(0);	// set to first entry
	else on_sel_change();
	unsigned ve = get_nr_of_visible_entries();
	if (es > ve)
		myscrollbar->set_nr_of_positions(es - ve + 1);
}

void widget_list::append_entry(const string& s)
{
	entries.push_back(s);
	unsigned es = entries.size();
	if (es == 1) set_selected(0);	// set to first entry
	else on_sel_change();
	unsigned ve = get_nr_of_visible_entries();
	if (es > ve)
		myscrollbar->set_nr_of_positions(es - ve + 1);
}

void widget_list::sort_entries(void)
{
	entries.sort();
	on_sel_change();
}

void widget_list::make_entries_unique(void)
{
	unique(entries.begin(), entries.end());
	unsigned es = entries.size();
	if (es == 1) set_selected(0);	// set to first entry
	else on_sel_change();
	unsigned ve = get_nr_of_visible_entries();
	if (es > ve)
		myscrollbar->set_nr_of_positions(es - ve + 1);
}

string widget_list::get_entry(unsigned n) const
{
	for (list<string>::const_iterator i = entries.begin(); i != entries.end(); ++i, --n)
		if (n == 0) return *i;
	return "";
}

unsigned widget_list::get_listsize(void) const
{
	return entries.size();
}

int widget_list::get_selected(void) const
{
	return selected;
}

void widget_list::set_selected(unsigned n)
{
	if (n < entries.size()) {
		selected = int(n);
		unsigned ve = get_nr_of_visible_entries();
		if (listpos > n || n >= listpos + ve) {
			listpos = n;
			myscrollbar->set_current_position(n - ve);
		}
		on_sel_change();
	}
}

string widget_list::get_selected_entry(void) const
{
	if (selected >= 0)
		return get_entry(selected);
	return "";
}

unsigned widget_list::get_nr_of_visible_entries(void) const
{
	return (size.y - 2*globaltheme->frame_size()) / globaltheme->myfont->get_height();
}

void widget_list::clear(void)
{
	listpos = 0;
	selected = -1;
	entries.clear();
	on_sel_change();
}

void widget_list::draw(void) const
{
	vector2i p = get_pos();
	draw_area(p.x, p.y, size.x, size.y, false);
	list<string>::const_iterator it = ith(listpos);
	int fw = globaltheme->frame_size();
	unsigned maxp = get_nr_of_visible_entries();
	color tcol = is_enabled() ? globaltheme->textcol : globaltheme->textdisabledcol;
	bool scrollbarvisible = (entries.size() > maxp);
	for (unsigned lp = 0; it != entries.end() && lp < maxp; ++it, ++lp) {
		if (selected == int(lp + listpos)) {
			int width = size.x-2*fw;
			if (scrollbarvisible)
				width -= 3*fw+globaltheme->icons[0]->get_width();
			color::white().set_gl_color();
			globaltheme->backg->draw(p.x+fw, p.y + fw + lp*globaltheme->myfont->get_height(), width, globaltheme->myfont->get_height());
		}
		globaltheme->myfont->print(p.x+fw, p.y+fw + lp*globaltheme->myfont->get_height(), *it, tcol, true);
	}
	if (entries.size() > maxp)
		myscrollbar->draw();
}

void widget_list::on_click(void)
{
	vector2i p = get_pos();
	int mx, my;
	system::sys().get_mouse_position(mx, my);
	int mbc = system::sys().get_mouse_buttons();	// fixme
	int oldselected = selected;
	if (mbc & 1) {
		int fw = globaltheme->frame_size();
		int sp = (my - p.y - fw)/int(globaltheme->myfont->get_height());
		if (sp < 0) sp = 0;
		selected = int(listpos) + sp;
		if (unsigned(selected) >= entries.size()) selected = int(entries.size())-1;
	}
	if (oldselected != selected)
		on_sel_change();
}

void widget_list::on_drag(void)
{
	on_click();
}

void widget_edit::draw(void) const
{
	vector2i p = get_pos();
	draw_area(p.x, p.y, size.x, size.y, false);
	int fw = globaltheme->frame_size();
	globaltheme->myfont->print_vc(p.x+fw, p.y+size.y/2, text, is_enabled() ? globaltheme->textcol : globaltheme->textdisabledcol, true);
	pair<unsigned, unsigned> sz = globaltheme->myfont->get_size(text.substr(0, cursorpos));
	system::sys().no_tex();
	globaltheme->textcol.set_gl_color();
	if (this == (const widget*)focussed)
		system::sys().draw_rectangle(p.x+fw+sz.first, p.y+size.y/4, fw/2, size.y/2);
}

void widget_edit::on_char(void)
{
	SDL_keysym ks = system::sys().get_key();
	int c = ks.sym;
	unsigned l = text.length();
	unsigned textw = globaltheme->myfont->get_size(text).first;
	if (c == SDLK_LEFT && cursorpos > 0) {
		--cursorpos;
	} else if (c == SDLK_RIGHT && cursorpos < l) {
		++cursorpos;
	} else if (c == SDLK_HOME) {
		cursorpos = 0;
	} else if (c == SDLK_END) {
		cursorpos = l;
	} else if (c == SDLK_RETURN) {
		on_enter();
	} else if (c >= 32 && c <= 255 && c != 127) {
		c = ks.unicode & 0xff;
		char tx[2] = { c, 0 };
		string stx(tx);
		unsigned stxw = globaltheme->myfont->get_size(stx).first;
		if (int(textw + stxw + 8) < size.x) {
			if (cursorpos < l) {
				text.insert(cursorpos, stx);
			} else {
				text += stx;
			}
			++cursorpos;
			on_change();
		}
	} else if (c == SDLK_DELETE && cursorpos < l) {
		text.erase(cursorpos, 1);
		on_change();
	} else if (c == SDLK_BACKSPACE && cursorpos > 0) {
		text = text.erase(cursorpos-1, 1);
		--cursorpos;
		on_change();
	}
}

widget_fileselector::widget_fileselector(int x, int y, int w, int h, const string& text_, widget* parent_)
	: widget(x, y, w, h, text_, parent_)
{
	current_path = new widget_text(120, 40, size.x-140, 32, get_current_directory());
	current_dir = new filelist(120, 80, size.x-140, size.y-136);
	current_filename = new widget_edit(120, size.y - 52, size.x-140, 32, "");
	add_child(current_path);
	add_child(current_dir);
	add_child(current_filename);
	add_child(new widget_text(20, 40, 80, 32, "Path:"));
	add_child(new widget_caller_arg_button<widget, void (widget::*)(int), int>(this, &widget::close, 1, 20, 80, 80, 32, "Ok"));
	add_child(new widget_caller_arg_button<widget, void (widget::*)(int), int>(this, &widget::close, 0, 20, 120, 80, 32, "Cancel"));
	
	read_current_dir();
}

void widget_fileselector::read_current_dir(void)
{
	current_dir->clear();
	directory dir = open_dir(current_path->get_text());
	system::sys().myassert(dir.is_valid(), "[widget_fileselector::read_current_dir] could not open directory");
	set<string> dirs, files;
	while (true) {
		string e = read_dir(dir);
		if (e.length() == 0) break;
		if (e[0] == '.') continue;	// avoid . .. and hidden files
		if (is_directory(current_path->get_text() + e)) {
			dirs.insert(e);
		} else {
			files.insert(e);
		}
	}
	close_dir(dir);
	nr_dirs = dirs.size()+1;
	nr_files = files.size();
	current_dir->append_entry("[..]");
	for (set<string>::iterator it = dirs.begin(); it != dirs.end(); ++it)
		current_dir->append_entry(string("[") + *it + string("]"));
	for (set<string>::iterator it = files.begin(); it != files.end(); ++it)
		current_dir->append_entry(*it);
}

void widget_fileselector::listclick(void)
{
	int n = current_dir->get_selected();
	if (n < 0 || unsigned(n) > nr_dirs + nr_files) return;
	string p = current_path->get_text();
	string filesep = p.substr(p.length()-1, 1);
	if (n == 0) {
		string::size_type st = p.rfind(filesep, p.length()-2);
		if (st != string::npos) {
			p = p.substr(0, st) + filesep;
		}
		current_path->set_text(p);
		read_current_dir();
	} else if (unsigned(n) < nr_dirs) {
		string p = current_path->get_text();
		string d = current_dir->get_selected_entry();
		d = d.substr(1, d.length()-2);	// remove [ ] characters
		p += d + filesep;
		current_path->set_text(p);
		read_current_dir();
	} else {
		current_filename->set_text(current_dir->get_selected_entry());
	}
}
