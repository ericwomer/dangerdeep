// depth charges
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "gun_shell.h"
#include "global_data.h"
#include "model.h"
#include "system.h"
#include "game.h"

gun_shell::gun_shell(const sea_object& parent, angle direction, angle elevation,
	double initial_velocity) : sea_object()
{
	position = parent.get_pos();	// fixme: calc correct position
	heading = direction;
	length = 0.2;
	width = 0.2;
	v0 = initial_velocity;
	t = 0;
	speed = v0;
	alpha = elevation;
	system::sys()->add_console("shell created");
	cross_section_factor = CROSS_SECTION_VIS_NULL;;
}

void gun_shell::simulate(game& gm, double delta_time)
{
	t += delta_time;
	speed = v0*exp(-AIR_RESISTANCE*t/v0);	// not needed
	double curvepos = v0*v0/AIR_RESISTANCE * (1.0 - exp(-AIR_RESISTANCE*t/v0));
	position.z = alpha.sin() * curvepos - GRAVITY * t*t/2;
	vector2 deltapos = heading.direction() * curvepos * alpha.cos();
	position.x += deltapos.x;
	position.y += deltapos.y;
	if (position.z <= 0) {
		gm.gs_impact(position);
		kill();
	}
}

void gun_shell::display(void) const
{
	gun_shell_mdl->display();
}
