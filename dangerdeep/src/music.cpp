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
#include <sstream>
#include <iostream>
#include <fstream>
#include <cmath>
#include <SDL.h>
#include <SDL_mixer.h>

using namespace std;


bool music::use_music = true;
music* music::instance = 0;


music::music()
	: current_track(0),
	  track_at_end(false),
	  pbm(PBM_LOOP_LIST),
	  stopped(true)
{
	if (instance) {
		throw error("only one instance of class music at a time valid");
	}
	instance = this;
	Mix_HookMusicFinished(track_finished);
	sys().set_periodical_function(check_playback_caller, this);
}



music::~music()
{
	sys().set_periodical_function(0, 0);
	halt();
  
	for (vector<Mix_Music*>::iterator it = musiclist.begin(); it != musiclist.end(); ++it) {
		Mix_FreeMusic(*it);
	}

	instance = 0;
}



void music::append_track(const std::string& filename)
{
	if (use_music) {
		Mix_Music *tmp = Mix_LoadMUS((get_sound_dir() + filename).c_str());
		if (!tmp) {
			throw file_read_error(filename);
		}
		playlist.push_back(filename);
		musiclist.push_back(tmp);
	}
}



void music::set_playback_mode(playback_mode pbm_)
{
	pbm = pbm_;
}



void music::play(unsigned fadein)
{
	if (!Mix_PlayingMusic()) {
		start_play_track(current_track, fadein);
	}
}



void music::stop(unsigned fadeout)
{
	if (Mix_PausedMusic())
		Mix_ResumeMusic();
	if (Mix_PlayingMusic()) {
		stopped = true;
		if (fadeout > 0)
			Mix_FadeOutMusic(int(fadeout));
		else
			Mix_HaltMusic();
	}
}



void music::pause()
{
	if (Mix_PlayingMusic() && !Mix_PausedMusic())
		Mix_PauseMusic();
}



void music::resume()
{
	Mix_ResumeMusic();
}



bool music::is_playing() const
{
	return Mix_PlayingMusic() && !Mix_PausedMusic();
}



void music::play_track(unsigned nr, unsigned fadeouttime, unsigned fadeintime)
{
	stop(fadeouttime);
	start_play_track(nr, fadeintime);
}



void music::check_playback()
{
	if (!track_at_end)
		return;
	if (!stopped) {
		play_next_track(0);
		track_at_end = false;
	}
}



void music::halt()
{
	if (Mix_PlayingMusic()) {
		stopped = true;
		if (Mix_PausedMusic()) {
			Mix_HaltMusic();
		} else {
			while (Mix_FadingMusic()) {
				SDL_Delay(50);
			}
		}
	}
}



void music::play_next_track(unsigned fadetime)
{
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
	start_play_track(current_track, fadetime);
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
		if (err < 0) {
			// music failed playing...
			sys().add_console("music playing failed.");
		} else {
			stopped = false;
			// avoid callback for old data
			track_at_end = false;
		}
	}
}



void music::track_finished()
{
	if (!instance)
		return;	// should never happen
	instance->track_at_end = true;
}



void music::check_playback_caller(void* musicptr)
{
	((music*)musicptr)->check_playback();
}
