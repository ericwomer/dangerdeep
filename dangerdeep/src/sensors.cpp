// Sensors
// subsim (C) + (W). See LICENSE

#include "sea_object.h"
#include "submarine.h"
#include "sensors.h"
#include "game.h"
#include "vector2.h"
#include "angle.h"

// Class Sensor
Sensor::Sensor ( const double& range, const double& detectionAngle ) :
	range ( range ), detectionCone ( detectionCone ), bearing ( 0.0f ),
	moveDirection ( 1 )
{}

double Sensor::getDistanceFactor ( const double& d ) const
{
	double df = range / d;
	df *= df;

	return df;
}

double Sensor::doNoiseMod ( const sea_object* s ) const
{
	const submarine* sub = s->get_submarine_ptr ();
	double noisefac = 1.0;

	if ( sub )
	{
		if ( sub->is_electric_engine () )
			noisefac = 0.007f;
		else
			noisefac = 0.1f;
	}

	return noisefac;
}

bool Sensor::isWithinDetectionCone ( const vector2& r, const angle& h ) const
{
	bool withinAngle = false;

	if ( detectionCone >= 360.0f )
	{
		// When the detection angle is larger equal 360 degrees
		// the target is everytime within this detection angle.
		withinAngle = true;
	}
	else
	{
		angle dir = bearing + h;
		angle diff = dir - angle ( r );
		double deltaAngle = diff.value_pm180 ();

		if ( deltaAngle >= -detectionCone && deltaAngle <= detectionCone )
			withinAngle = true;
	}

	return withinAngle;
}

void Sensor::autoMoveBearing ( const SensorMoveMode& mode )
{
	if ( detectionCone < 360.0f )
		bearing += moveDirection * detectionCone * 0.666666f;

	if ( mode == Sweep )
	{
		double b = bearing.value ();
		if ( b < 180.0f && b > 90.0f && moveDirection > 0 )
			moveDirection = -1;
		else if ( b > 180.0f && b < 270.0f && moveDirection < 0 )
			moveDirection = 1;
	}
}



// Class LookoutSensor
LookoutSensor::LookoutSensor ( const int& type ) : Sensor ()
{}


bool LookoutSensor::isDetected ( const game* gm, const sea_object* d,
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



// Class PassiveSonarSensor
PassiveSonarSensor::PassiveSonarSensor ( const int& type ) : Sensor ()
{
	init ( type );
}

void PassiveSonarSensor::init ( const int& type )
{
	switch ( type )
	{
		case Default:
			setRange ( 9500.0f );
			break;
	}
}

bool PassiveSonarSensor::isDetected ( const game* gm, const sea_object* d,
	const sea_object* t ) const
{
	bool detected = false;

	// Surfaced submarines detect anything with their passive sonars.
	const submarine* sub = d->get_submarine_ptr ();
    if ( sub && !sub->is_submerged () )
    {
		detected = false;
	}
	else
	{
		vector2 r = t->get_pos ().xy () - d->get_pos ().xy ();
		double df = getDistanceFactor ( r.length () );

		// The throttle speed is the real noise of the ship.
		// A ship on flank speed is really deaf.
		double dnoisefac = d->get_throttle_speed () / d->get_max_speed ();
		double tnoisefac = t->get_throttle_speed () / t->get_max_speed ();

		// Submarines are using a diesel engine and have a much smaller
		// passive detection range.
		dnoisefac *= doNoiseMod ( d );
		tnoisefac *= doNoiseMod ( t );

		// The noise modificator for the detecting unit must be
		// subtracted from 1.
		dnoisefac = 1.0f - dnoisefac;

		if (dnoisefac * tnoisefac * df > ( 0.1f + 0.01f * rnd ( 10 ) ) )
			detected = true;
	}

	return detected;
}



// Class ActiveSensor
ActiveSensor::ActiveSensor ( const double& range ) : Sensor ( range )
{}

double ActiveSensor::getDistanceFactor ( const double& d ) const
{
	double df = getRange () / d;
	df *= df;
	df *= df;

	return df;
}



// Class RadarSensor
RadarSensor::RadarSensor ( const int& type ) : ActiveSensor ()
{
	init ( type );
}

void RadarSensor::init ( const int& type )
{
	switch ( type )
	{
		case Default:
			setRange  ( 0.0f );
			break;
	}
}

bool RadarSensor::isDetected ( const game* gm, const sea_object* d,
	const sea_object* t ) const
{
	bool detected = false;
	vector2 r = t->get_pos ().xy () - d->get_pos ().xy ();
	double df = getDistanceFactor ( r.length () );
	double vis = 1.0f;

	// Radars use the surface visibility factor.
	vis = t->surface_visibility ( d->get_pos ().xy () );

	if ( df * vis > 0.1f + 0.01f * rnd ( 10 ) )
		detected = true;

	return detected;
}


// Class ActiveSonarSensor
ActiveSonarSensor::ActiveSonarSensor ( const int& type ) : ActiveSensor ()
{
	init ( type );
}

void ActiveSonarSensor::init ( const int& type )
{
	switch ( type )
	{
		case Default:
			setRange ( 1500.0f );
			setDetectionCone ( 15.0f );
			break;
	}
}

inline double ActiveSonarSensor::getDepthFactor ( const vector3& sub ) const
{
	return ( 1.0f - 0.5f * sub.z / 400.0f );
}

bool ActiveSonarSensor::isDetected ( const game* gm, const sea_object* d,
	const sea_object* t ) const
{
	bool detected = false;

	// Surfaced submarines cannot use ASDIC.
	const submarine* dsub = d->get_submarine_ptr ();
    if ( dsub && !dsub->is_submerged () )
	{
		detected = false;
	}
	else
	{
		// Only submerged submarines can be detected with ASDIC.
		const submarine* tsub = t->get_submarine_ptr ();
		if ( tsub && tsub->is_submerged () )
		{
			vector2 r = t->get_pos ().xy () - d->get_pos ().xy ();

			// Verify if target is within detection cone of the sensor.
			if ( isWithinDetectionCone ( r, d->get_heading () ) )
			{
				double distFactor = getDistanceFactor ( r.length () );
				// The throttle speed is the real noise of the ship.
				// A ship on flank speed is really deaf.
				double dnoisefac = d->get_throttle_speed () / d->get_max_speed ();
				// Submarines are quieter than ship with turbines and have
				// a better chance to detect a ship with active sonar.
				dnoisefac *= doNoiseMod ( d );
				// The noise modificator for the detecting unit must be
				// subtracted from 1.
				dnoisefac = 1.0f - dnoisefac;
				double sonarVis = tsub->sonar_visibility ( d->get_pos ().xy () );
				// The deeper the submarine dives as harder it is detectable.
				double depthFactor = getDepthFactor ( t->get_pos () );
				double prod = distFactor * sonarVis * dnoisefac * depthFactor;

				if ( prod > ( 0.1f + 0.01f * rnd ( 10 ) ) )
					detected = true;
			}
		}
	}

	return detected;
}


// Class SensorFactory
LookoutSensor* SensorFactory::getLookoutSystem ( const int& nation, const int& year )
{
	return new LookoutSensor;
}

RadarSensor* SensorFactory::getRadarSystem ( const int& nation, const int& year )
{
	return new RadarSensor;
}

ActiveSonarSensor* SensorFactory::getActiveSonarSystem ( const int& nation, const int& year )
{
	return new ActiveSonarSensor;
}

PassiveSonarSensor* SensorFactory::getPassiveSonarSystem ( const int& nation, const int& year )
{
	return new PassiveSonarSensor;
}

/*
HFDFSensor* SensorFactory::getHFDFSensor ( const int& nation, const int& year )
{
	return new HFDFSensor;
}
*/

vector<Sensor*> SensorFactory::getSensors ( const int& nation,
	const int& shipType, const int& year )
{
	vector<Sensor*> sensors;
	sensors.resize ( LastSystemItem );

	switch ( shipType )
	{
		case CapitalShip:
			sensors[LookoutSystem] = getLookoutSystem ( nation, year );
			sensors[RadarSystem] = getRadarSystem ( nation, year );
			break;

		case Destroyer:
			sensors[LookoutSystem] = getLookoutSystem ( nation, year );
			sensors[RadarSystem] = getRadarSystem ( nation, year );
			sensors[ActiveSonarSystem] = getActiveSonarSystem ( nation, year );
			sensors[PassiveSonarSystem] = getPassiveSonarSystem ( nation, year );
			break;

		case Freighter:
			sensors[LookoutSystem] = getLookoutSystem ( nation, year );
			sensors[RadarSystem] = getRadarSystem ( nation, year );
			break;

		case Submarine:
			sensors[LookoutSystem] = getLookoutSystem ( nation, year );
			sensors[RadarSystem] = getRadarSystem ( nation, year );
			sensors[ActiveSonarSystem] = getActiveSonarSystem ( nation, year );
			sensors[PassiveSonarSystem] = getPassiveSonarSystem ( nation, year );
			break;

		case Torpedo:
			sensors[PassiveSonarSystem] = getPassiveSonarSystem ( nation, year );
			break;
	}

	return sensors;
}
