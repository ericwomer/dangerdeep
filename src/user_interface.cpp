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

// user interface common code
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "oglext/OglExt.h"
#include <SDL.h>
#include <glu.h>

#include "airplane.h"
#include "depth_charge.h"
#include "game.h"
#include "gun_shell.h"
#include "ui_messages.h"
#include "model.h"
#include "submarine.h" // needed for underwater sound reduction
#include "submarine_interface.h"
#include "system.h"
#include "texts.h"
#include "user_interface.h"
#include "vector3.h"
#include "weather_renderer.h"
#include "terrain_manager.h"
#include "widget.h"
#include <iomanip>
#include <iostream>
#include <sstream>
// #include "ship_interface.h"
// #include "airplane_interface.h"
#include "cfg.h"
#include "global_data.h"
#include "keys.h"
#include "log.h"
#include "matrix4.h"
#include "music.h"
#include "particle.h"
#include "sky.h"
#include "water.h"
using namespace std;

#undef RAIN
#undef SNOW

#define MAX_PANEL_SIZE 256

/*
        a note on our coordinate system (11/10/2003):
        We simulate earth by projecting objects according to curvature from earth
        space to Euclidian space. This projection is yet a identity projection, that means
        we ignore curvature yet.
        The map forms a cylinder around the earth, that means x,y position on the map translates
        to longitude,latitude values. Hence valid coordinates go from -20000km...20000km in x
        direction and -10000km to 10000km in y direction. (we could use exact values, around
        20015km). The wrap around is a problem, but that's somewhere in the Pacific ocean, so
        we just ignore it. This mapping leads to some distorsion and wrong distance values
        when coming to far north or south on the globe. We just ignore this for simplicity's
        sake. The effect shouldn't be noticeable.
*/

user_interface::user_interface(game &gm) : mygame(&gm),
                                           config(cfg::instance()),
                                           audio(music::instance()),
                                           pause(false),
                                           time_scale(1),
                                           panel_visible(true),
                                           screen_selector_visible(false),
                                           playlist_visible(false),
                                           main_menu_visible(false),
                                           mymessages(std::make_unique<ui_message_queue>()),
                                           bearing(0),
                                           elevation(90),
                                           bearing_is_relative(true),
                                           current_display(0),
                                           current_popup(0),
                                           mycoastmap(get_map_dir() + "default.xml"),
                                           daymode(gm.is_day_mode()),
                                           myweather(std::make_unique<weather_renderer>()) {
    add_loading_screen("coast map initialized");
    mysky = std::make_unique<sky>();
    panel = std::make_unique<widget>(0, 768 - 32, 1024, 32, "", nullptr);
    panel->set_background(0);
    // ca. 1024-2*8 for 6 texts => 168 pix. for each text
    int paneltextnrs[6] = {1, 4, 5, 2, 98, 61};
    const char *paneltexts[6] = {"000", "000", "000", "000", "000", "00:00:00"};
    for (int i = 0; i < 6; ++i) {
        int off = 8 + i * (1024 - 2 * 8) / 6;
        string tx = texts::get(paneltextnrs[i]);
        vector2i sz = widget::get_theme()->myfont->get_size(tx);
        panel->add_child(new widget_text(off, 4, 0, 0, tx));
        panel_valuetexts[i] = new widget_text(off + 8 + sz.x, 4, 0, 0, paneltexts[i]);
        panel->add_child(panel_valuetexts[i]);
    }

    // create screen selector widget
    screen_selector = std::make_unique<widget>(0, 0, 256, 32, "", nullptr);
    screen_selector->set_background(0);

    // create playlist widget
    music_playlist = std::make_unique<widget>(0, 0, 384, 512, texts::get(262), nullptr);
    music_playlist->set_background(0);
    struct musiclist : public widget_list {
        music& music_ref;
        bool active;
        void on_sel_change() {
            if (!active)
                return;
            int s = get_selected();
            if (s >= 0)
                music_ref.play_track(unsigned(s), 500);
        }
        musiclist(music& m, int x, int y, int w, int h) : widget_list(x, y, w, h), music_ref(m), active(false) {}
    };
    musiclist *playlist = new musiclist(audio, 0, 0, 384, 512);
    music_playlist->add_child_near_last_child(playlist);
    music &m = audio;
    vector<string> mpl = m.get_playlist();
    for (vector<string>::const_iterator it = mpl.begin();
         it != mpl.end(); ++it) {
        playlist->append_entry(*it);
    }
    typedef widget_caller_checkbox<user_interface, void (user_interface::*)()> wccui;
    // fixme: use checkbox here...
    playlist_repeat_checkbox = new wccui(this, &user_interface::playlist_mode_changed, 0, 0, 192, 32, false, texts::get(263));
    music_playlist->add_child_near_last_child(playlist_repeat_checkbox);
    playlist_shuffle_checkbox = new wccui(this, &user_interface::playlist_mode_changed, 0, 0, 192, 32, false, texts::get(264));
    music_playlist->add_child_near_last_child(playlist_shuffle_checkbox, 0, 1);
    playlist_mute_checkbox = new wccui(this, &user_interface::playlist_mute, 0, 0, 384, 32, false, texts::get(265));
    music_playlist->add_child_near_last_child(playlist_mute_checkbox, 0);
    playlist_mute_checkbox->move_pos(vector2i(-192, 0));
    music_playlist->add_child_near_last_child(new widget_set_button<bool>(playlist_visible, false, 0, 0, 384, 32, texts::get(260)), 0);
    music_playlist->clip_to_children_area();
    music_playlist->set_pos(vector2i(0, 0));
    // enable music switching finally, to avoid on_sel_change changing the music track,
    // because on_sel_change is called above, when adding entries.
    playlist->active = true;

    // create main menu widget
    main_menu = std::make_unique<widget>(0, 0, 256, 128, texts::get(104), nullptr);
    main_menu->set_background(0);
    typedef widget_caller_button<user_interface, void (user_interface::*)()> wcbui;
    main_menu->add_child_near_last_child(new wcbui(this, &user_interface::show_screen_selector, 0, 0, 256, 32, texts::get(266)));
    main_menu->add_child_near_last_child(new wcbui(this, &user_interface::toggle_popup, 0, 0, 256, 32, texts::get(267)), 0);
    main_menu->add_child_near_last_child(new wcbui(this, &user_interface::show_playlist, 0, 0, 256, 32, texts::get(261)), 0);
    main_menu->add_child_near_last_child(new wcbui(this, &user_interface::toggle_pause, 0, 0, 256, 32, texts::get(268)), 0);
    main_menu->add_child_near_last_child(new widget_caller_arg_button<user_interface, void (user_interface::*)(bool), bool>(this, &user_interface::request_abort, true, 0, 0, 256, 32, texts::get(177)), 0);
    main_menu->add_child_near_last_child(new widget_set_button<bool>(main_menu_visible, false, 0, 0, 256, 32, texts::get(260)), 0);
    main_menu->clip_to_children_area();
    vector2i mmp = sys().get_res_2d() - main_menu->get_size();
    main_menu->set_pos(vector2i(mmp.x / 2, mmp.y / 2));

    particle::init();

    // level size is N * sample_spacing * 2^j, here we give n = log2(N)
    // where j is level number from 0 on.
    // so we compute number of levels:
    // 2^n * sample_spacing * 2^j_max <= z_far
    // j_max <= log2(z_far / (2^n * sample_spacing))
    // and #levels = j_max+1
    // so #levels = floor(log2(z_far / (2^n * sample_spacing))) + 1
    // const double z_far = 20000.0;

    add_loading_screen("user interface initialized");

    myterrain = std::make_unique<terrain_manager>(TERRAIN_NR_LEVELS, TERRAIN_RESOLUTION_N, mygame->get_height_gen());
    myterrain->set_viewer_position(gm.get_player()->get_pos());

    add_loading_screen("terrain loaded");
}

void user_interface::finish_construction() {
    mycoastmap.finish_construction();
}

user_interface *user_interface::create(game &gm) {
    sea_object *p = gm.get_player();
    user_interface *ui = 0;
    // check for interfaces
    if (dynamic_cast<submarine *>(p))
        ui = new submarine_interface(gm);
#if 0
	else if (dynamic_cast<ship*>(p)) ui = new ship_interface(gm);
	else if (dynamic_cast<airplane*>(p)) ui = new airplane_interface(gm);
#endif
    if (ui)
        ui->finish_construction();
    return ui;
}

user_interface::~user_interface() {
    particle::deinit();
}

const water &user_interface::get_water() const {
    return mygame->get_water();
}

angle user_interface::get_relative_bearing() const {
    if (bearing_is_relative)
        return bearing;
    return bearing - mygame->get_player()->get_heading();
}

angle user_interface::get_absolute_bearing() const {
    if (bearing_is_relative)
        return mygame->get_player()->get_heading() + bearing;
    return bearing;
}

angle user_interface::get_elevation() const {
    return elevation;
}

void user_interface::add_bearing(angle a) {
    bearing += a;
}

void user_interface::add_elevation(angle a) {
    elevation += a;
}

void user_interface::display() const {
    // fixme: brightness needs sun_pos, so compute_sun_pos() is called multiple times per frame
    // but is very costly. we could cache it.
    mygame->get_water().set_refraction_color(mygame->compute_light_color(mygame->get_player()->get_pos()));
    displays[current_display]->display(*mygame);

    // popups
    if (current_popup > 0)
        popups[current_popup - 1]->display(*mygame);

    // draw screen selector if visible
    if (screen_selector_visible) {
        sys().prepare_2d_drawing();
        screen_selector->draw();
        sys().unprepare_2d_drawing();
    }

    // draw music playlist if visible
    if (playlist_visible) {
        sys().prepare_2d_drawing();
        music_playlist->draw();
        sys().unprepare_2d_drawing();
    }

    // draw main_menu if visible
    if (main_menu_visible) {
        sys().prepare_2d_drawing();
        main_menu->draw();
        sys().unprepare_2d_drawing();
    }
}

void user_interface::set_time(double tm) {
    // if we switched from day to night mode or vice versa, reload current screen.
    if (mygame) {
        bool newdaymode = mygame->is_day_mode();
        if (newdaymode != daymode) {
            mygame->freeze_time();
            displays[current_display]->leave();
            displays[current_display]->enter(newdaymode);
            mygame->unfreeze_time();
        }
        daymode = newdaymode;
    }

    mysky->set_time(tm);
    mycaustics.set_time(tm);
    mygame->get_water().set_time(tm);
}

void user_interface::process_input(const SDL_Event &event) {
    if (panel_visible) {
        if (panel->check_for_mouse_event(event))
            return;
    }

    if (main_menu_visible) {
        if (main_menu->check_for_mouse_event(event)) {
            return;
        }
    }

    if (screen_selector_visible) {
        if (screen_selector->check_for_mouse_event(event)) {
            // drag for the menu
            // fixme: drag&drop support should be in widget class...
            if (event.type == SDL_MOUSEMOTION) {
                vector2i p = screen_selector->get_pos();
                vector2i s = screen_selector->get_size();
                // drag menu with left mouse button when on title or right mouse button else
                vector2i position = sys().translate_position(event);
                vector2 motion = sys().translate_motion(event);
                if (event.motion.state & SDL_BUTTON_MMASK || (event.motion.state & SDL_BUTTON_LMASK && position.x >= p.x && position.y >= p.y && position.x < p.x + s.x && position.y < p.y + 32)) {

                    p.x += int(ceil(motion.x));
                    p.y += int(ceil(motion.y));
                    p = p.max(vector2i(0, 0));
                    p = p.min(sys().get_res_2d() - s);
                    screen_selector->set_pos(p);
                }
            }
            return;
        }
    }

    if (playlist_visible) {
        if (music_playlist->check_for_mouse_event(event)) {
            // drag for the menu
            // fixme: drag&drop support should be in widget class...
            if (event.type == SDL_MOUSEMOTION) {
                vector2i p = music_playlist->get_pos();
                vector2i s = music_playlist->get_size();
                vector2i pos = sys().translate_position(event);
                // drag menu with left mouse button when on title or right mouse button else
                if (event.motion.state & SDL_BUTTON_MMASK || (event.motion.state & SDL_BUTTON_LMASK && pos.x >= p.x && pos.y >= p.y && pos.x < p.x + s.x && pos.y < p.y + 32 + 8)) {

                    p.x += int(ceil(sys().translate_motion_x(event)));
                    p.y += int(ceil(sys().translate_motion_y(event)));
                    if (p.x < 0)
                        p.x = 0;
                    if (p.y < 0)
                        p.y = 0;
                    // 2006-11-30 doc1972 negative pos and size of a playlist makes no sence, so we cast
                    if ((unsigned int)(p.x + s.x) > sys().get_res_x_2d())
                        p.x = sys().get_res_x_2d() - s.x;
                    if ((unsigned int)(p.y + s.y) > sys().get_res_y_2d())
                        p.y = sys().get_res_y_2d() - s.y;
                    music_playlist->set_pos(p);
                }
            }
            return;
        }
    }

    if (event.type == SDL_KEYDOWN) {
        if (config.getkey(KEY_TOGGLE_RELATIVE_BEARING).equal(event.key.keysym)) {
            bearing_is_relative = !bearing_is_relative;
            add_message(texts::get(bearing_is_relative ? 220 : 221));
            return;
        } else if (config.getkey(KEY_TOGGLE_POPUP).equal(event.key.keysym)) {
            toggle_popup();
            return;
        }
    }

    displays[current_display]->process_input(*mygame, event);
}

void user_interface::process_input(list<SDL_Event> &events) {
    // if screen selector menu is open and mouse is over that window, handle mouse events there.

    if (current_popup > 0)
        popups[current_popup - 1]->process_input(*mygame, events);

    for (list<SDL_Event>::const_iterator it = events.begin();
         it != events.end(); ++it)
        process_input(*it);
}

void user_interface::show_target(double vx, double vy, double w, double h, const vector3 &viewpos) {
    if (mygame && mygame->get_player()->get_target()) {
        // draw red triangle below target
        // find screen position of target by projecting its position to screen
        // coordinates.
        vector4 tgtscr = (matrix4::get_glf(GL_PROJECTION_MATRIX) * matrix4::get_glf(GL_MODELVIEW_MATRIX)) * (mygame->get_player()->get_target()->get_pos() - viewpos).xyz0();
        if (tgtscr.z > 0) {
            // only when in front.
            // transform to screen coordinates, using the projection coordinates
            double x = (0.5 * tgtscr.x / tgtscr.w + 0.5) * w + vx;
            double y = sys().get_res_y_2d() - ((0.5 * tgtscr.y / tgtscr.w + 0.5) * h + vy);
            sys().prepare_2d_drawing();
            primitives::triangle(vector2f(x - 10, y + 20),
                                 vector2f(x, y + 10),
                                 vector2f(x + 10, y + 20),
                                 colorf(1, 0, 0, 0.5))
                .render();
            sys().unprepare_2d_drawing();
        }
    }
}

void user_interface::draw_terrain(const vector3 &viewpos, angle dir,
                                  double max_view_dist, bool mirrored, int above_water) const {
#if 0
	glPushMatrix();
	glTranslated(0, 0, -viewpos.z);
	// still needed to render the props.
	mycoastmap.render(viewpos.xy(), max_view_dist, mirrored);
	glPopMatrix();
#endif

    // frustum is mirrored inside geoclipmap
    frustum viewfrustum = frustum::from_opengl();
    glPushMatrix();
    if (mirrored)
        glScalef(1.0f, 1.0f, -1.0f);
    viewfrustum.translate(viewpos);
    myterrain->set_viewer_position(viewpos);
    myterrain->render(viewfrustum, -viewpos, mirrored, above_water);
    glPopMatrix();
}

void user_interface::draw_weather_effects() const {
    myweather->draw(mygame->get_time());
}

void user_interface::toggle_pause() {
    pause = !pause;
    if (pause) {
        add_message(texts::get(52));
        pause_all_sound();
    } else {
        add_message(texts::get(53));
        resume_all_sound();
    }
}

bool user_interface::time_scale_up() {
    if (time_scale < 4096) {
        time_scale *= 2;
        return true;
    }
    return false;
}

bool user_interface::time_scale_down() {
    if (time_scale > 1) {
        time_scale /= 2;
        return true;
    }
    return false;
}

void user_interface::draw_infopanel(bool onlytexts) const {
    if (!onlytexts && panel_visible) {
        ostringstream os0;
        os0 << setw(3) << left << mygame->get_player()->get_heading().ui_value();
        panel_valuetexts[0]->set_text(os0.str());
        ostringstream os1;
        os1 << setw(3) << left << unsigned(fabs(round(sea_object::ms2kts(mygame->get_player()->get_speed()))));
        panel_valuetexts[1]->set_text(os1.str());
        ostringstream os2;
        os2 << setw(3) << left << unsigned(round(std::max(0.0, -mygame->get_player()->get_pos().z)));
        panel_valuetexts[2]->set_text(os2.str());
        ostringstream os3;
        os3 << setw(3) << left << get_absolute_bearing().ui_value();
        panel_valuetexts[3]->set_text(os3.str());
        ostringstream os4;
        os4 << setw(3) << left << time_scale;
        panel_valuetexts[4]->set_text(os4.str());
        // compute time string
        panel_valuetexts[5]->set_text(get_time_string(mygame->get_time()));

        panel->draw();
    }

    // draw messages using message queue subsystem
    int y = (onlytexts ? sys().get_res_y_2d() : panel->get_pos().y);
    mymessages->draw(mygame->get_time(), y, font_vtremington12);
}

void user_interface::add_message(const string &s) {
    mymessages->add_message(s, mygame->get_time());
}

void user_interface::switch_geo_wire() {
    myterrain->toggle_wireframe();
}

void user_interface::play_sound_effect(const string &se,
                                       const vector3 &noise_source /*, bool loop*/) const {
    audio.play_sfx(se, mygame->get_player()->get_pos(),
                               mygame->get_player()->get_heading(),
                               noise_source);
}

void user_interface::set_allowed_popup() {
    // 0 is always valid (no popup)
    if (current_popup == 0)
        return;

    unsigned mask = displays[current_display]->get_popup_allow_mask();
    mask >>= (current_popup - 1);
    while (mask != 0) {
        // is popup number valid?
        if (mask & 1)
            return;
        ++current_popup;
        mask >>= 1;
    }
    current_popup = 0;
}

void user_interface::set_current_display(unsigned curdis) {
    if (current_display == curdis) {
        // if we are already on the screen, toggle between popups instead.
        toggle_popup();
        return;
    }
    if (mygame)
        mygame->freeze_time();
    displays[current_display]->leave();
    current_display = curdis;

    // clear both screen buffers
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    sys().swap_buffers(sys().get_sdl_window());
    glClear(GL_COLOR_BUFFER_BIT);
    sys().swap_buffers(sys().get_sdl_window());

    displays[current_display]->enter(daymode);
    if (mygame)
        mygame->unfreeze_time();

    // check if current popup is still allowed. if not, clear popup
    if (current_popup > 0) {
        unsigned mask = displays[current_display]->get_popup_allow_mask();
        mask >>= (current_popup - 1);
        if ((mask & 1) == 0)
            current_popup = 0;
    }
}

void user_interface::playlist_mode_changed() {
    if (playlist_repeat_checkbox->is_checked()) {
        audio.set_playback_mode(music::PBM_LOOP_TRACK);
    } else if (playlist_shuffle_checkbox->is_checked()) {
        audio.set_playback_mode(music::PBM_SHUFFLE_TRACK);
    } else {
        audio.set_playback_mode(music::PBM_LOOP_LIST);
    }
}

void user_interface::playlist_mute() {
    if (playlist_mute_checkbox->is_checked())
        audio.stop();
    else
        audio.play();
}

void user_interface::show_screen_selector() {
    screen_selector_visible = true;
    playlist_visible = false;
    main_menu_visible = false;
}

void user_interface::toggle_popup() {
    // determine which pop is shown and which is allowed and switch to it
    ++current_popup;
    set_allowed_popup();
}

void user_interface::show_playlist() {
    screen_selector_visible = false;
    playlist_visible = true;
    main_menu_visible = false;
}

void user_interface::pause_all_sound() const {
    audio.pause_sfx(true);
}

void user_interface::resume_all_sound() const {
    audio.pause_sfx(false);
}
