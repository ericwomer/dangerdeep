// Sensors
// subsim (C) + (W). See LICENSE

#include "vector3.h"
#include "angle.h"

#ifndef _SENSORS_H_
#define _SENSORS_H_

#define MIN_VISIBLE_DISTANCE 0.01f

class ActiveSensor;
class RadarSensor;
class PassiveSonarSensor;
class ActiveSonarSensor;
class HVDFSensor;
class LookoutSensor;
class sea_object;

/** Base class for all sensor types. */
class Sensor
{
public:
	enum SensorMoveMode { Rotate, Sweep };

private:
	/// Range of sensor. Mostly a decline value.
	double range;
	/// Bearing of dectector.
	angle bearing;
	/// Size of detector cone.
	double detectionCone;
	/// This flag shows in what direction the sensor moves.
	/// 1 right, -1 left.
	char moveDirection;

protected:
	/**
		This method calculates the decline of the signal strength.
		@parm d distance value in meters
		@return factor of declined signal
	*/
	virtual double getDistanceFactor ( const double& d ) const;
	/**
		Noise modification for submarines. Submarines are using diesel engines
		that are fare less audible than the turbine engines of other ships.
		@param s sea object that noise modification factor is going to be calculated
		@return noise modification factor
	*/
	virtual double doNoiseMod ( const sea_object* s ) const;
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
	virtual bool isWithinDetectionCone ( const vector2& r, const angle& h ) const;

public:
	/**
		Constructor.
		@param range Range of sensor
		@param detectionAngle Size of detection cone of detector
	*/
	Sensor ( const double& range = 0.0f, const double& detectionCone = 360.0f );
	/// Destructor
	virtual ~Sensor () {};

	/**
		Sets the range value.
		@param range new range value
	*/
	virtual void setRange ( const double& range ) { this->range = range; }
	/**
		Sets the bearing value.
		@param bearing new bearing value
	*/
	virtual void setBearing ( const angle& bearing) { this->bearing = bearing; }
	/**
		Sets the detection angle value.
		@param detectionAngle new detection angle
	*/
	virtual void setDetectionCone ( const double& detectionCone )
		{ this->detectionCone = detectionCone; }
	/**
		Returns the range value.
		@return range
	*/
	virtual double getRange () const { return range; }
	/**
		Returns the bearing of the detector.
		@return bearing
	*/
	virtual angle getBearing () const { return bearing; }
	/**
		Returns the detection angle.
		@return detectionAngle
	*/
	virtual double getDetectionCone () const { return detectionCone; }
	/**
		This method can be used to move the bearing of the detector. Whenever
		this method is called the bearing is shifted about the two third
		of the detection angle.
	*/
	virtual void autoMoveBearing ( const SensorMoveMode& mode = Rotate );
	/**
		This method verifies if the target unit t can be detected by
		detecting unit d.
		@param gm game object. Some parameters are stored here.
		@param d detecting unit
		@param t target unit
	*/
	virtual bool isDetected ( const game* gm, const sea_object* d,
		const sea_object* t ) const = 0;

	// Conversion methods.
	virtual ActiveSensor* getActiveSensor () { return 0; }
	virtual const ActiveSensor* getActiveSensor () const { return 0; }
	virtual RadarSensor* getRadarSensor () { return 0; }
	virtual const RadarSensor* getRadarSensor () const { return 0; }
	virtual ActiveSonarSensor* getActiveSonarSensor () { return 0; }
	virtual const ActiveSonarSensor* getActiveSonarSensor () const { return 0; }
	virtual PassiveSonarSensor* getPassiveSonarSensor () { return 0; }
	virtual const PassiveSonarSensor* getPassiveSonarSensor () const { return 0; }
	virtual HVDFSensor* getHVDFSensor () { return 0; }
	virtual const HVDFSensor* getHVDFSensor () const { return 0; }
	virtual LookoutSensor* getLookoutSensor () { return 0; }
	virtual const LookoutSensor* getLookoutSensor () const { return 0; }
};

/** Class for lookout. */
class LookoutSensor : public Sensor
{
public:
	enum LookoutType { Default };

public:
	LookoutSensor ( const int& type = Default );
	virtual ~LookoutSensor () {};
	/**
		This method verifies if the target unit t can be detected by
		detecting unit d.
		@param gm game object. Some parameters are stored here.
		@param d detecting unit
		@param t target unit
	*/
	virtual bool isDetected ( const game* gm, const sea_object* d, const sea_object* t ) const;

	// Conversion method.
	virtual LookoutSensor* getLookoutSensor () { return this; }
	virtual const LookoutSensor* getLookoutSensor () const { return this; }
};

class PassiveSonarSensor : public Sensor
{
public:
	enum PassiveSonarType { Default };

private:
	void init ( const int& type );

public:
	PassiveSonarSensor ( const int& type = Default );
	virtual ~PassiveSonarSensor () {};
	/**
		This method verifies if the target unit t can be detected by
		detecting unit d.
		@param gm game object. Some parameters are stored here.
		@param d detecting unit
		@param t target unit
	*/
	virtual bool isDetected ( const game* gm, const sea_object* d, const sea_object* t ) const;

	// Conversion method.
	virtual PassiveSonarSensor* getPassiveSonarSensor () { return this; }
	virtual const PassiveSonarSensor* getPassiveSonarSensor () const { return this; }
};

/** Base class for all active Sensors. */
class ActiveSensor : public Sensor
{
protected:
	/**
		This method calculates the decline of the signal strength. For
		active sensors another function must be used than for passive
		sensors.
		@parm d distance value in meters
		@return factor of declined signal
	*/
	virtual double getDistanceFactor ( const double& d ) const;

public:
	ActiveSensor ( const double& range = 0.0f );
	virtual ~ActiveSensor () {};
	/**
		This method verifies if the target unit t can be detected by
		detecting unit d.
		@param gm game object. Some parameters are stored here.
		@param d detecting unit
		@param t target unit
	*/
	virtual bool isDetected ( const game* gm, const sea_object* d, const sea_object* t ) const = 0;

	// Conversion methods.
	virtual ActiveSensor* getActiveSensor () { return this; }
	virtual const ActiveSensor* getActiveSensor () const { return this; }
};

class RadarSensor : public ActiveSensor
{
public:
	enum RadarType { Default };

private:
	void init ( const int& type );

public:
	RadarSensor ( const int& type = Default );
	virtual ~RadarSensor () {};
	/**
		This method verifies if the target unit t can be detected by
		detecting unit d.
		@param gm game object. Some parameters are stored here.
		@param d detecting unit
		@param t target unit
	*/
	virtual bool isDetected ( const game* gm, const sea_object* d, const sea_object* t ) const;

	// Conversion method.
	virtual RadarSensor* getRadarSensor () { return this; }
	virtual const RadarSensor* getRadarSensor () const { return this; }
};

class ActiveSonarSensor : public ActiveSensor
{
public:
	enum ActiveSonarType { Default };

private:
	void init ( const int& type );
	/**
		This method calculates a depth depending factor. A deep diving
		submarine is harder to detect with ASDIC than a submarine at
		periscope depth.
		@param sub location vector of submarine
		@return depth factor
	*/
	inline double getDepthFactor ( const vector3& sub ) const;

public:
	ActiveSonarSensor ( const int& type = Default );
	virtual ~ActiveSonarSensor () {};
	/**
		This method verifies if the target unit t can be detected by
		detecting unit d.
		@param gm game object. Some parameters are stored here.
		@param d detecting unit
		@param t target unit
	*/
	virtual bool isDetected ( const game* gm, const sea_object* d, const sea_object* t ) const;

	// Conversion method.
	virtual ActiveSonarSensor* getActiveSonarSensor () { return this; }
	virtual const ActiveSonarSensor* getActiveSonarSensor () const { return this; }
};



// Class SensorFactory
class SensorFactory
{
public:
	enum Nationality { DE, GB, US, JP };
	enum ShipType { CapitalShip, Destroyer, Freighter, Submarine, Torpedo };
	enum SensorSystem { LookoutSystem, RadarSystem, ActiveSonarSystem,
		PassiveSonarSystem, HFDFSystem, LastSystemItem };

private:
	static LookoutSensor* getLookoutSystem ( const int& nation,
		const int& year );
	static RadarSensor* getRadarSystem ( const int& nation,
		const int& year );
	static ActiveSonarSensor* getActiveSonarSystem ( const int& nation,
		const int& year );
	static PassiveSonarSensor* getPassiveSonarSystem ( const int& nation,
		const int& year );
/* fixme
	static HFDFSensor* getHFDFSystem ( const int& nation,
		const int& year );
*/

public:
	static vector<Sensor*> getSensors ( const int& nation,
		const int& shipType, const int& year );
};

#endif /* _SENSORS_H_ */
