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

	class ref {
		friend class sea_object;
		sea_object* myobj;
	public:
		ref() : myobj(0) {}
		ref(sea_object* obj) : myobj(0) { if (obj) obj->register_ref(*this); }
		~ref() { if (myobj) myobj->unregister_ref(*this); }
		ref& operator= (const ref& other) {
			if (this != &other) {
				if (myobj) myobj->unregister_ref(*this);
				if (other.myobj) other.myobj->register_ref(*this);
			}
			return *this;
		}
		ref(const ref& other) : myobj(0) { if (other.myobj) other.myobj->register_ref(*this); }
		sea_object& operator* () const { return *myobj; }
		sea_object* operator-> () const { return myobj; }
		bool is_null(void) const { return myobj == 0; }
	};

	friend class ref;

protected:
	string specfilename;	// filename for specification .xml file, read from spec file

	string modelname;	// filename for model file (also used for modelcache requests), read from spec file

	vector3 position, velocity;
	quaternion orientation, rot_velocity;

	virtual vector3 get_acceleration(void) const = 0;	// drag must be already included!
	virtual quaternion get_rot_acceleration(void) const = 0;// drag must be already included!

	double length, width;	// computed from model, indirect read from spec file
	
	// an object is alive until it is killed.
	// references to it will get deleted when it is killed.
	// any object can set to disfunctional status (defunct) (all references will get deleted).
	alive_status alive_stat;

	// Sensor systems, created after data in spec file
	vector<sensor*> sensors;
	
	string descr_near, descr_medium, descr_far;	// read from spec file

	// reference handling
	list<ref*> references;
	
	void register_ref(ref& myref);
	void unregister_ref(ref& myref);
	void remove_all_references(void);

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
	virtual double get_depth() const { return -position.z; };
	virtual double get_length(void) const { return length; };
	virtual double get_width(void) const { return width; };
	virtual float surface_visibility(const vector2& watcher) const;

	/**
		Noise modification for submarines. Submarines are using diesel engines
		that are fare less audible than the turbine engines of other ships.
		@return noise modification factor
	*/
	virtual double get_noise_factor () const;
	virtual vector2 get_engine_noise_source () const;

	virtual void display(void) const;
	double get_bounding_radius(void) const { return width + length; }	// fixme: could be computed more exact

	virtual sensor* get_sensor ( sensor_system ss );
	virtual const sensor* get_sensor ( sensor_system ss ) const;
};

#endif
