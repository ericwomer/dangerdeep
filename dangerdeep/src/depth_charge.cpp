// depth charges
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "depth_charge.h"
#include "global_data.h"
#include "model.h"
#include "system.h"
#include "game.h"



depth_charge::depth_charge(const sea_object& parent, double expl_depth) : sea_object()
{
	position = parent.get_pos();	// fixme depends on parent! and parent's size, dc's can be thrown, etc.!
/*	heading = parent.get_heading();	// not used
	head_to = 0;
	length = 1;
	width = 1;*/
	explosion_depth = expl_depth;
/*	speed = 0;
	max_speed = 0;
	max_rev_speed = 0;
	throttle = stop;*/
	system::sys().add_console("depth charge created");
}



void depth_charge::load(istream& in, class game& g)
{
	sea_object::load(in, g);
	explosion_depth = read_double(in);
}



void depth_charge::save(ostream& out, const class game& g) const
{
	sea_object::save(out, g);
	write_double(out, explosion_depth);
}



void depth_charge::simulate(game& gm, double delta_time)
{
	sea_object::simulate(gm, delta_time);
	if (position.z < -explosion_depth) {
		gm.dc_explosion(*this);
		kill();	// dc is "dead"
	}
}



vector3 depth_charge::get_acceleration(void) const
{
	double vm = velocity.z/DEPTH_CHARGE_SINK_SPEED;
	return vector3(0, 0, -GRAVITY + GRAVITY*vm*vm);
}



void depth_charge::display(void) const
{
	depth_charge_mdl->display();
}
