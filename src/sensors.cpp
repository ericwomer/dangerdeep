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

// Sensors
// subsim (C) + (W). See LICENSE

#include "sea_object.h"
#include "submarine.h"
#include "sensors.h"
#include "game.h"
#include "vector2.h"
#include "particle.h"
#include "angle.h"
#include "global_data.h"

// Class sensor
sensor::sensor ( double range, double detection_cone ) :
	range ( range ), bearing ( 0.0f ), detection_cone ( detection_cone ),
	move_direction ( 1 )
{}

double sensor::get_distance_factor ( double d ) const
{
	double df = 0;
	
	if (d <= range)
	{
		df = range / d;
		df *= df;
	}

	return df;
}

bool sensor::is_within_detection_cone ( const vector2& r, const angle& h ) const
{
	bool within_angle = false;

	if ( detection_cone >= 360.0f )
	{
		// When the detection angle is larger equal 360 degrees
		// the target is everytime within this detection angle.
		within_angle = true;
	}
	else
	{
		angle dir = bearing + h;
		angle dir_to_target = angle ( r );
		angle diff = dir - dir_to_target;
		double delta_angle = diff.value_pm180 ();

		if ( delta_angle >= -detection_cone && delta_angle <= detection_cone )
			within_angle = true;
	}

	return within_angle;
}

void sensor::auto_move_bearing ( sensor_move_mode mode )
{
	if ( detection_cone < 360.0f )
	{
		bearing += 1.5f * move_direction * detection_cone;

		if ( mode == sweep )
		{
			double b = bearing.value ();
			if ( b < 180.0f && b > 90.0f && move_direction > 0 )
				move_direction = -1;
			else if ( b > 180.0f && b < 270.0f && move_direction < 0 )
				move_direction = 1;
		}
	}
}



// Class lookout_sensor
lookout_sensor::lookout_sensor ( lookout_type type ) : sensor ()
{}



bool lookout_sensor::is_detected ( const game* gm, const sea_object* d,
	const sea_object* t ) const
{
	bool detected = false;
	double max_view_dist = gm->get_max_view_distance ();
	vector2 r = t->get_pos ().xy () - d->get_pos ().xy ();
	double dist = r.length ();

	if (dist < max_view_dist)
	{
		if (dist < 1.0) return true;	// avoid divide by zero
	
		// the probabilty of visibility depends on indivial values
		// relative course, distance to and type of watcher.
		// (height of masts, experience etc.), weather fixme
		double vis = t->surface_visibility ( d->get_pos ().xy () );

		// The Malaya has 1500 m² cross section at broadside.
		// It can be seen at maximum distance (30km)
		// 1500/30000 = 0.05, so the factor must be less or equal than that.
		// A destroyer (771m²) is then visible from its broadside at 15420 meters.
		// A corvette (232m²) is then visible from its broadside at 4640 meters.
		// A carrier (1087m²) is then visible from its broadside at 21740 meters.
		// A small tanker (576m²) is then visible from its broadside at 11520 meters.
		// A large freighter (889m²) is then visible from its broadside at 17780 meters.
		// A VIIc sub (120m²) is then visible from its broadside at 2400 meters.
		// The factor is obviously too large. A sub can be seen from its broadside
		// at superb conditions in 5km at least, but that would lead to a factor so
		// that large freighters are visible from 37km (~20sm)!
		// Effects like smoke or wake are ignored here, but are essential, fixme!!!
		// A ship's/sub's speed influenced the visibility, especially for subs!
		// fixme: earth curvature is ignored here!!!
		// fixme: we should visualize the visibility for testing purposes.
		const double visfactor = 0.05;

		// this model ignores special features of visibility for water splashes, particles or grenades...
		// all of these have a cross section of 100 square meters hard coded for testing,
		// except particles, which have a real cross section

		// multiply with overall visibility factor: max_view_dist/30km.
		// the idea behind this formula is that at night smaller objects are harder to detect.
		// however it's results are bad.
		//double condition_visfactor = (max_view_dist/30000.0) * 0.5 + 0.5;

		// visibility depends on visible area in viewer's projected space.
		// Projected area ~ Real area / Real distance.
		if (/*condition_visfactor * */ vis/dist >= visfactor)
			detected = true;
		// fixme: add some randomization! really?
	}

	return detected;
}



bool lookout_sensor::is_detected ( const game* gm, const sea_object* d,
	const particle* p ) const
{
	bool detected = false;
	double max_view_dist = gm->get_max_view_distance ();
	vector2 r = p->get_pos ().xy () - d->get_pos ().xy ();
	double dist = r.length ();

	if (dist < max_view_dist)
	{
		if (dist < 1.0) return true;	// avoid divide by zero
	
		// the probabilty of visibility depends on cross section
		double vis = p->get_width() * p->get_height();

		if (vis < 100.0) vis = 100.0;

		const double visfactor = 0.05;

		// visibility depends on visible area in viewer's projected space.
		// Projected area ~ Real area / Real distance.
		if (vis/dist >= visfactor)
			detected = true;
	}

	return detected;
}



// Class passive_sonar_sensor
passive_sonar_sensor::passive_sonar_sensor ( passive_sonar_type type ) : sensor ()
{
	init ( type );
}

void passive_sonar_sensor::init ( passive_sonar_type type )
{
	switch ( type )
	{
		case passive_sonar_type_default:
			set_range ( 9500.0f );
			break;
		case passive_sonar_type_tt_t5:
			set_range ( 1000.0f );
			set_detection_cone ( 20.0f );
			break;
		case passive_sonar_type_tt_t11:
			set_range ( 1500.0f );
			set_detection_cone ( 40.0f );
			break;
	}
}

bool passive_sonar_sensor::is_detected ( double& sound_level,
	const game* gm, const sea_object* d, const sea_object* t ) const
{
	bool detected = false;
	sound_level = 0.0f;

	// Surfaced submarines detect anything with their passive sonars.
	const submarine* sub = dynamic_cast<const submarine*> ( d );
    if ( sub && !sub->is_submerged () )
    {
		detected = false;
	}
	else
	{
		vector2 r = t->get_engine_noise_source () - d->get_pos ().xy ();

		if ( is_within_detection_cone ( r, d->get_heading () ) )
		{
			double df = get_distance_factor ( r.length () );

			// The throttle speed is the real noise of the ship.
			// A ship on flank speed is really deaf.
			double dnoisefac = d->get_noise_factor ();
			double tnoisefac = t->get_noise_factor ();

			// The noise modificator for the detecting unit must be
			// subtracted from 1.
			dnoisefac = 1.0f - dnoisefac;
			sound_level = dnoisefac * tnoisefac * df;

			if ( sound_level > ( 0.1f + 0.01f * rnd ( 10 ) ) )
				detected = true;
		}
	}

	return detected;
}

bool passive_sonar_sensor::is_detected ( const game* gm, const sea_object* d,
	const sea_object* t ) const
{
	double sound_level;

	return is_detected ( sound_level, gm, d, t );
}



// Class active_sensor
active_sensor::active_sensor ( double range ) : sensor ( range )
{}

double active_sensor::get_distance_factor ( double d ) const
{
	double df = 0;
	
	if (d <= get_range())
	{
		df = get_range () / d;
		df *= df;
		df *= df;
	}

	return df;
}



// Class radar_rensor
radar_sensor::radar_sensor ( radar_type type ) : active_sensor ()
{
	init ( type );
}

void radar_sensor::init ( radar_type type )
{
	switch ( type )
	{
		case radar_type_default:
			set_range  ( 0.0f );
			break;
		case radar_british_type_271:
		case radar_british_type_272:
		case radar_british_type_273:
			set_range(18520); // 10-25 nautical miles
			break;
		case radar_british_type_277:
			set_range(46300); // 25-35 nautical miles
			break;
			
		case radar_german_fumo_29:
			set_range(7500);
			set_bearing(0);
			set_detection_cone(20);
			break;
		case radar_german_fumo_30:	
			set_range(7000);
			set_bearing(0);
			set_detection_cone(60);
			break;
		case radar_german_fumo_61:
		case radar_german_fumo_64:
			set_range(7000);
			break;
		case radar_german_fumo_391:
			set_range(10000);
			break;
	}
}

bool radar_sensor::is_detected ( const game* gm, const sea_object* d,
	const sea_object* t ) const
{
	if (!t) return false; // should not happen

	bool detected = false;
	
	// Surfaced submarines cannot use ASDIC.
	const submarine* dsub = dynamic_cast<const submarine*> ( d );
	if ((dsub && !dsub->is_submerged ()) || !dsub)
	{
		vector2 r = t->get_pos ().xy () - d->get_pos ().xy ();
		if ( is_within_detection_cone ( r, d->get_heading () ) )
		{
			double df = get_distance_factor ( r.length () );
			double vis = 1.0f;
			
			// Radars use the surface visibility factor. 2004/05/16 fixme adapt constants
			vis = t->surface_visibility ( d->get_pos ().xy () );
			
			if ( df * vis > (0.1f + 0.01f * rnd ( 10 )) )
				detected = true;
		}
	}	

	return detected;
}


// Class active_sonar_sensor
active_sonar_sensor::active_sonar_sensor ( active_sonar_type type ) : active_sensor ()
{
	init ( type );
}

void active_sonar_sensor::init ( active_sonar_type type )
{
	switch ( type )
	{
		case active_sonar_type_default:
			set_range ( 1500.0f );
			set_detection_cone ( 15.0f );
			break;
	}
}

bool active_sonar_sensor::is_detected ( const game* gm, const sea_object* d,
	const sea_object* t ) const
{
	bool detected = false;

	// Surfaced submarines cannot use ASDIC.
	const submarine* dsub = dynamic_cast<const submarine*> ( d );
    if ( dsub && !dsub->is_submerged () )
	{
		detected = false;
	}
	else
	{
		// Only submerged submarines can be detected with ASDIC.
		const submarine* tsub = dynamic_cast<const submarine*> ( t );
		if ( tsub && tsub->is_submerged () )
		{
			vector2 r = t->get_pos ().xy () - d->get_pos ().xy ();

			// Verify if target is within detection cone of the sensor.
			if ( is_within_detection_cone ( r, d->get_heading () ) )
			{
				double dist_factor = get_distance_factor ( r.length () );
				// The throttle speed is the real noise of the ship.
				// A ship on flank speed is really deaf.
				double dnoisefac = d->get_noise_factor ();
				// The noise modificator for the detecting unit must be
				// subtracted from 1.
				dnoisefac = 1.0f - dnoisefac;
				double sonar_vis = tsub->sonar_visibility ( d->get_pos ().xy () );
				// The deeper the submarine dives as harder it is detectable.
				double depth_factor = gm->get_depth_factor ( t->get_pos () );
				double prod = dist_factor * sonar_vis * dnoisefac * depth_factor;

				if ( prod > ( 0.1f + 0.01f * rnd ( 10 ) ) )
					detected = true;
			}
		}
	}

	return detected;
}
