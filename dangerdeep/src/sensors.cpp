// Sensors
// subsim (C) + (W). See LICENSE

#include "sea_object.h"
#include "submarine.h"
#include "sensors.h"
#include "game.h"
#include "vector2.h"
#include "angle.h"

// Class sensor
sensor::sensor ( double range, double detection_cone ) :
	range ( range ), detection_cone ( detection_cone ), bearing ( 0.0f ),
	move_direction ( 1 )
{}

double sensor::get_distance_factor ( double d ) const
{
	double df = range / d;
	df *= df;

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
		angle diff = dir - angle ( r );
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
		// the probabilty of visibility depends on indivial values
		// relative course, distance to and type of watcher.
		// (height of masts, experience etc.), weather fixme
		double vis = t->surface_visibility ( d->get_pos ().xy () );

		if ( dist < max_view_dist * vis )
			detected = true;
		// fixme: add some randomization!
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
	}
}

bool passive_sonar_sensor::is_detected ( const game* gm, const sea_object* d,
	const sea_object* t ) const
{
	bool detected = false;

	// Surfaced submarines detect anything with their passive sonars.
	const submarine* sub = dynamic_cast<const submarine*> ( d );
    if ( sub && !sub->is_submerged () )
    {
		detected = false;
	}
	else
	{
		vector2 r = t->get_pos ().xy () - d->get_pos ().xy ();
		double df = get_distance_factor ( r.length () );

		// The throttle speed is the real noise of the ship.
		// A ship on flank speed is really deaf.
		double dnoisefac = d->get_noise_factor ();
		double tnoisefac = t->get_noise_factor ();

		// The noise modificator for the detecting unit must be
		// subtracted from 1.
		dnoisefac = 1.0f - dnoisefac;

		if (dnoisefac * tnoisefac * df > ( 0.1f + 0.01f * rnd ( 10 ) ) )
			detected = true;
	}

	return detected;
}



// Class active_sensor
active_sensor::active_sensor ( double range ) : sensor ( range )
{}

double active_sensor::get_distance_factor ( double d ) const
{
	double df = get_range () / d;
	df *= df;
	df *= df;

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
	}
}

bool radar_sensor::is_detected ( const game* gm, const sea_object* d,
	const sea_object* t ) const
{
	bool detected = false;
	vector2 r = t->get_pos ().xy () - d->get_pos ().xy ();
	double df = get_distance_factor ( r.length () );
	double vis = 1.0f;

	// Radars use the surface visibility factor.
	vis = t->surface_visibility ( d->get_pos ().xy () );

	if ( df * vis > 0.1f + 0.01f * rnd ( 10 ) )
		detected = true;

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


// Class sensor_factory
lookout_sensor* sensor_factory::get_lookout_system ( int nation, int year )
{
	return new lookout_sensor;
}

radar_sensor* sensor_factory::get_radar_system ( int nation, int year )
{
	return new radar_sensor;
}

active_sonar_sensor* sensor_factory::get_active_sonar_system ( int nation, int year )
{
	return new active_sonar_sensor;
}

passive_sonar_sensor* sensor_factory::get_passive_sonar_system ( int nation, int year )
{
	return new passive_sonar_sensor;
}

/*
hfdf_sensor* sensor_factory::get_hfdf_sensor ( int nation, int year )
{
	return new hfdf_sensor;
}
*/

vector<sensor*> sensor_factory::get_sensors ( int nation, int ship_type, int year )
{
	vector<sensor*> sensors;
	sensors.resize ( last_system_item );

	switch ( ship_type )
	{
		case capital_ship:
			sensors[lookout_system] = get_lookout_system ( nation, year );
			sensors[radar_system] = get_radar_system ( nation, year );
			break;

		case destroyer:
			sensors[lookout_system] = get_lookout_system ( nation, year );
			sensors[radar_system] = get_radar_system ( nation, year );
			sensors[active_sonar_system] = get_active_sonar_system ( nation, year );
			sensors[passive_sonar_system] = get_passive_sonar_system ( nation, year );
			break;

		case freighter:
			sensors[lookout_system] = get_lookout_system ( nation, year );
			sensors[radar_system] = get_radar_system ( nation, year );
			break;

		case submarine:
			sensors[lookout_system] = get_lookout_system ( nation, year );
			sensors[radar_system] = get_radar_system ( nation, year );
			sensors[active_sonar_system] = get_active_sonar_system ( nation, year );
			sensors[passive_sonar_system] = get_passive_sonar_system ( nation, year );
			break;

		case torpedo:
			sensors[passive_sonar_system] = get_passive_sonar_system ( nation, year );
			break;
	}

	return sensors;
}
