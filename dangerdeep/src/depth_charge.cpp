// depth charges
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "depth_charge.h"
#include "global_data.h"
#include "model.h"
#include "system.h"
#include "game.h"

depth_charge::depth_charge(const sea_object& parent, double expl_depth)
{
	sea_object::init();
	position = parent.get_pos();	// fixme bei schiffen + length/2 bla...
	heading = parent.get_heading();	// not used
	head_to = 0;
	length = 1;
	width = 1;
	explosion_depth = expl_depth;	// fixme: the dcs seem to keep falling eternally after an explosion?!
	speed = 0;
	max_speed = 0;
	max_rev_speed = 0;
	throttle = stop;
	system::sys()->add_console("depth charge created");
}

void depth_charge::simulate(game& gm, double delta_time)
{
	sea_object::simulate(gm, delta_time);	// nothing happens
	position.z -= DEPTH_CHARGE_SINK_SPEED * delta_time;
	if (position.z < -explosion_depth) {
		// are subs affected?
		// fixme: ships can be damaged by DCs also...
		list<submarine*>& submarines = gm.get_submarines();
		for (list<submarine*>::iterator it = submarines.begin(); it != submarines.end(); ++it) {
			double deadly_radius = DEADLY_DC_RADIUS_SURFACE +
				(*it)->get_pos().z * DEADLY_DC_RADIUS_200M / 200;
			vector3 special_dist = (*it)->get_pos() - get_pos();
			special_dist.z *= 2;	// depth differences change destructive power
			if (special_dist.length() <= deadly_radius) {
				system::sys()->add_console("depth charge hit!");
				(*it)->kill();	// sub is killed.
			}
			// fixme handle damages!
		}
		kill();	// dc is "dead"
		gm.dc_explosion(position);
	}
}

void depth_charge::display(void) const
{
	depth_charge_mdl->display();
}
