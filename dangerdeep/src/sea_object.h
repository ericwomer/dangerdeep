// sea objects
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef SEA_OBJECT_H
#define SEA_OBJECT_H

#include <list>
#include <string>
#include <vector>
using namespace std;

#include "vector3.h"
#include "angle.h"
#include "quaternion.h"

/*
fixme: global todo (2004/06/26):
-> move much code from sea_object to ship.
-> maybe remove silly reference counting.
-> split AI in several children
-> maybe introduce C++ exceptions.
-> fix load/save for sea_object and heirs
-> fix simulate/acceleration code for all sea_objects and heirs.
-> replace silly head_chg code by real rudder position simulation code
*/


class sensor;

class sea_object
{
public:
	enum alive_status { defunct, dead, alive };

	//fixme: should move to damageable_part class ...
	enum damage_status { nodamage, lightdamage, mediumdamage, heavydamage, wrecked };

	//fixme: this should move to sensor.h!!!
	enum sensor_system { lookout_system, radar_system, passive_sonar_system,
		active_sonar_system, last_sensor_system };

	// some useful functions needed for sea_objects
	inline static double kts2ms(double knots) { return knots*1852.0/3600.0; }
	inline static double ms2kts(double meters) { return meters*3600.0/1852.0; }
	inline static double kmh2ms(double kmh) { return kmh/3.6; }
	inline static double ms2kmh(double meters) { return meters*3.6; }

	// translate coordinates from degrees to meters and vice versa
	static void degrees2meters(bool west, unsigned degx, unsigned minx, bool south,
		unsigned degy, unsigned miny, double& x, double& y);
	static void meters2degrees(double x, double y, bool& west, unsigned& degx, unsigned& minx, bool& south,
		unsigned& degy, unsigned& miny);

protected:
	string specfilename;	// filename for specification .xml file, read from spec file

	string modelname;	// filename for model file (also used for modelcache requests), read from spec file

	vector3 position;	// global position
	vector3 velocity;	// local velocity
	quaternion orientation;	// global orientation
	double turn_velocity;	// angular velocity around global z-axis
	angle heading;		// global z-orientation is stored additionally
	
	virtual vector3 get_acceleration(void) const;		// drag must be already included!
	virtual double get_turn_acceleration(void) const;	// drag must be already included!

	vector3f size3d;		// computed from model, indirect read from spec file, width, length, height
	
	// an object is alive until it is killed.
	// any object can set to disfunctional status (defunct).
	alive_status alive_stat;

	// Sensor systems, created after data in spec file
	vector<sensor*> sensors;
	
	string descr_near, descr_medium, descr_far;	// read from spec file

	class ai* myai;

	sea_object();
	sea_object& operator=(const sea_object& other);
	sea_object(const sea_object& other);

	virtual void set_sensor ( sensor_system ss, sensor* s );

	/**
		This method calculates the visible cross section of the target.
		@param d location vector of the detecting object
		@return cross section in square meters.
	*/
	virtual double get_cross_section ( const vector2& d ) const;

	// give a loaded xml document to this c'tor, it will create an object after the specs
	sea_object(class TiXmlDocument* specfile, const char* topnodename = "dftd-object");
	
public:
	virtual ~sea_object();
	virtual void load(istream& in, class game& g);
	virtual void save(ostream& out, const class game& g) const;

	// call with ship/submarine/etc node from mission file
	virtual void parse_attributes(class TiXmlElement* parent);

	// detail: 0 - category, 1 - finer category, >=2 - exact category
	virtual string get_description(unsigned detail) const;
	virtual string get_specfilename(void) const { return specfilename; }
	virtual string get_modelname(void) const { return modelname; }

	virtual void simulate(class game& gm, double delta_time);
//	virtual bool is_collision(const sea_object* other);
//	virtual bool is_collision(const vector2& pos);
	// the strength is proportional to damage_status, 0-none, 1-light, 2-medium...
	virtual bool damage(const vector3& fromwhere, unsigned strength); // returns true if object was destroyed
	virtual unsigned calc_damage(void) const;	// returns damage in percent (0 means dead)
	virtual void kill(void);
	virtual void destroy(void);
	virtual bool is_defunct(void) const { return alive_stat == defunct; };
	virtual bool is_dead(void) const { return alive_stat == dead; };
	virtual bool is_alive(void) const { return alive_stat == alive; };

	// command interface - no special commands for a generic sea_object

	virtual vector3 get_pos(void) const { return position; };
	virtual vector3 get_velocity(void) const { return velocity; };
	virtual double get_speed(void) const { return get_velocity().y; }
	virtual quaternion get_orientation(void) const { return orientation; };
	virtual double get_turn_velocity(void) const { return turn_velocity; };
	virtual double get_depth() const { return -position.z; };
	virtual float get_width(void) const { return size3d.x; };
	virtual float get_length(void) const { return size3d.y; };
	virtual float get_height(void) const { return size3d.z; };
	virtual float surface_visibility(const vector2& watcher) const;
	virtual angle get_heading(void) const { return heading; }
	virtual class ai* get_ai(void) { return myai; }

	/**
		Noise modification for submarines. Submarines are using diesel engines
		that are fare less audible than the turbine engines of other ships.
		@return noise modification factor
	*/
	virtual double get_noise_factor () const { return 0; }
	virtual vector2 get_engine_noise_source () const;

	virtual void display(void) const;
	double get_bounding_radius(void) const { return size3d.x+size3d.y; }	// fixme: could be computed more exact

	virtual sensor* get_sensor ( sensor_system ss );
	virtual const sensor* get_sensor ( sensor_system ss ) const;
};

#endif
