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

#include "music.h"
#include "audio_backend.h"
#include "datadirs.h"
#include "error.h"
#include "global_data.h"
#include "log.h"
#include "system.h"
#include "xml.h"
#include "display_backend.h"
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;

#define SOUND_SPEC_FILENAME "sound-categories.data"
#define SFX_CHANNELS_TOTAL 8
#define SFX_CHANNEL_MACHINE 0

bool music::use_music = true;

music::music(bool useit, unsigned sample_rate_)
    : thread("music___"),
      nr_reserved_channels(1),
      sample_rate(sample_rate_),
      current_track(0),
      usersel_next_track(-1),
      usersel_fadein(0),
      pbm(PBM_LOOP_LIST),
      stopped(true),
      current_machine_sfx(0) {
    use_music = useit;
}

music::~music() {
}

void music::destructor() {
    audio_backend* be = get_audio_backend();
    for (auto& kv : sfx_events) {
        for (auto* c : kv.second)
            be->free_chunk(c);
    }
    sfx_events.clear();
    for (auto& kv : sfx_machines) {
        for (auto* c : kv.second)
            be->free_chunk(c);
    }
    sfx_machines.clear();
}

void music::start_play_track(unsigned nr, unsigned fadeintime) {
    if (nr < playlist.size()) {
        current_track = nr;
        audio_backend* be = get_audio_backend();
        bool ok = be->play_music(musiclist[current_track], 1, static_cast<int>(fadeintime));
        if (!ok)
            throw sdl_error(std::string("music playing failed: ") + be->get_error());
        stopped = false;
    }
}

void music::callback_track_finished() {
    // note! we CAN NOT protect the instance variable with a mutex here, because
    // we can't make a static variable for this, because we need to initialize SDL
    // before using SDL_mutexes...
    instance().track_finished();
}

void music::init() {
    if (!use_music)
        return;

    audio_backend* be = get_audio_backend();
    if (!be->init_audio_subsystem()) {
        log_warning("Unable to initialize SDL Audio: " << display_get_error());
        use_music = false;
        return;
    }

    const int audio_channels = 2;
    const int audio_buffers = 4096;
    log_info("Audio initialize " << audio_channels << " chnls " << sample_rate << "hz");

    if (!be->open_audio(sample_rate, 0, audio_channels, audio_buffers)) {
        log_warning("Unable to initialize audio: " << be->get_error());
        use_music = false;
        return;
    }

    be->set_music_finished_callback(callback_track_finished);

    if (!be->allocate_channels(SFX_CHANNELS_TOTAL))
        throw error("could not allocate enough channels");

    if (!be->reserve_channels(nr_reserved_channels))
        throw error("could not reserve enough channels");

#if 0
	// load sfx files
	// fixme: later implement a cache!
	try {
		xml_doc spec(get_sound_dir() + SOUND_SPEC_FILENAME);
		spec.load();
		xml_elem sf = spec.first_child();
		xml_elem mc = sf.child("machines");
		for (xml_elem::iterator it = mc.iterate(); !it.end(); it.next()) {
			xml_elem m = it.elem();
			string mn = m.get_name();
			vector<Mix_Chunk*>& v = sfx_machines[mn];
			// the levels/level attributes are ignored yet...fixme
			for (xml_elem::iterator ik = m.iterate(); !ik.end(); ik.next()) {
				xml_elem e = ik.elem();
				if (e.get_name() != "throttle")
					throw xml_error("illegal child of \"machine\"", e.doc_name());
				string fn = e.attr("file");
				v.push_back(0);
				Mix_Chunk* chk = Mix_LoadWAV((get_sound_dir() + fn).c_str());
				if (!chk) {
					v.pop_back();
					throw file_read_error(get_sound_dir() + fn);
				}
				v.back() = chk;
			}
		}
		xml_elem ef = sf.child("effects");
		for (xml_elem::iterator it = ef.iterate(); !it.end(); it.next()) {
			xml_elem e = it.elem();
			string en = e.get_name();
			string fn = e.attr("file");
			vector<Mix_Chunk*>& v = sfx_events[en];
			v.push_back(0);
			Mix_Chunk* chk = Mix_LoadWAV((get_sound_dir() + fn).c_str());
			if (!chk) {
				v.pop_back();
				throw file_read_error(get_sound_dir() + fn);
			}
			v.back() = chk;
		}
	}
	catch (...) {
		destructor();
		throw;
	}
#endif
}

void music::loop() {
    command_queue.process_messages();
}

void music::deinit() {
    if (!use_music)
        return;

    audio_backend* be = get_audio_backend();
    if (be->is_playing_music()) {
        be->halt_music();
        stopped = true;
    }

    for (auto* m : musiclist)
        be->free_music(m);
    musiclist.clear();

    destructor();

    be->close_audio();
}

void music::request_abort() {
    thread::request_abort();
    command_queue.wakeup_receiver();
}

// -------------------- commands --------------------

bool music::append_track(const std::string &filename) {
    return command_queue.send(std::make_unique<command_append_track>(*this, filename));
}

bool music::set_playback_mode(playback_mode pbm) {
    return command_queue.send(std::make_unique<command_set_playback_mode>(*this, pbm));
}

bool music::play(unsigned fadein) {
    return command_queue.send(std::make_unique<command_play>(*this, fadein));
}

bool music::stop(unsigned fadeout) {
    return command_queue.send(std::make_unique<command_stop>(*this, fadeout));
}

bool music::pause() {
    return command_queue.send(std::make_unique<command_pause>(*this));
}

bool music::resume() {
    return command_queue.send(std::make_unique<command_resume>(*this));
}

bool music::set_music_position(float pos) {
    return command_queue.send(std::make_unique<command_set_music_position>(*this, pos));
}

bool music::play_track(unsigned nr, unsigned fadeouttime, unsigned fadeintime) {
    return command_queue.send(std::make_unique<command_play_track>(*this, nr, fadeouttime, fadeintime));
}

bool music::track_finished() {
    return command_queue.send(std::make_unique<command_track_finished>(*this), false);
}

std::vector<std::string> music::get_playlist() {
    std::vector<std::string> myplaylist;
    command_queue.send(std::make_unique<command_get_playlist>(*this, myplaylist));
    return myplaylist;
}

unsigned music::get_current_track() {
    unsigned track = 0;
    command_queue.send(std::make_unique<command_get_current_track>(*this, track));
    return track;
}

bool music::is_playing() {
    bool isply = false;
    command_queue.send(std::make_unique<command_is_playing>(*this, isply));
    return isply;
}

bool music::play_sfx(const std::string &category, const vector3 &listener, angle listener_dir, const vector3 &noise_pos) {
    return command_queue.send(std::make_unique<command_play_sfx>(*this, category, listener, listener_dir, noise_pos));
}

bool music::play_sfx_machine(const std::string &name, unsigned throttle) {
    return command_queue.send(std::make_unique<command_play_sfx_machine>(*this, name, throttle));
}

bool music::pause_sfx(bool on) {
    return command_queue.send(std::make_unique<command_pause_sfx>(*this, on));
}

// -------------------- command exec --------------------

void music::exec_append_track(const std::string &filename) {
    if (!use_music)
        throw std::invalid_argument("no music support");

    audio_backend* be = get_audio_backend();
    audio_music_handle* tmp = be->load_music(get_sound_dir() + filename);

    if (!tmp) {
        log_warning("Failed to load track: " << filename << ", " << be->get_error());
        throw file_read_error(filename);
    }
    playlist.push_back(filename);
    musiclist.push_back(tmp);
}

void music::exec_set_playback_mode(playback_mode pbm_) {
    if (!use_music)
        throw std::invalid_argument("no music support");
    pbm = pbm_;
}

void music::exec_play(unsigned fadein) {
    if (!use_music)
        throw std::invalid_argument("no music support");
    if (!get_audio_backend()->is_playing_music()) {
        start_play_track(current_track, fadein);
    } else {
        throw std::runtime_error("music still playing, can't execute play()");
    }
}

void music::exec_stop(unsigned fadeout) {
    if (!use_music)
        throw std::invalid_argument("no music support");
    audio_backend* be = get_audio_backend();
    if (be->is_paused_music())
        be->resume_music();
    if (be->is_playing_music()) {
        stopped = true;
        if (fadeout > 0)
            be->fade_out_music(static_cast<int>(fadeout));
        else
            be->halt_music();
    } else {
        throw std::runtime_error("music not playing, can't execute stop()");
    }
}

void music::exec_pause() {
    if (!use_music)
        throw std::invalid_argument("no music support");
    audio_backend* be = get_audio_backend();
    if (be->is_playing_music() && !be->is_paused_music())
        be->pause_music();
}

void music::exec_resume() {
    if (!use_music)
        throw std::invalid_argument("no music support");
    get_audio_backend()->resume_music();
}

void music::exec_set_music_position(float pos) {
    audio_backend* be = get_audio_backend();
    be->rewind_music();
    if (!be->set_music_position(pos)) {
        throw std::runtime_error("music set position failed");
    }
}

void music::exec_play_track(unsigned nr, unsigned fadeouttime, unsigned fadeintime) {
    if (!use_music)
        throw std::invalid_argument("no music support");
    try {
        exec_stop(fadeouttime);
    } catch (exception &e) {
        // couldn't stop, so music was already stopped?
        current_track = nr;
        exec_play(fadeintime);
        return;
    }
    usersel_next_track = int(nr);
    usersel_fadein = fadeintime;
}

void music::exec_track_finished() {
    if (usersel_next_track >= 0) {
        current_track = unsigned(usersel_next_track);
        exec_play(usersel_fadein);
        usersel_next_track = -1;
        usersel_fadein = 0;
        return;
    }
    if (!stopped) {
        switch (pbm) {
        case PBM_LOOP_LIST:
            ++current_track;
            if (current_track >= playlist.size())
                current_track = 0;
            break;
        case PBM_LOOP_TRACK:
            break;
        case PBM_SHUFFLE_TRACK:
            current_track = rnd(playlist.size());
            break;
        }
        start_play_track(current_track, 0);
    }
}

void music::exec_get_playlist(std::vector<std::string> &playlist_) {
    playlist_ = playlist;
}

void music::exec_get_current_track(unsigned &track) {
    track = current_track;
}

void music::exec_is_playing(bool &isply) {
    audio_backend* be = get_audio_backend();
    if (!be->is_playing_music() || be->is_paused_music())
        throw std::runtime_error("music not playing");
    isply = true;
}

void music::exec_play_sfx(const std::string &category, const vector3 &listener, angle listener_dir, const vector3 &noise_pos) {
    if (!use_music)
        throw std::invalid_argument("no music support");
    auto it = sfx_events.find(category);
    if (it == sfx_events.end())
        throw invalid_argument(string("unknown category for sfx: ") + category);
    unsigned snr = unsigned(it->second.size() * rnd());
    audio_chunk_handle* chk = it->second[snr];
    audio_backend* be = get_audio_backend();

    double distanceFromPlayer = listener.distance(noise_pos);
    // calculate max hearing distance (meters)
    double hearing_range = 0.0;
    double hearing_increment = 0.0;

    // fixme: this depends on wether the noise is inside the sub or not!

    // audible range varies depending on whether player/noise source is submerged or not
    //
    // 255 == max dist value for Mix_SetPosition()
    if (listener.z < 0) {
        // player submerged
        if (noise_pos.z < 0) {
            hearing_range = 20000.0;
            hearing_increment = 78.43; // hearing_range / 255;
        } else {
            hearing_range = 30000.0;
            hearing_increment = 117.64; // hearing_range / 255;
        }
    } else {
        // player on the surface
        if (noise_pos.z < 0) {
            hearing_range = 5000.0;
            hearing_increment = 19.60; // hearing_range / 255;
        } else {
            hearing_range = 10000.0;
            hearing_increment = 39.21; // hearing_range / 255;
        }
    }

    // is sound within audible range
    if (distanceFromPlayer <= hearing_range) {
        angle bearingFromPlayer(noise_pos.xy() - listener.xy());
        bearingFromPlayer = bearingFromPlayer + listener_dir;

        // dist for set_channel_position is 0-255, 0 = loudest
        int dist = static_cast<int>(distanceFromPlayer / hearing_increment);
        short angle_val = static_cast<short>(bearingFromPlayer.value());

        int channel_num = be->play_channel(-1, chk, 0);
        if (channel_num < 0)
            throw sdl_error(std::string("unable to play sfx: ") + be->get_error());
        if (!be->set_channel_position(channel_num, angle_val, static_cast<unsigned char>(dist)))
            throw sdl_error("set_channel_position() failed");
    }
}

void music::exec_play_sfx_machine(const std::string &name, unsigned throttle) {
    if (!use_music)
        throw std::invalid_argument("no music support");
    auto it = sfx_machines.find(name);
    if (it == sfx_machines.end())
        throw invalid_argument(string("unknown machine name: ") + name);
    audio_backend* be = get_audio_backend();
    unsigned nrthr = it->second.size();
    unsigned thr = throttle * (nrthr + 1) / 100;
    if (thr == 0) {
        if (be->is_channel_playing(SFX_CHANNEL_MACHINE))
            be->halt_channel(SFX_CHANNEL_MACHINE);
        current_machine_sfx = nullptr;
        return;
    }
    thr = std::min(thr - 1, nrthr - 1);

    audio_chunk_handle* chk = it->second[thr];
    if (chk == current_machine_sfx)
        return;

    if (be->is_channel_playing(SFX_CHANNEL_MACHINE))
        be->halt_channel(SFX_CHANNEL_MACHINE);
    current_machine_sfx = nullptr;
    if (be->play_channel(SFX_CHANNEL_MACHINE, chk, -1) < 0)
        throw sdl_error(std::string("can't play channel: ") + be->get_error());
    current_machine_sfx = chk;
}

void music::exec_pause_sfx(bool on) {
    if (on)
        get_audio_backend()->pause_channel(-1);
    else
        get_audio_backend()->resume_channel(-1);
}
