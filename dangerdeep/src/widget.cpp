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

// OpenGL based widgets
// (C)+(W) by Thorsten Jordan. See LICENSE

#include "oglext/OglExt.h"
#include <SDL_image.h>

#include "widget.h"
#include "global_data.h"
#include "filehelper.h"
#include "system.h"
#include "texture.h"
#include "model.h"
#include <set>
#include <sstream>

widget::theme* widget::globaltheme = 0;
widget* widget::focussed = 0;
widget* widget::mouseover = 0;
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
	sys().myassert(tmp != 0, string("Unable to load widget theme elements file: ") + get_texture_dir() + elements_filename);
	fw = tmp->h;
	backg = new texture(tmp, 0, 0, fw, fw);
	skbackg = new texture(tmp, fw, 0, fw, fw);
	for (int i = 0; i < 8; ++i)
		frame[i] = new texture(tmp, (i+2)*fw, 0, fw, fw);
	for (int i = 0; i < 8; ++i)
		frameinv[i] = new texture(tmp, (i+10)*fw, 0, fw, fw);
	SDL_FreeSurface(tmp);
	tmp = IMG_Load((get_texture_dir() + icons_filename).c_str());
	sys().myassert(tmp != 0, "Unable to load widget theme icons file");
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
	: pos(x, y), size(w, h), text(text_), parent(parent_), background(backgr),
	  background_tex(0), enabled(true), retval(-1), closeme(false)
{
}

widget::~widget()
{
	for (list<widget*>::iterator it = children.begin(); it != children.end(); ++it)
		delete *it;
	if (this == focussed) focussed = parent;
	if (this == mouseover) mouseover = 0;
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

void widget::remove_children(void)
{
	for (list<widget*>::iterator it = children.begin(); it != children.end(); ++it) {
		delete *it;
	}
	children.clear();
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
		sz.x = int(sys().get_res_x_2d());
		sz.y = int(sys().get_res_y_2d());
	}
	set_pos(vector2i(
		(h < 0) ? 0 : ((h > 0) ? (sz.x-size.x) : ((sz.x-size.x)/2)),
		(v < 0) ? 0 : ((v > 0) ? (sz.y-size.y) : ((sz.y-size.y)/2)) ));
}

void widget::draw() const
{
	redrawme = false;
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
	// fixme: if childs have the same size as parent, don't draw parent?
	// i.e. do not use stacking then...
	// maybe make chooseable via argument of run()
	for (list<widget*>::const_iterator it = children.begin(); it != children.end(); ++it)
		(*it)->draw();
}

bool widget::compute_focus(int mx, int my)
{
	focussed = 0;
	if (is_mouse_over(mx, my)) {
		for (list<widget*>::const_iterator it = children.begin(); it != children.end(); ++it)
			if ((*it)->compute_focus(mx, my)) return true;
		focussed = this;
		return true;
	}
	return false;
}

bool widget::compute_mouseover(int mx, int my)
{
	mouseover = 0;
	if (is_mouse_over(mx, my)) {
		for (list<widget*>::const_iterator it = children.begin(); it != children.end(); ++it)
			if ((*it)->compute_mouseover(mx, my)) return true;
		mouseover = this;
		return true;
	}
	return false;
}

bool widget::is_enabled() const
{
	bool e = enabled;
	if (parent)
		e = e && parent->is_enabled();
	return e;
}

void widget::enable()
{
	enabled = true;
}

void widget::disable()
{
	enabled = false;
}

void widget::redraw()
{
	redrawme = true;
	if (parent) parent->redraw();
}

void widget::on_char(const SDL_keysym& ks)
{
	// we can't handle it, so pass it to the parent
	if (parent) parent->on_char(ks);
}

void widget::on_wheel(int wd)
{
	// we can't handle it, so pass it to the parent
	if (parent) parent->on_wheel(wd);
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



void widget::process_input(const SDL_Event& event)
{
	switch (event.type) {
	case SDL_KEYDOWN:
		if (focussed && focussed->is_enabled())
			focussed->on_char(event.key.keysym);
		break;

	case SDL_KEYUP:	// ignore for now
		break;

	case SDL_MOUSEBUTTONDOWN:
		if (event.button.button == SDL_BUTTON_LEFT) {
			compute_focus(event.button.x, event.button.y);
			if (focussed) focussed->on_click(event.button.x, event.button.y, SDL_BUTTON_LMASK);
		} else if (event.button.button == SDL_BUTTON_RIGHT) {
			compute_focus(event.button.x, event.button.y);
			if (focussed) focussed->on_click(event.button.x, event.button.y, SDL_BUTTON_RMASK);
		} else if (event.button.button == SDL_BUTTON_MIDDLE) {
			compute_focus(event.button.x, event.button.y);
			if (focussed) focussed->on_click(event.button.x, event.button.y, SDL_BUTTON_MMASK);
		} else if (event.button.button == SDL_BUTTON_WHEELUP) {
			if (focussed) focussed->on_wheel(1);
		} else if (event.button.button == SDL_BUTTON_WHEELDOWN) {
			if (focussed) focussed->on_wheel(2);
		}
		break;

	case SDL_MOUSEBUTTONUP:
		if (event.button.button == SDL_BUTTON_LEFT) {
			if (focussed) focussed->on_release();
		}
		break;

	case SDL_MOUSEMOTION:
		compute_mouseover(event.motion.x, event.motion.y);
		if (focussed) focussed->on_drag(event.motion.x, event.motion.y,
						event.motion.xrel, event.motion.yrel,
						event.motion.state);
		break;
	}
}

void widget::process_input(const list<SDL_Event>& events)
{
	for (list<SDL_Event>::const_iterator it = events.begin(); it != events.end(); ++it) {
		process_input(*it);
	}
}

bool widget::check_for_mouse_event(const SDL_Event& event)
{
	if (event.type == SDL_MOUSEMOTION && is_mouse_over(event.motion.x, event.motion.y)) {
		process_input(event);
		return true;
	}
	if (event.type == SDL_MOUSEBUTTONDOWN && is_mouse_over(event.button.x, event.button.y)) {
		process_input(event);
		return true;
	}
	if (event.type == SDL_MOUSEBUTTONUP && is_mouse_over(event.button.x, event.button.y)) {
		process_input(event);
		return true;
	}
	return false;
}


void widget::draw_rect(int x, int y, int w, int h, bool out)
{
	glColor4f(1,1,1,1);
	if (out)
		globaltheme->backg->draw(x, y, w, h);
	else
		globaltheme->skbackg->draw(x, y, w, h);
}

void widget::draw_line(int x1, int y1, int x2, int y2)
{
	glBindTexture(GL_TEXTURE_2D, 0);
	globaltheme->textcol.set_gl_color();
	glBegin(GL_LINES);
	glVertex2i(x1, y1);
	glVertex2i(x2, y2);
	glEnd();
}

void widget::draw_area(int x, int y, int w, int h, bool out) const
{
	int fw = globaltheme->frame_size();
	draw_rect(x+fw, y+fw, w-2*fw, h-2*fw, out);
	if (background) {
		int bw = int(background->get_width());
		int bh = int(background->get_height());
		background->draw(x + (w-bw)/2, y + (h-bh)/2);
	} else if (background_tex) {
		background_tex->draw_tiles(x, y, w, h);
	}
	draw_frame(x, y, w, h, out);
}

void widget::draw_area_col(int x, int y, int w, int h, bool out, color c) const
{
	int fw = globaltheme->frame_size();
	glBindTexture(GL_TEXTURE_2D, 0);
	c.set_gl_color();
	glBegin(GL_QUADS);
	glVertex2i(x, y);
	glVertex2i(x, y+h);
	glVertex2i(x+w, y+h);
	glVertex2i(x+w, y);
	glEnd();
	draw_frame(x, y, w, h, out);
}

bool widget::is_mouse_over(int mx, int my) const
{
	vector2i p = get_pos();
	return (mx >= p.x && my >= p.y && mx < p.x+size.x && my < p.y + size.y);
}

widget* widget::create_dialogue_ok(widget* parent_, const string& title, const string& text)
{
	unsigned res_x = sys().get_res_x_2d();
	unsigned res_y = sys().get_res_y_2d();
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
	unsigned res_x = sys().get_res_x_2d();
	unsigned res_y = sys().get_res_y_2d();
	widget* w = new widget(res_x/4, res_y/4, res_x/2, res_y/2, title, parent_);
	w->add_child(new widget_text(32, 64, res_x/2-64, res_y/2-128, text));
	int fw = globaltheme->frame_size();
	int fh = int(globaltheme->myfont->get_height());
	int butw = 4*fh+2*fw;
	w->add_child(new widget_caller_arg_button<widget, void (widget::*)(int), int>(w, &widget::close, 1, res_x/8-butw/2, res_y/2-64, butw, fh+4*fw, "Ok"));
	w->add_child(new widget_caller_arg_button<widget, void (widget::*)(int), int>(w, &widget::close, 0, 3*res_x/8-butw/2, res_y/2-64, butw, fh+4*fw, "Cancel"));
	return w;
}

int widget::run(unsigned timeout, bool do_stacking)
{
	unsigned short inited = false; // draw first, then only draw when an event occurred
	glClearColor(0, 0, 0, 0);
	widget* myparent = parent;	// store parent info and unlink chain to parent
	parent = 0;
	if (myparent) myparent->disable();
	closeme = false;
	widgets.push_back(this);
	unsigned endtime = sys().millisec() + timeout;
	focussed = this;
	while (!closeme) {
		unsigned time = sys().millisec();
		if (timeout != 0 && time > endtime) break;

		list<SDL_Event> events = sys().poll_event_queue();
		if (!redrawme && inited && events.size() == 0) {
			SDL_Delay(50);
			continue;
		}
		inited = true;
		glClear(GL_COLOR_BUFFER_BIT);
		sys().prepare_2d_drawing();
		glColor4f(1,1,1,1);
		if (do_stacking) {
			for (list<widget*>::iterator it = widgets.begin(); it != widgets.end();
			     ++it)
				(*it)->draw();
		} else {
			draw();
		}
		sys().unprepare_2d_drawing();
		process_input(events);
		sys().swap_buffers();
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

widget_menu::widget_menu(int x, int y, int w, int h, const string& text_, bool horizontal_,
			 widget* parent_)
		: widget(x, y, 0, 0, text_, parent_), horizontal(horizontal_),
		entryw(w), entryh(h), entryspacing(16)
{
	if (text.length() > 0) {
		size.x = entryw;
		size.y = entryh;
	}
}

widget_button* widget_menu::add_entry(const string& s, widget_button* wb)
{
	int x, y, w, h;
	unsigned mult = children.size();
	if (text.length() > 0)
		++mult;
	if (horizontal) {
		x = mult * (entryw + entryspacing);
		y = 0;
		w = entryw;
		h = entryh;
		size.x += entryw;
		size.y = entryh;
		if (mult > 0) size.x += entryspacing;
	} else {
		x = 0;
		y = mult * (entryh + entryspacing);
		w = entryw;
		h = entryh;
		size.x = entryw;
		size.y += entryh;
		if (mult > 0) size.y += entryspacing;
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

int widget_menu::get_selected() const
{
	int sel = 0;
	for (list<widget*>::const_iterator it = children.begin(); it != children.end(); ++it, ++sel)
		if ((dynamic_cast<widget_button*>(*it))->is_pressed())
			return sel;
	return -1;
}

void widget_menu::draw() const
{
	vector2i p = get_pos();
	// draw tile bar if there is text
	if (text.length() > 0) {
		draw_area(p.x, p.y, entryw, entryh, true);
		draw_area(p.x, p.y, entryw, entryh, false);
		globaltheme->myfont->print_c(p.x+entryw/2, p.y+entryh/2, text,
					     globaltheme->textcol, true);
	}
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
			int w = int(globaltheme->myfont->get_size((*it)->get_text()).x);
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
					int mytextw = int(globaltheme->myfont->get_size((*it)->get_text()).x);
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

void widget_text::draw() const
{
	vector2i p = get_pos();
	if (sunken) {
		draw_area(p.x, p.y, size.x, size.y, false);
		int fw = globaltheme->frame_size();
		globaltheme->myfont->print_wrapped(p.x+2*fw, p.y+2*fw, size.x-4*fw, 0, text, globaltheme->textcol, true);
	} else {
		globaltheme->myfont->print_wrapped(p.x, p.y, size.x, 0, text, globaltheme->textcol, true);
	}
}

void widget_button::draw() const
{
	vector2i p = get_pos();
	bool mover = is_enabled() && mouseover == this;
	draw_area(p.x, p.y, size.x, size.y, !mover);
	color col = (is_enabled() ? (mouseover == this ? globaltheme->textselectcol : globaltheme->textcol) : globaltheme->textdisabledcol);
	globaltheme->myfont->print_c(p.x+size.x/2, p.y+size.y/2, text, col, true);
}

void widget_button::on_click(int mx, int my, int mb)
{
	pressed = true;
	on_change();
}

void widget_button::on_release()
{
	pressed = false;
	on_change();
}

widget_scrollbar::widget_scrollbar(int x, int y, int w, int h, widget* parent_)
	: widget(x, y, w, h, "", parent_), scrollbarpos(0), scrollbarmaxpos(0)
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

unsigned widget_scrollbar::get_current_position() const
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

unsigned widget_scrollbar::get_max_scrollbarsize() const
{
	return size.y-globaltheme->icons[0]->get_height()-globaltheme->icons[1]->get_height()-4*globaltheme->frame_size();
}

unsigned widget_scrollbar::get_scrollbarsize() const
{
	unsigned msbs = get_max_scrollbarsize();
	if (scrollbarmaxpos == 0)
		return msbs;
	else
		return msbs/2 + msbs/(1+scrollbarmaxpos);
}

void widget_scrollbar::compute_scrollbarpixelpos()
{
	if (scrollbarmaxpos <= 1) scrollbarpixelpos = 0;
	else scrollbarpixelpos = (get_max_scrollbarsize() - get_scrollbarsize()) * scrollbarpos/(scrollbarmaxpos-1);
}

void widget_scrollbar::draw() const
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

void widget_scrollbar::on_click(int mx, int my, int mb)
{
	unsigned oldpos = scrollbarpos;
	vector2i p = get_pos();
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
	}
	if (oldpos != scrollbarpos)
		on_scroll();
}

void widget_scrollbar::on_drag(int mx, int my, int rx, int ry, int mb)
{
	unsigned oldpos = scrollbarpos;
	vector2i p = get_pos();
	if ((my >= int(p.y + globaltheme->icons[0]->get_height() + 4)) &&
	    (my < int(p.y + size.y - globaltheme->icons[1]->get_height() - 4))) {
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
		if (oldpos != scrollbarpos)
			on_scroll();
	}
}

void widget_scrollbar::on_wheel(int wd)
{
	unsigned oldpos = scrollbarpos;
	if (wd == 1) {
		if (scrollbarpos > 0) {
			--scrollbarpos;
			compute_scrollbarpixelpos();
		}
	}
	if (wd == 2) {
		if (scrollbarpos+1 < scrollbarmaxpos) {
			++scrollbarpos;
			compute_scrollbarpixelpos();
		}
	}
	if (oldpos != scrollbarpos)
		on_scroll();
}

widget_list::widget_list(int x, int y, int w, int h, widget* parent_)
	: widget(x, y, w, h, "", parent_), listpos(0), selected(-1), columnwidth(-1)
{
	struct wls : public widget_scrollbar
	{
		unsigned& p;
		void on_scroll() { p = get_current_position(); }
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

void widget_list::set_entry(unsigned n, const string& s)
{
	for (list<string>::iterator i = entries.begin(); i != entries.end(); ++i, --n)
		if (n == 0) *i = s;
}

void widget_list::sort_entries()
{
	entries.sort();
	on_sel_change();
}

void widget_list::make_entries_unique()
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

unsigned widget_list::get_listsize() const
{
	return entries.size();
}

int widget_list::get_selected() const
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

string widget_list::get_selected_entry() const
{
	if (selected >= 0)
		return get_entry(selected);
	return "";
}

unsigned widget_list::get_nr_of_visible_entries() const
{
	return (size.y - 2*globaltheme->frame_size()) / globaltheme->myfont->get_height();
}

void widget_list::clear()
{
	listpos = 0;
	selected = -1;
	entries.clear();
	on_sel_change();
}

void widget_list::draw() const
{
	vector2i p = get_pos();
	draw_area(p.x, p.y, size.x, size.y, false);
	list<string>::const_iterator it = ith(listpos);
	int fw = globaltheme->frame_size();
	unsigned maxp = get_nr_of_visible_entries();
	bool scrollbarvisible = (entries.size() > maxp);
	for (unsigned lp = 0; it != entries.end() && lp < maxp; ++it, ++lp) {
        	color tcol = !is_enabled() ? globaltheme->textdisabledcol : (selected==lp)? globaltheme->textselectcol : globaltheme->textcol;
		if (selected == int(lp + listpos)) {
			int width = size.x-2*fw;
			if (scrollbarvisible)
				width -= 3*fw+globaltheme->icons[0]->get_width();
			color::white().set_gl_color();
			globaltheme->backg->draw(p.x+fw, p.y + fw + lp*globaltheme->myfont->get_height(), width, globaltheme->myfont->get_height());
		}
		// optionally split string into columns
		if (columnwidth < 0) {
			globaltheme->myfont->print(p.x+fw, p.y+fw + lp*globaltheme->myfont->get_height(), *it, tcol, true);
		} else {
			string tmp = *it;
			unsigned col = 0;
			while (true) {
				string::size_type tp = tmp.find("\t");
				string ct = tmp.substr(0, tp);
				globaltheme->myfont->print(p.x+fw+col*unsigned(columnwidth),
							   p.y+fw + lp*globaltheme->myfont->get_height(),
							   ct, tcol, true);
				if (tp == string::npos)
					break;
				tmp = tmp.substr(tp+1);
				++col;
			}
		}
	}
	if (entries.size() > maxp)
		myscrollbar->draw();
}

void widget_list::on_click(int mx, int my, int mb)
{
	vector2i p = get_pos();
	int oldselected = selected;
	if (mb & 1) {
		int fw = globaltheme->frame_size();
		int sp = (my - p.y - fw)/int(globaltheme->myfont->get_height());
		if (sp < 0) sp = 0;
		selected = int(listpos) + sp;
		if (unsigned(selected) >= entries.size()) selected = int(entries.size())-1;
	}
	if (oldselected != selected)
		on_sel_change();
}

void widget_list::on_drag(int mx, int my, int rx, int ry, int mb)
{
	// fixme: this is not correct, translate mb here!
	on_click(mx, my, mb);
}

void widget_list::on_wheel(int wd)
{
	myscrollbar->on_wheel(wd);
}

void widget_list::set_column_width(int cw)
{
	columnwidth = cw;
}

void widget_edit::draw() const
{
	vector2i p = get_pos();
	draw_area(p.x, p.y, size.x, size.y, false);
	int fw = globaltheme->frame_size();
	globaltheme->myfont->print_vc(p.x+fw, p.y+size.y/2, text, is_enabled() ? globaltheme->textcol : globaltheme->textdisabledcol, true);
	vector2i sz = globaltheme->myfont->get_size(text.substr(0, cursorpos));
	glBindTexture(GL_TEXTURE_2D, 0);
	globaltheme->textcol.set_gl_color();
	if (this == (const widget*)focussed)
		sys().draw_rectangle(p.x+fw+sz.x, p.y+size.y/4, fw/2, size.y/2);
}

void widget_edit::on_char(const SDL_keysym& ks)
{
	int c = ks.sym;
	unsigned l = text.length();
	unsigned textw = globaltheme->myfont->get_size(text).x;
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
		unsigned stxw = globaltheme->myfont->get_size(stx).x;
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

void widget_fileselector::read_current_dir()
{
	current_dir->clear();
	directory dir = open_dir(current_path->get_text());
	sys().myassert(dir.is_valid(), "[widget_fileselector::read_current_dir] could not open directory");
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

void widget_fileselector::listclick()
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



widget_3dview::widget_3dview(int x, int y, int w, int h, auto_ptr<model> mdl_, color bgcol, widget* parent_)
	: widget(x, y, w, h, "", parent_), mdl(mdl_), backgrcol(bgcol),
	  z_angle(90), x_angle(0), lightdir(0, 0, 1, 0), lightcol(color::white())
{
	translation.z = 100;
	if (mdl.get()) {
		translation.z = mdl->get_boundbox_size().length() / 1.2;
	}
}



void widget_3dview::set_model(std::auto_ptr<model> mdl_)
{
	mdl = mdl_;
	if (mdl.get()) {
		translation.z = mdl->get_boundbox_size().length() / 1.2;
	} else {
		translation.z = 100;
	}
}



void widget_3dview::on_wheel(int wd)
{
	if (wd == 1) {
		translation.z += 2;
	} else if (wd == 2) {
		translation.z -= 2;
	}
}



void widget_3dview::on_drag(int mx, int my, int rx, int ry, int mb)
{
	if (mb & SDL_BUTTON_LMASK) {
		z_angle += rx * 0.5;
		x_angle += ry * 0.5;
	}
	if (mb & SDL_BUTTON_RMASK) {
		translation.x += rx * 0.1;
		translation.y += ry * 0.1;
	}
}



void widget_3dview::draw() const
{
	if (!mdl.get()) return;

	vector3f bb = mdl->get_boundbox_size();
	float bbl = bb.length();
	float zfar = translation.z + bbl * 0.5;

	sys().unprepare_2d_drawing();
	glFlush();
	unsigned rx = sys().get_res_x();
	unsigned ry = sys().get_res_y();
	unsigned rx2 = sys().get_res_x_2d();
	unsigned ry2 = sys().get_res_y_2d();
	unsigned vpx = pos.x*rx/rx2;
	unsigned vpy = (ry2 - pos.y - size.y)*ry/ry2;
	unsigned vpw = size.x*rx/rx2;
	unsigned vph = size.y*ry/ry2;
	glViewport(vpx, vpy, vpw, vph);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	sys().gl_perspective_fovx(70.0/*fovx*/, float(size.x)/size.y, 1.0f, zfar);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	float clr[4];
	backgrcol.store_rgba(clr);
	glClearColor(clr[0], clr[1], clr[2], clr[3]);
	glClear(GL_DEPTH_BUFFER_BIT /* | GL_COLOR_BUFFER_BIT*/);
	glEnable(GL_LIGHTING);
	glLightfv(GL_LIGHT0, GL_POSITION, &lightdir.x);
	GLfloat diffcolor[4];
	lightcol.store_rgba(diffcolor);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffcolor);
	glLightfv(GL_LIGHT0, GL_SPECULAR, diffcolor);
	GLfloat ambcolor[4] = { 0.1f, 0.1f, 0.1f, 1 };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambcolor);

	glTranslatef(-translation.x, -translation.y, -translation.z);
	glRotatef(-80, 1, 0, 0);
	glRotatef(z_angle, 0, 0, 1);
	glRotatef(x_angle, 1, 0, 0);
	glColor4f(0, 0, 0, 1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBegin(GL_LINES);
	glVertex3f(-bb.x*0.5, 0.0, -bb.z*0.5);
	glVertex3f( bb.x*0.5, 0.0, -bb.z*0.5);
	glVertex3f( 0.0,-bb.y*0.5, -bb.z*0.5);
	glVertex3f( 0.0, bb.y*0.5, -bb.z*0.5);
	glEnd();
	glColor3f(1, 1, 1);
	mdl->display();
	glEnable(GL_LIGHTING);

	sys().prepare_2d_drawing();
}



widget_slider::widget_slider(int x, int y, int w, int h, const string& text_,
			     int minv, int maxv, int currv, int descrstep_,
			     widget* parent_)
	: widget(x, y, w, h, text_, parent_),
	  minvalue(minv), maxvalue(maxv), currvalue(currv), descrstep(descrstep_)
{
	size.x = std::max(size.x, int(4));
	size.y = std::max(size.y, int(4));
	maxvalue = std::max(minvalue + 1, maxvalue);
	currvalue = std::max(currvalue, minvalue);
	currvalue = std::min(currvalue, maxvalue);
	descrstep = std::max(int(1), descrstep);
}



void widget_slider::draw() const
{
	// draw sunken area that just has the sunken border height*2, so you see only borders.
	// draw the slider as raised area with height of 8*border or so.
	// draw vertical lines below the slider each n values, so that we have at least 32pix
	// between each value or 16 or so.
	// draw descriptions (min...maxval) every n pixels/positions, so that there is at
	// least n pixel space between each description
	
	// draw description if there is one
	color tcol = is_enabled() ? globaltheme->textcol : globaltheme->textdisabledcol;
	unsigned h2 = globaltheme->myfont->get_height();
	unsigned h0 = 0;
	if (text.length() > 0) {
		globaltheme->myfont->print(pos.x, pos.y, text, tcol, true);
		h0 = globaltheme->myfont->get_size(text).y;
	}

	// draw slider bar
	unsigned h1 = size.y - h0 - h2;
	unsigned barh = globaltheme->frame[0]->get_height() * 2;
	unsigned sliderw = h2;
	unsigned baroff = h1/2 - barh;
	draw_area(pos.x, pos.y + h0 + baroff, size.x, barh, false);

	// draw marker texts and lines.
	for (int i = minvalue; i <= maxvalue; i += descrstep) {
		ostringstream oss;
		oss << i;
		string vals = oss.str();
		unsigned offset = (size.x - sliderw) * (i - minvalue)/(maxvalue - minvalue);
		int valw = globaltheme->myfont->get_size(vals).x;
		globaltheme->myfont->print(pos.x + sliderw/2 + offset - valw/2, pos.y + h0 + h1, vals, tcol, true);
		draw_line(pos.x + sliderw/2 + offset, pos.y + h0 + baroff + barh, pos.x + sliderw/2 + offset, pos.y + h0 + h1);
		// last descriptions should be aligned right and printed, so maybe skip second last one.
		if (i < maxvalue && i + descrstep > maxvalue) {
			i = maxvalue - descrstep;
		}
	}

	// draw slider knob
	unsigned offset = (size.x-sliderw)*(currvalue - minvalue)/(maxvalue - minvalue);
	draw_area_col(pos.x + offset, pos.y + h0, sliderw, h1 - barh, true, globaltheme->textdisabledcol);
	draw_line(pos.x + sliderw/2 + offset, pos.y + h0 + barh/2, pos.x + sliderw/2 + offset, pos.y + h0 + h1 - barh*3/2);
}



void widget_slider::on_char(const SDL_keysym& ks)
{
	// move with cursor
	int c = ks.sym;
	if (c == SDLK_LEFT && currvalue > minvalue) {
		--currvalue;
		on_change();
	} else if (c == SDLK_RIGHT && currvalue < maxvalue) {
		++currvalue;
		on_change();
	}
}



void widget_slider::on_click(int mx, int my, int mb)
{
	// set slider...
	if (mb & SDL_BUTTON_LMASK) {
		int sliderpos = std::min(std::max(pos.x, mx), pos.x + size.x) - pos.x;
		currvalue = sliderpos * (maxvalue - minvalue) / size.x + minvalue;
		on_change();
	}
}



void widget_slider::on_drag(int mx, int my, int rx, int ry, int mb)
{
	// move slider...
	if (mb & SDL_BUTTON_LMASK) {
		int sliderpos = std::min(std::max(pos.x, mx), pos.x + size.x) - pos.x;
		currvalue = sliderpos * (maxvalue - minvalue) / size.x + minvalue;
		on_change();
	}
}
