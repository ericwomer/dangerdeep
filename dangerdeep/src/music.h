// music
// subsim (C)+(W) Markus Petermann and Thorsten Jordan. SEE LICENSE

/*
   class music doesn't deal with SDL audio initialization since It assumes it was already inited by sound class
*/


#ifndef _MUSIC_H_
#define _MUSIC_H_

#include "sound.h"

#include <vector>

using std::vector;
using std::string;

class music
{
 protected:

  int ml_item;
  int pl_item;
  int fade_time;
  bool pl_mode;
  bool shuffle;
  string dir;

  vector<Mix_Music *> musiclist;

  vector<int> playlist; // not implemented yet

  void load_list();
  int next_in_list();

 public:

  enum{ stopped, playing, paused };

  static bool use_music;

  music();
  music(const string& dir);
  ~music();

  void deinit();  // used for waiting for a fade_out before killing the

  void load(const string& filename);
  void load_musiclist();
  void load_musiclist(const string& filename);  // will be used if we desire loading files dynamically
  void unload_musiclist();

  void _play(int music);
  void stop();
  void pause();
  void resume();
  void next();

  void fade_in(int item,int timeout=0);   // _fade_in() + pl_mode updates
  void _fade_in(int music,int timeout=0);
  void fade_out(int timeout=0);
  void _fade_out(int timeout=0);
  void _fade_to(int music, int timeout=0);
  void fade_to(int item,int timeou=0);     // _fade_to() + pl_mode update

  void fade_next(int timeout=0);

  int _current() { return ml_item; }
  int current() { return ((pl_mode)? pl_item : -1); }
  int status();

  void set_fade_time(int timeout){ fade_time = timeout; }
  void shuffle_mode(bool onoff) { shuffle = onoff; }

  void load_playlist(const string& filename);
  void unload_playlist();
  int playlist_size(){ return playlist.size(); }
};

#endif /* __MUSIC_H_ */

