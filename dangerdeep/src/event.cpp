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

// game event
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "event.h"
#include "user_interface.h"
#include "texts.h"
#include "music.h"
#include "particle.h"
#include "game.h"

void event_torpedo_dud_shortrange::evaluate(user_interface& ui)
{
	ui.add_message(texts::get(59));
}



void event_torpedo_dud::evaluate(user_interface& ui)
{
	ui.add_message(texts::get(60));
}



void event_ship_sunk::evaluate(user_interface& ui)
{
	ui.add_message(texts::get(83));
}



void event_preparing_to_dive::evaluate(user_interface& ui)
{
	ui.add_message(texts::get(125));
}



void event_diving::evaluate(user_interface& ui)
{
	ui.add_message(texts::get(129));
}



void event_unmanning_gun::evaluate(user_interface& ui)
{
	ui.add_message(texts::get(126));
}



void event_gun_manned::evaluate(user_interface& ui)
{
	ui.add_message(texts::get(127));
}



void event_gun_unmanned::evaluate(user_interface& ui)
{
	ui.add_message(texts::get(128));
}



void event_depth_charge_in_water::evaluate(user_interface& ui)
{
	ui.add_message(texts::get(205));
	ui.play_sound_effect(SFX_DEPTH_CHARGE_LAUNCH, source);
}



void event_depth_charge_exploding::evaluate(user_interface& ui)
{
	ui.add_message(texts::get(204));
	ui.play_sound_effect(SFX_DEPTH_CHARGE_EXPLODE, source);
}



void event_gunfire_light::evaluate(user_interface& ui)
{
	ui.play_sound_effect(SFX_DECK_GUN_FIRE, source);
}



void event_gunfire_medium::evaluate(user_interface& ui)
{
	ui.play_sound_effect(SFX_MEDIUM_GUN_FIRE, source);
}



void event_gunfire_heavy::evaluate(user_interface& ui)
{
	ui.play_sound_effect(SFX_BIG_GUN_FIRE, source);
}



void event_shell_explosion::evaluate(user_interface& ui)
{
	ui.play_sound_effect(SFX_SHELL_EXPLODE, source);
}



void event_shell_splash::evaluate(user_interface& ui)
{
	ui.play_sound_effect(SFX_SHELL_SPLASH, source);
}



void event_ship_collision::evaluate(user_interface& ui)
{
	// modifying game because of events is probably wrong, but here we use it for debugging only.
#if 1
	ui.get_game().spawn_particle(new marker_particle(position));
#endif
}



void event_torpedo_explosion::evaluate(user_interface& ui)
{
	ui.play_sound_effect(SFX_SHELL_EXPLODE /* what else?! */, source);
}



void event_ping::evaluate(user_interface& ui)
{
	ui.play_sound_effect(SFX_PING, source);
}



void event_tube_reloaded::evaluate(user_interface& ui)
{
	ui.add_message(texts::get_replace(184, tube_nr));
}
