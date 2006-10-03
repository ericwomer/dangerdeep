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

// music
// subsim (C)+(W) Markus Petermann and Thorsten Jordan. SEE LICENSE

/*
   class music doesn't deal with SDL audio initialization since It assumes it was already inited by sound class
*/


#ifndef _MUSIC_H_
#define _MUSIC_H_

#include <string>
#include <vector>
#include <SDL_mixer.h>

///\brief Handles music and background songs.
class music
{
 public:
	///> which mode to use when playing tracks from a play list
	enum playback_mode {
		PBM_LOOP_LIST,
		PBM_LOOP_TRACK,
		PBM_SHUFFLE_TRACK
	};

	///> create music handler
	music();

	///> destroy music handler
	~music();

	///> append entry to play list
	void append_track(const std::string& filename);

	///> set playback mode
	void set_playback_mode(playback_mode pbm);

	///> start playing music, optionally fade it in
	void play(unsigned fadein = 0);

	///> stop playing music, optionally fade it out
	void stop(unsigned fadeout = 0);

	///> pause music play
	void pause();

	///> resume music play
	void resume();

	///> switch playback to track
	void play_track(unsigned nr, unsigned fadeouttime = 0, unsigned fadeintime = 0);

	///> check play state, will advance to next track if music finished, call that every frame
	void check_playback();

	///> set to false if you don't want music.
	static bool use_music;

 protected:
	unsigned current_track;
	bool track_at_end;
	playback_mode pbm;
	bool stopped;
	std::vector<std::string> playlist;
	// we can't use the ptrvector here, since Mix_Music ptrs must be freed with special function.
	std::vector<Mix_Music*> musiclist;
	static music* instance;

	void halt();
	void play_next_track(unsigned fadetime);
	void start_play_track(unsigned nr, unsigned fadeintime = 0);
	static void track_finished();
	static void check_playback_caller(void* musicptr);
};

#endif /* __MUSIC_H_ */
