
#include "system.h"
#include "music.h"

#include <sstream>
#include <iostream>
#include <fstream>
#include <cmath>

using namespace std;

bool music::use_music = true;

void music::load( const string& filename )
{
  if( use_music ){

    Mix_Music *tmp = Mix_LoadMUS( filename.c_str() );
    if( !tmp ){
      ostringstream oss;
      oss << "Unable to load Music file: " << Mix_GetError();
      sys().add_console(oss.str());
      return;
    }
    musiclist.push_back(tmp);
  }
}

void music::load_playlist(const string& filename)
{
}

void music::load_musiclist()
{
   const char* files[] = { "ImInTheMood.ogg" , "Betty_Roche-Trouble_Trouble.ogg" };
   int numfiles = 2;

   for( int i=0; i<numfiles; i++ ){
     load( dir + string(files[i]) );
   }
}

void music::unload_musiclist()
{
  if( Mix_PlayingMusic() ){
    if( Mix_PausedMusic() )
      Mix_HaltMusic();
    else
      do{}while(Mix_FadingMusic());
  }
  
  vector<Mix_Music *>::iterator it;
  for( it=musiclist.end(); it!=musiclist.end(); it-- ){
    Mix_FreeMusic( *it );
    musiclist.pop_back();
  }  
}

music::music() : ml_item(0), pl_item(0), fade_time(2000), pl_mode(false), shuffle(false), dir("")
{
  musiclist.reserve(20);  // allow up to 20 different tunes loaded to prevent resizing
  playlist.reserve(20);   // allow up to 20 different tunes in the playlist
  load_musiclist();
}

music::music(const string& _dir) : ml_item(0), pl_item(0), fade_time(2000), pl_mode(false), shuffle(false), dir(_dir)
{
  musiclist.reserve(14);
  load_musiclist();
}

music::~music()
{
  unload_musiclist();
}

int music::next_in_list()
{
  if( playlist.size()==1 ) return 0;

  if( shuffle ){
    int r;
    do{
      srand((rand()%rand()));
      r = rand()%playlist.size();
    } while( r == pl_item );
    return r;
  } else {
    return ((pl_item+1)> playlist.size())? 0 : (pl_item+1);
  }
}

void music::_play(int music)
{
  if( musiclist.size()<1 ) return;

  int which = ( music<0 || music>musiclist.size() ) ? 0 : music;

  if( !use_music || !musiclist[which] ) return;

  if( Mix_PlayMusic( musiclist[which], 0 )==-1 ){
    ostringstream oss;
    oss << "Unable to play Music file: " << Mix_GetError();
    sys().add_console(oss.str());
  } else ml_item = which;
  pl_mode = false;
}

void music::stop()
{
  if(Mix_PausedMusic()) Mix_ResumeMusic();
  Mix_HaltMusic();
}

void music::pause()
{
  if( Mix_PlayingMusic() && !Mix_PausedMusic() ) Mix_PauseMusic();
}

void music::resume()
{
  Mix_ResumeMusic();
}

int music::status()
{
  if( !Mix_PlayingMusic() )
    return stopped;
  else
    return Mix_PausedMusic() ? paused : playing;
}

void music::_fade_in(int music, int timeout)
{
  int which = ( music<0 || music>=musiclist.size() )? 0 : music ;

  Mix_FadeInMusic( musiclist[which], 0, (timeout==0)? fade_time : timeout );
  ml_item = which;
  pl_mode = false;
}

void music::fade_in(int music, int timeout)
{
  int which = ( music<0 || playlist.size() )? 0 : music;

  Mix_FadeInMusic( musiclist[playlist[which]], 0, (timeout==0)? fade_time : timeout );
  ml_item = playlist[which];
  pl_item = which;
  pl_mode = true;
}

void music::_fade_out(int timeout)
{
  fade_out(timeout);
}

void music::fade_out(int timeout)
{
  if( Mix_PlayingMusic() ){
    do{}while(Mix_FadingMusic());
    if( Mix_PausedMusic() )
      Mix_HaltMusic();
    else {
      Mix_FadeOutMusic( (timeout==0)? fade_time : timeout );
    }
  }
}

void music::_fade_to(int music, int timeout)
{
  if( Mix_PlayingMusic() ){
    do{}while(Mix_FadingMusic());
    if( !Mix_PausedMusic() )
      fade_out( timeout );
    else Mix_HaltMusic();
  }
  _fade_in( music, timeout );
}

void music::fade_to(int music, int timeout)
{
  if( Mix_PlayingMusic() ){
    do{}while(Mix_FadingMusic());
    if( !Mix_PausedMusic() )
      fade_out( timeout );
    else Mix_HaltMusic();
  }
  fade_in( music, timeout );
}

void music::fade_next(int timeout)
{
  int music = next_in_list();
 
  if( Mix_PlayingMusic() ){
    do{}while(Mix_FadingMusic());
    if( Mix_PausedMusic() )
      Mix_HaltMusic();
    else
      fade_out( timeout );
  }
  _fade_in( playlist[music], timeout );
}
