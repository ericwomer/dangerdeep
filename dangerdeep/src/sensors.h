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

#include "vector3.h"
#include "angle.h"

#ifndef _SENSORS_H_
#define _SENSORS_H_

#define MIN_VISIBLE_DISTANCE 0.01f

class active_sensor;
class radar_sensor;
class passive_sonar_sensor;
class active_sonar_sensor;
class hfdf_sensor;
class lookout_sensor;
class sea_object;
class particle;

/** Base class for all sensor types. */
class sensor
{
public:
	enum sensor_move_mode { rotate, sweep };

private:
	/// Range of sensor. Mostly a decline value.
	double range;
	/// Bearing of dectector.
	angle bearing;
	/// Size of detector cone.
	double detection_cone;
	/// This flag shows in what direction the sensor moves.
	/// 1 right, -1 left.
	int move_direction;

protected:
	/**
		This method calculates the decline of the signal strength.
		@parm d distance value in meters
		@return factor of declined signal
	*/
	virtual double get_distance_factor ( double d ) const;
	/**
		A detector can be directed to a specified bearing and has a detection
		cone. A target can only be detected when the target is within this cone.
		This method gets a location vector and uses then known bearing of the
		sensor and its detection angle to verify if the specified target is within
		this cone.
		@param r location vector from detecting to target unit
		@param h heading of the detecting unit
		@return is target within detection cone or not
	*/
	virtual bool is_within_detection_cone ( const vector2& r, const angle& h ) const;

public:
	/**
		Constructor.
		@param range Range of sensor
		@param detectionAngle Size of detection cone of detector
	*/
	sensor ( double range = 0.0f, double detectionCone = 360.0f );
	/// Destructor
	virtual ~sensor () {};

	/**
		Sets the range value.
		@param range new range value
	*/
	virtual void set_range ( double range ) { this->range = range; }
	/**
		Sets the bearing value.
		@param bearing new bearing value
	*/
	virtual void set_bearing ( const angle& bearing) { this->bearing = bearing; }
	/**
		Sets the detection angle value.
		@param detectionAngle new detection angle
	*/
	virtual void set_detection_cone ( double detection_cone )
		{ this->detection_cone = detection_cone; }
	/**
		Returns the range value.
		@return range
	*/
	virtual double get_range () const { return range; }
	/**
		Returns the bearing of the detector.
		@return bearing
	*/
	virtual angle get_bearing () const { return bearing; }
	/**
		Returns the detection angle.
		@return detectionAngle
	*/
	virtual double get_detection_cone () const { return detection_cone; }
	/**
		This method can be used to move the bearing of the detector. Whenever
		this method is called the bearing is shifted about the two third
		of the detection angle.
	*/
	virtual void auto_move_bearing ( sensor_move_mode mode = rotate );
	/**
		This method verifies if the target unit t can be detected by
		detecting unit d.
		@param gm game object. Some parameters are stored here.
		@param d detecting unit
		@param t target unit
	*/
	virtual bool is_detected ( const game* gm, const sea_object* d,
		const sea_object* t ) const = 0;
};

/** Class for lookout. */
class lookout_sensor : public sensor
{
public:
	enum lookout_type { lookout_type_default };

public:
	lookout_sensor ( lookout_type type = lookout_type_default );
	virtual ~lookout_sensor () {};
	/**
		This method verifies if the target unit t can be detected by
		detecting unit d.
		@param gm game object. Some parameters are stored here.
		@param d detecting unit
		@param t target unit
	*/
	virtual bool is_detected ( const game* gm, const sea_object* d, const sea_object* t ) const;
	virtual bool is_detected ( const game* gm, const sea_object* d, const particle* p ) const;
};

class passive_sonar_sensor : public sensor
{
public:
	enum passive_sonar_type { passive_sonar_type_default,	/* fixme: tt_t4 is missing here */
		passive_sonar_type_tt_t5, passive_sonar_type_tt_t11 };

private:
	void init ( passive_sonar_type type );

public:
	passive_sonar_sensor ( passive_sonar_type type = passive_sonar_type_default );
	virtual ~passive_sonar_sensor () {};
	/**
		This method verifies if the target unit t can be detected by
		detecting unit d.
		@param gm game object. Some parameters are stored here.
		@param d detecting unit
		@param t target unit
	*/
	virtual bool is_detected ( const game* gm, const sea_object* d, const sea_object* t ) const;
	/**
		This method verifies if the target unit t can be detected by
		detecting unit d.
		@param sound_level noise level of object t
		@param gm game object. Some parameters are stored here.
		@param d detecting unit
		@param t target unit
	*/
	virtual bool is_detected ( double& sound_level, const game* gm, const sea_object* d, const sea_object* t ) const;
};

/** Base class for all active sensors. */
class active_sensor : public sensor
{
protected:
	/**
		This method calculates the decline of the signal strength. For
		active sensors another function must be used than for passive
		sensors.
		@parm d distance value in meters
		@return factor of declined signal
	*/
	virtual double get_distance_factor ( double d ) const;

public:
	active_sensor ( double range = 0.0f );
	virtual ~active_sensor () {};
	/**
		This method verifies if the target unit t can be detected by
		detecting unit d.
		@param gm game object. Some parameters are stored here.
		@param d detecting unit
		@param t target unit
	*/
	virtual bool is_detected ( const game* gm, const sea_object* d, const sea_object* t ) const = 0;
};

class radar_sensor : public active_sensor
{
public:
	enum radar_type { radar_type_default, 
					  radar_british_type_271, radar_british_type_272, radar_british_type_273, radar_british_type_277, 
					  radar_german_fumo_29, radar_german_fumo_30, radar_german_fumo_61, radar_german_fumo_64,
					  radar_german_fumo_391 };

private:
	void init ( radar_type type );

public:
	radar_sensor ( radar_type type = radar_type_default );
	virtual ~radar_sensor () {};
	/**
		This method verifies if the target unit t can be detected by
		detecting unit d.
		@param gm game object. Some parameters are stored here.
		@param d detecting unit
		@param t target unit
	*/
	virtual bool is_detected ( const game* gm, const sea_object* d, const sea_object* t ) const;
};

class active_sonar_sensor : public active_sensor
{
public:
	enum active_sonar_type { active_sonar_type_default };

private:
	void init ( active_sonar_type type );

public:
	active_sonar_sensor ( active_sonar_type type = active_sonar_type_default );
	virtual ~active_sonar_sensor () {};
	/**
		This method verifies if the target unit t can be detected by
		detecting unit d.
		@param gm game object. Some parameters are stored here.
		@param d detecting unit
		@param t target unit
	*/
	virtual bool is_detected ( const game* gm, const sea_object* d, const sea_object* t ) const;
};

#endif /* _SENSORS_H_ */
