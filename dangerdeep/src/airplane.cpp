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

// airplanes
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "airplane.h"
#include "model.h"
#include "system.h"
#include "global_data.h"
#include "environment.h"



airplane::airplane(game& gm_, const xml_elem& parent)
	: sea_object(gm_, parent), rollfac(0), pitchfac(0)
{
}



void airplane::load(const xml_elem& parent)
{
	sea_object::load(parent);
	xml_elem ma = parent.child("air_motion");
	rollfac = ma.attrf("rollfac");
	pitchfac = ma.attrf("pitchfac");
}



void airplane::save(xml_elem& parent) const
{
	sea_object::save(parent);
	xml_elem ma = parent.add_child("air_motion");
	ma.set_attr(rollfac, "rollfac");
	ma.set_attr(pitchfac, "pitchfac");
}



void airplane::simulate(double delta_time)
{
	quaternion invrot = orientation.conj();
	vector3 localvelocity = invrot.rotate(velocity);
		
	vector3 locx = orientation.rotate(1, 0, 0);
	vector3 locy = orientation.rotate(0, 1, 0);
	vector3 locz = orientation.rotate(0, 0, 1);

/*
	fixme: 2004/06/18
	the whole model is slightly wrong. When a plane turns, some speed is lost, but the
	speed doesn't reduce to the part of the old speed vector that is common with the
	new one. E.g. with the old model a turn from north to east (90 degr.) would drop
	the speed to zero, a turn to 60 deg, would have speed, etc. that's not true.
	E.g. a car that turns keeps moving even when changing the direction (without applying
	further force to the wheels).
	So the model would be like this:
	Forces:
	Thrust: along plane's local y-axis.
	Drag: along every axis the plane moves, depends on orientation.
	Lift: along global z-axis, depends on orientation, more precisy along local z-axis!
	Gravity: along global z-axis.
	Speed of the plane is along local y-axis (Mass*velocity = impulse)
	and is not affected by turning (at least not that much).
	The turn rate is influenced by speed and other forces. The more impulse a plane
	has the harder it is to turn (change direction of impulse).
	This has to be modelled with inertia, rotational impulse, torque etc.
*/

	// fixme: the plane's orientation must change with velocity:
	// when rolling the plane to the side, it is lifted hence changing the course.
	// this means that the plane changes its orientation too!
	// according to wind (spatial velocity) it turns its nose!
	// this would explain why the speed drops when making a dive (for now!):
	// the plane can dive at its specific rate no matter how strong the wind resistance
	// is - if the plane would change its orientation with respect to spatial velocity
	// it couldn't turn or dive that fast, allowing the speed to catch up...

/*	
	vector3 winddir = localvelocity.normal();
	double windturnangle = acos(locy * winddir) * 180.0 / M_PI;
	if (windturnangle <= 0.0001) windturnangle = 0.0;
	vector3 windturnaxis = (windturnangle > 0.0001) ? locy.cross(winddir).normal() : vector3(0, 0, 0);
*/	
/*
cout << "winddir      " << winddir << "\n";	
cout << "windta      " << windturnaxis << "\n";	
cout << "windtan      " << windturnangle << "\n";	
cout << "windtal      " << windturnaxis.length() << "\n";	
*/
//	double windturnfactor = 0.333333;	// 1/sec
//	quaternion windrotation = quaternion::rot(windturnfactor * windturnangle * delta_time, windturnaxis);

	// forces:
	// thrust: engine thrust along local y-axis
	// lift: wing lift along local z-axis, proportional to square of speed
	// gravity*mass: along negative global z-axis

	// fixme: simulate stall: if speed drops below a specific quantum, the plane's nose
	// drops down. This avoids negative values for speed
//	if (speed < 0) {speed = 0; speed_sq = 0;}

	// avoid negative speeds/forces fixme

	// compute forces	
	// propulsion by engine (thrust)
	vector3 thrust = get_engine_thrust() * locy;
	// lift by wings (fixme: works also if plane is upside down or nearly!)
	vector3 lift = (localvelocity.y * localvelocity.y * get_lift_factor()) * locz;//fixme: negate locz if locz.z<0
	// gravity
	vector3 gravity = vector3(0, 0, get_mass() * (0 - GRAVITY));


	// deceleration by air friction (drag etc.)
	vector3 airfriction = orientation.rotate(vector3(
		-mysgn(localvelocity.x) * localvelocity.x * localvelocity.x * get_antislide_factor(),
		-mysgn(localvelocity.y) * localvelocity.y * localvelocity.y * get_drag_factor(),
		-mysgn(localvelocity.z) * localvelocity.z * localvelocity.z * get_antilift_factor()
		));

/*
cout << "global velocity " << velocity << "\n";
cout << "local velocity  " << localvelocity << "\n";
cout << "thrust          " << thrust << "\n";
cout << "lift            " << lift << "\n";
cout << "gravity         " << gravity << "\n";
cout << "air friction    " << airfriction << "\n";
*/

	// update position and speed, fixme move to get_acceleration()!
	vector3 accel = (thrust + lift + gravity) * (1.0/get_mass()) + airfriction;
	position += velocity * delta_time + accel * (0.5 * delta_time * delta_time);
	velocity += accel * delta_time;

	quaternion qpitch = quaternion::rot(pitchfac * get_pitch_deg_per_sec() * delta_time, 1, 0, 0); // fixme: also depends on speed
	quaternion qroll = quaternion::rot(rollfac * get_roll_deg_per_sec() * delta_time, 0, 1, 0); // fixme: also depends on speed
	orientation *= qpitch * qroll;
	// * windrotation;

//	if ( myai )
//		myai->act(gm, delta_time);

//	// Adjust fuel_level.
//	calculate_fuel_factor ( delta_time );
}



// this all should be replaced by rudder states, fixme
void airplane::roll_left()
{
	rollfac = -1;
}



void airplane::roll_right()
{
	rollfac = 1;
}



void airplane::roll_zero()
{
	rollfac = 0;
}



void airplane::pitch_down()
{
	pitchfac = -1;
}



void airplane::pitch_up()
{
	pitchfac = 1;
}



void airplane::pitch_zero()
{
	pitchfac = 0;
}
