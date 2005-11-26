// depth charges
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "depth_charge.h"
#include "global_data.h"
#include "model.h"
#include "system.h"
#include "game.h"



depth_charge::depth_charge(game& gm_)
	: sea_object(gm_, "depth_charge.3ds"), explosion_depth(0)
{
	// for loading
}



depth_charge::depth_charge(game& gm_, double expl_depth, const vector3& pos)
	: sea_object(gm_, "depth_charge.3ds"), explosion_depth(expl_depth)
{
	// fixme depends on parent! and parent's size, dc's can be thrown, etc.!
	position = pos;
	system::sys().add_console("depth charge created");
}



void depth_charge::load(const xml_elem& parent)
{
	sea_object::load(parent);
	explosion_depth = parent.child("explosion_depth").attrf();
}



void depth_charge::save(xml_elem& parent) const
{
	sea_object::save(parent);
	parent.add_child("explosion_depth").set_attr(explosion_depth);
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
