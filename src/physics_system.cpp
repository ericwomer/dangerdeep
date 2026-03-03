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

// Physics system - manages collision detection and response
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "physics_system.h"
#include "bv_tree.h"
#include "log.h"
#include "matrix4.h"
#include "sea_object.h"
#include "ship.h"
#include "vector3.h"
#include <algorithm>

void physics_system::check_collisions(const std::vector<ship *> &allships) {
    // torpedoes are special... check collision only for impact fuse?
    // Torpedoes are at the beginning of allships vector, determined by m
    unsigned m = 0; // Note: torpedoes count would need to be passed in or determined differently
    
    // now check for collisions for all ships idx i with partner index > max(m,i)
    // so we have N^2/2 tests and not N^2.
    // we don't check for torpedo<->torpedo collisions.
    for (unsigned i = 0; i < allships.size(); ++i) {
        const vector3 &actor_pos = allships[i]->get_pos();
        // use partner's position relative to actor
        bv_tree::param p0 = allships[i]->compute_bv_tree_params();
        for (unsigned j = std::max(i + 1, m); j < allships.size(); ++j) {
            const vector3 &partner_pos = allships[j]->get_pos();
            matrix4 rel_trans = matrix4::trans(partner_pos - actor_pos);
            bv_tree::param p1 = allships[j]->compute_bv_tree_params();
            p1.transform = rel_trans * p1.transform;
#if 0
			std::list<vector3f> contact_points;
			bool intersects = bv_tree::collides(p0, p1, contact_points);
			if (intersects) {
				// compute intersection pos, sum of contact points
				vector3f sum;
				unsigned sum_count = 0;
				for (std::list<vector3f>::iterator it = contact_points.begin(); it != contact_points.end(); ++it) {
					sum += *it;
					++sum_count;
				}
				sum *= 1.0f/sum_count;
				collision_response(*allships[i], *allships[j], vector3(sum) + actor_pos);
			}
#else
            vector3f contact_point;
            bool intersects = bv_tree::closest_collision(p0, p1, contact_point);
            if (intersects) {
                collision_response(*allships[i], *allships[j], contact_point + actor_pos);
            }
#endif
        }
    }

    // collision response:
    // the two objects collide at a position P that is relative to their center
    // (P(a) and P(b)). We have to compute their velocity (direction and strength,
    // a vector), which can be derived from the rigid body state variables.
    // The direction of the response force is orthogonal to the surface normal,
    // but maybe direction of P(a)-P(b) would do it as well. In theory response
    // vector is just +/- (P(a)-P(b)), for A/B (or vice versa), multiplied by some
    // dampening factor simulating friction.
    // just apply the force instantly to the body states to change their velocities
    // and we are done...

    // fixme: collision checks between fast moving small objects and bigger objects
    // (like shells vs. ships) should be done here too, and not only in gun_shell
    // class. Later other objects may need that code too (machine cannons, guns etc).

    // fixme remove obsolete code from bbox/voxel collision checking
}

void physics_system::collision_response(sea_object &a, sea_object &b, const vector3 &collision_pos) {
#if 0
	// for debugging - fixme not visible. is position correct?!
	// Would need game reference to spawn particles
	// spawn_particle(std::make_unique<marker_particle>(collision_pos));
#endif
    // compute directions to A, B to compute collision response direction
    const vector3 &A = a.get_pos();
    const vector3 &B = b.get_pos();
    // fixme: relative position is ok, why there arent any visible markers?!
    //	printf("pos %f %f %f   a %f %f %f    b %f %f %f\n",collision_pos.x,collision_pos.y,collision_pos.z,
    //	       A.x,A.y,A.z,B.x,B.y,B.z);
    vector3 dA = (A - collision_pos).normal();
    vector3 dB = (B - collision_pos).normal();
    vector3 N;
    if (dA * dB < 1e-4) {
        N = dA;
    } else {
        N = dA.cross(dB).normal().cross((dA + dB).normal()).normal();
    }
    log_debug("collision response dir=" << N);

    // compute speed of A and B at the collision point, compute opposing force and
    // apply directly to A, B, modifying their speed
    // we need to compute velocity at collision point of A, B.
    // the scalar product with N gives collision speed, its negative value
    // must be dampened and applied as force along N.
    // clip speed so positive values along N are not used, i.e. when collision response
    // has already been applied, and velocity vector points already along N, do not
    // apply it again, even if objects still collide at next physics step.
    // Speed at collision point is linear speed plus relative vector cross linear velocity
    // add a function to sea_object to compute linear speed of any relative point.
    // fixme
    vector3 vA = a.compute_linear_velocity(collision_pos);
    vector3 vB = b.compute_linear_velocity(collision_pos);
    double vrel = N * (vA - vB);
    log_debug("linear velocity A=" << vA << "   B=" << vB << " vrel=" << vrel);
    // if the contact points move away each other, do nothing
    if (vrel > 0)
        return;
    const double epsilon = vrel < -4.0 ? 0.5 : 1.0 + vrel / 8.0; // dampening of response force
    double j = -(1.0 + epsilon) * vrel / (a.compute_collision_response_value(collision_pos, N) + b.compute_collision_response_value(collision_pos, N));
    log_debug("j=" << j << " force=" << (j * N));
    a.apply_collision_impulse(collision_pos, j * N);
    b.apply_collision_impulse(collision_pos, -j * N);
}
