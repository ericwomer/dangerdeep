// depth charges
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "gun_shell.h"
#include "global_data.h"
#include "model.h"
#include "system.h"
#include "game.h"

gun_shell::gun_shell(const sea_object& parent, angle direction, angle elevation,
	double initial_velocity)
{
	init_empty();
	position = parent.get_pos();	// fixme: calc correct position
	heading = direction;
	length = 0.2;
	width = 0.2;
	v0 = initial_velocity;
	t = 0;
	speed = v0;
	alpha = elevation;
	system::sys()->add_console("shell created");
}

void gun_shell::simulate(game& gm, double delta_time)
{
	t += delta_time;
	speed = v0*exp(-AIR_RESISTANCE*t/v0);
	position.z = alpha.sin() * t * speed - GRAVITY * t*t/2;
	vector2 deltapos = heading.direction() * speed * alpha.cos();
	position.x += deltapos.x;
	position.y += deltapos.y;
	if (position.z <= 0) {
		list<ship*>& ships = gm.get_ships();
		for (list<ship*>::iterator it = ships.begin(); it != ships.end(); ++it) {
			// fixme: we need a special collision detection, because
			// the shell is so fast that it can be collisionfree with *it
			// delta_time ago and now, but hit *it in between
			if (is_collision(*it)) {
				(*it)->damage((*it)->get_pos() /*fixme*/,GUN_SHELL_HITPOINTS);
				return;	// only one hit possible
			}
		}
		list<submarine*>& submarines = gm.get_submarines();
		for (list<submarine*>::iterator it = submarines.begin(); it != submarines.end(); ++it) {
			if (is_collision(*it)) {
				(*it)->damage((*it)->get_pos() /*fixme*/,GUN_SHELL_HITPOINTS);
				return; // only one hit possible
			}
		}
		gm.gs_impact(position);
		kill();
	}
}

void gun_shell::display(void) const
{
	gun_shell_mdl->display();
}
