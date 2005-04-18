// depth charges
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "depth_charge.h"
#include "global_data.h"
#include "model.h"
#include "system.h"
#include "game.h"



depth_charge::depth_charge(game& gm_) : sea_object(gm_)
{
}



depth_charge::depth_charge(game& gm_, const sea_object& parent, double expl_depth) : sea_object(gm_)
{
	position = parent.get_pos();	// fixme depends on parent! and parent's size, dc's can be thrown, etc.!
	explosion_depth = expl_depth;
	system::sys().add_console("depth charge created");
}



depth_charge::~depth_charge()
{
}



void depth_charge::load(istream& in)
{
	sea_object::load(in);
	explosion_depth = read_double(in);
}



void depth_charge::save(ostream& out) const
{
	sea_object::save(out);
	write_double(out, explosion_depth);
}



void depth_charge::simulate(double delta_time)
{
	sea_object::simulate(delta_time);
	if (is_defunct()) return;
	if (position.z < -explosion_depth) {
		gm.dc_explosion(*this);
		kill();	// dc is "dead"
	}
}



vector3 depth_charge::get_acceleration(void) const
{
	if (position.z > 0) {	// DC's can be thrown, so they can be above water.
		return vector3(0, 0, -GRAVITY);
	} else {
		double vm = velocity.z/DEPTH_CHARGE_SINK_SPEED;
		return vector3(0, 0, -GRAVITY + GRAVITY*vm*vm);
	}
}



void depth_charge::display(void) const
{
	depth_charge_mdl->display();	// fixme: replace by modelcache
}
