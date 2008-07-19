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


#include "system.h"
#include "music.h"
#include "datadirs.h"
#include "global_data.h"
#include "error.h"
#include "xml.h"
#include "log.h"
#include <sstream>
#include <iostream>
#include <fstream>
#include <cmath>
#include <SDL.h>
#include <SDL_mixer.h>

using namespace std;

#define SOUND_SPEC_FILENAME "sound-categories.data"
#define SFX_CHANNELS_TOTAL 8
#define SFX_CHANNEL_MACHINE 0

bool music::use_music = true;



music::music(bool useit)
	: thread("music___"),
	  nr_reserved_channels(1),
	  current_track(0),
	  usersel_next_track(-1),
	  usersel_fadein(0),
	  pbm(PBM_LOOP_LIST),
	  stopped(true),
	  current_machine_sfx(0)
{
	use_music = useit;
}



music::~music()
{
}



void music::destructor()
{
	map<string, vector<Mix_Chunk*> >::iterator it;
	for (it = sfx_events.begin(); it != sfx_events.end(); ++it) {
		for (vector<Mix_Chunk*>::iterator ik = it->second.begin();
		     ik != it->second.end(); ++ik) {
			Mix_FreeChunk(*ik);
		}
	}
	sfx_events.clear();
	for (it = sfx_machines.begin(); it != sfx_machines.end(); ++it) {
		for (vector<Mix_Chunk*>::iterator ik = it->second.begin();
		     ik != it->second.end(); ++ik) {
			Mix_FreeChunk(*ik);
		}
	}
	sfx_machines.clear();
}



void music::start_play_track(unsigned nr, unsigned fadeintime)
{
	if (nr < playlist.size()) {
		current_track = nr;
		int err = -1;
		if (fadeintime > 0) {
			err = Mix_FadeInMusic(musiclist[current_track], 1, fadeintime);
		} else {
			err = Mix_PlayMusic(musiclist[current_track], 1);
		}
		if (err < 0)
			throw sdl_error("music playing failed.");
		stopped = false;
	}
}



void music::callback_track_finished()
{
	// note! we CAN NOT protect the instance variable with a mutex here, because
	// we can't make a static variable for this, because we need to initialize SDL
	// before using SDL_mutexes...
	instance().track_finished();
}



void music::init()
{
	if (!use_music) return;

	if (SDL_Init(SDL_INIT_AUDIO) != 0) {
		log_warning("Unable to to initialize SDL Audio: " << SDL_GetError());
		use_music = false;
		return;
	}

	int audio_rate = 44100; // 22050;
	Uint16 audio_format = AUDIO_S16SYS;
	int audio_channels = 2;
	int audio_buffers = 4096;

	if (Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers) != 0) {
		log_warning("Unable to initialize audio: " << Mix_GetError());
		use_music = false;
		return;
	}

	Mix_HookMusicFinished(callback_track_finished);

	// allocate channels
	if (Mix_AllocateChannels(SFX_CHANNELS_TOTAL) < int(SFX_CHANNELS_TOTAL))
		throw error("could not allocate enough channels");

	// reserve sfx channels for environmental noises (engine, sonar)
	if (Mix_ReserveChannels(nr_reserved_channels) < int(nr_reserved_channels))
		throw error("could not reserve enough channels");

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
}



void music::loop()
{
	command_queue.process_messages();
}



void music::deinit()
{
	if (!use_music) return;

	// halt
	if (Mix_PlayingMusic()) {
		Mix_HaltMusic();
		stopped = true;
	}

	// clear playlist
	for (vector<Mix_Music*>::iterator it = musiclist.begin(); it != musiclist.end(); ++it) {
		Mix_FreeMusic(*it);
	}
	musiclist.clear();

	destructor();

	Mix_CloseAudio();
}



void music::request_abort()
{
	thread::request_abort();
	command_queue.wakeup_receiver();
}



// -------------------- commands --------------------

bool music::append_track(const std::string& filename)
{
	return command_queue.send(std::auto_ptr<message>(new command_append_track(*this, filename)));
}


bool music::set_playback_mode(playback_mode pbm)
{
	return command_queue.send(std::auto_ptr<message>(new command_set_playback_mode(*this, pbm)));
}


bool music::play(unsigned fadein)
{
	return command_queue.send(std::auto_ptr<message>(new command_play(*this, fadein)));
}


bool music::stop(unsigned fadeout)
{
	return command_queue.send(std::auto_ptr<message>(new command_stop(*this, fadeout)));
}


bool music::pause()
{
	return command_queue.send(std::auto_ptr<message>(new command_pause(*this)));
}


bool music::resume()
{
	return command_queue.send(std::auto_ptr<message>(new command_resume(*this)));
}


bool music::play_track(unsigned nr, unsigned fadeouttime, unsigned fadeintime)
{
	return command_queue.send(std::auto_ptr<message>(new command_play_track(*this, nr, fadeouttime, fadeintime)));
}


bool music::track_finished()
{
        return command_queue.send(std::auto_ptr<message>(new command_track_finished(*this)), false);
}


std::vector<std::string> music::get_playlist()
{
	std::vector<std::string> myplaylist;
        command_queue.send(std::auto_ptr<message>(new command_get_playlist(*this, myplaylist)));
	return myplaylist;
}


unsigned music::get_current_track()
{
	unsigned track = 0;
        command_queue.send(std::auto_ptr<message>(new command_get_current_track(*this, track)));
	return track;
}


bool music::is_playing()
{
	bool isply = false;
        command_queue.send(std::auto_ptr<message>(new command_is_playing(*this, isply)));
	return isply;
}


bool music::play_sfx(const std::string& category, const vector3& listener, angle listener_dir, const vector3& noise_pos)
{
        return command_queue.send(std::auto_ptr<message>(new command_play_sfx(*this, category, listener, listener_dir, noise_pos)));
}


bool music::play_sfx_machine(const std::string& name, unsigned throttle)
{
        return command_queue.send(std::auto_ptr<message>(new command_play_sfx_machine(*this, name, throttle)));
}

bool music::pause_sfx(bool on)
{
        return command_queue.send(std::auto_ptr<message>(new command_pause_sfx(*this, on)));
}

// -------------------- command exec --------------------

void music::exec_append_track(const std::string& filename)
{
	if (!use_music) throw std::invalid_argument("no music support");
	Mix_Music *tmp = Mix_LoadMUS((get_sound_dir() + filename).c_str());
	if (!tmp) {
		throw file_read_error(filename);
	}
	playlist.push_back(filename);
	musiclist.push_back(tmp);
}



void music::exec_set_playback_mode(playback_mode pbm_)
{
	if (!use_music) throw std::invalid_argument("no music support");
	pbm = pbm_;
}



void music::exec_play(unsigned fadein)
{
	if (!use_music) throw std::invalid_argument("no music support");
	if (!Mix_PlayingMusic()) {
		start_play_track(current_track, fadein);
	} else {
		throw std::runtime_error("music still playing, can't execute play()");
	}
}



void music::exec_stop(unsigned fadeout)
{
	if (!use_music) throw std::invalid_argument("no music support");
	if (Mix_PausedMusic())
		Mix_ResumeMusic();
	if (Mix_PlayingMusic()) {
		stopped = true;
		if (fadeout > 0)
			Mix_FadeOutMusic(int(fadeout));
		else
			Mix_HaltMusic();
	} else {
		throw std::runtime_error("music not playing, can't execute stop()");
	}
}



void music::exec_pause()
{
	if (!use_music) throw std::invalid_argument("no music support");
	if (Mix_PlayingMusic() && !Mix_PausedMusic())
		Mix_PauseMusic();
}



void music::exec_resume()
{
	if (!use_music) throw std::invalid_argument("no music support");
	Mix_ResumeMusic();
}



void music::exec_play_track(unsigned nr, unsigned fadeouttime, unsigned fadeintime)
{
	if (!use_music) throw std::invalid_argument("no music support");
	try {
		exec_stop(fadeouttime);
	}
	catch (exception& e) {
		// couldn't stop, so music was already stopped?
		current_track = nr;
		exec_play(fadeintime);
		return;
	}
	usersel_next_track = int(nr);
	usersel_fadein = fadeintime;
}



void music::exec_track_finished()
{
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



void music::exec_get_playlist(std::vector<std::string>& playlist_)
{
        playlist_ = playlist;
}



void music::exec_get_current_track(unsigned& track)
{
	track = current_track;
}



void music::exec_is_playing(bool& isply)
{
	if (!Mix_PlayingMusic() || Mix_PausedMusic())
		throw std::runtime_error("music not playing");

}



void music::exec_play_sfx(const std::string& category, const vector3& listener, angle listener_dir, const vector3& noise_pos)
{
	if (!use_music) throw std::invalid_argument("no music support");
	map<string, vector<Mix_Chunk*> >::iterator it = sfx_events.find(category);
	if (it == sfx_events.end())
		throw invalid_argument(string("unknown category for sfx: ") + category);
	// chose random sound of category
	unsigned snr = unsigned(it->second.size() * rnd());
	Mix_Chunk* chk = it->second[snr];

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
			
		// dist for Mix_SetPosition() is calculated as a value between 0 and 255 with 0 being the loudest
		// (closest)			
		int dist = (int)(distanceFromPlayer / hearing_increment);
			
		int channel_num = Mix_PlayChannel(-1, chk, 0);
		if (channel_num < 0)
			throw sdl_error("unable to play sfx"); // Mix_GetError() here...
		if (!Mix_SetPosition(channel_num, (short)bearingFromPlayer.value(), dist))
			throw sdl_error("Mix_SetPosition() failed");
	}
}



void music::exec_play_sfx_machine(const std::string& name, unsigned throttle)
{
	if (!use_music) throw std::invalid_argument("no music support");
	map<string, vector<Mix_Chunk*> >::iterator it = sfx_machines.find(name);
	if (it == sfx_machines.end())
		throw invalid_argument(string("unknown machine name: ") + name);
	unsigned nrthr = it->second.size();
	unsigned thr = throttle * (nrthr+1)/100;
	if (thr == 0) {
		// stop machine
		if (Mix_Playing(SFX_CHANNEL_MACHINE))
			Mix_HaltChannel(SFX_CHANNEL_MACHINE);
		current_machine_sfx = 0;
		return;
	}
	thr = std::min(thr-1, nrthr-1);

	Mix_Chunk* chk = it->second[thr];
	if (chk == current_machine_sfx) {
		// nothing to do, already playing
		return;
	}
	// stop old sfx
	if (Mix_Playing(SFX_CHANNEL_MACHINE))
		Mix_HaltChannel(SFX_CHANNEL_MACHINE);
	current_machine_sfx = 0;
	if (Mix_PlayChannel(SFX_CHANNEL_MACHINE, chk, -1) < 0)
		throw sdl_error("can't play channel");
	current_machine_sfx = chk;
}



void music::exec_pause_sfx(bool on)
{
        if (on)
		Mix_Pause(-1);
	else
		Mix_Resume(-1);
}
