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

#define SINK_SPEED 0.5  // m/sec
#define MAXPREVPOS 20

class sensor;

class sea_object
{
public:
	enum alive_status { defunct, dead, sinking, alive };

	// give negative values for fixed speeds, positive values for knots.
	enum throttle_status { reverse=-7, aheadlisten=-6, aheadsonar=-5, aheadslow=-4,
		aheadhalf=-3, aheadfull=-2, aheadflank=-1, stop=0  };

	enum damage_status { nodamage, lightdamage, mediumdamage, heavydamage, wrecked };

	enum rudder_status { rudderfullleft, rudderleft, ruddermid, rudderright,
		rudderfullright };

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

	vector3 position;
	angle heading;	//, pitch, roll; // rotation of object, recomputed every frame for ships, maybe it should get stored
	double speed;		// m/sec
	double max_speed, max_rev_speed;	// m/sec, read from spec file
	int throttle;
	double acceleration;	// read from spec file

	// -1 <= head_chg <= 1
	// if head_chg is != 0 the object is turning with head_chg * turn_rate angles/sec.
	// until heading == head_to or permanently.
	// if head_chg is == 0, nothing happens.
	bool permanent_turn;
	double head_chg;
	int rudder; // rudder state
	angle head_to;
	angle turn_rate;	// in angle/(time*speed) = angle/m, means angle change per forward movement in meters, read from spec file

	double length, width;	// computed from model, indirect read from spec file
	
	// an object is alive until it is sunk or killed.
	// it will sink to some depth then it is killed.
	// it will be dead for exactly one simulation cycle, to give other
	// objects a chance to erase their pointers to this dead object, in the next
	// cycle it will be defunct and erased by its owner.
	alive_status alive_stat;

	list<vector2> previous_positions;

	// Sensor systems, created after data in spec file
	vector<sensor*> sensors;

	sea_object();
	sea_object& operator=(const sea_object& other);
	sea_object(const sea_object& other);

	virtual void change_rudder (int dir);
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
	virtual string get_description(unsigned detail) const { return "UNKNOWN"; }
	virtual string get_specfilename(void) const { return specfilename; }
	virtual string get_modelname(void) const { return modelname; }

	virtual void simulate(class game& gm, double delta_time);
//	virtual bool is_collision(const sea_object* other);
//	virtual bool is_collision(const vector2& pos);
	// the strength is proportional to damage_status, 0-none, 1-light, 2-medium...
	virtual bool damage(const vector3& fromwhere, unsigned strength); // returns true if object was destroyed
	virtual unsigned calc_damage(void) const;	// returns damage in percent (0 means dead)
	virtual void sink(void);
	virtual void kill(void);
	// fixme: this is ugly.
	virtual bool is_defunct(void) const { return alive_stat == defunct; };
	virtual bool is_dead(void) const { return alive_stat == dead; };
	virtual bool is_sinking(void) const { return alive_stat == sinking; };
	virtual bool is_alive(void) const { return alive_stat == alive; };

	// command interface
	virtual void head_to_ang(const angle& a, bool left_or_right);	// true == left
	virtual void rudder_left(void);
	virtual void rudder_right(void);
	virtual void rudder_hard_left(void);
	virtual void rudder_hard_right(void);
	virtual void rudder_midships(void);
	virtual void set_throttle(throttle_status thr);

	virtual void remember_position(void);
	virtual list<vector2> get_previous_positions(void) const { return previous_positions; }

	virtual vector3 get_pos(void) const { return position; };
	virtual double get_depth () const { return -position.z; };
	virtual angle get_heading(void) const { return heading; };
	virtual angle get_head_to(void) const { return permanent_turn ? heading : head_to; };
	virtual angle get_turn_rate(void) const { return turn_rate; };
	virtual double get_length(void) const { return length; };
	virtual double get_width(void) const { return width; };
	virtual double get_speed(void) const { return speed; };
	virtual double get_max_speed(void) const { return max_speed; };
	virtual double get_throttle_speed(void) const;
	virtual int get_rudder (void) const { return rudder; }
	virtual float surface_visibility(const vector2& watcher) const;
	/**
		Noise modification for submarines. Submarines are using diesel engines
		that are fare less audible than the turbine engines of other ships.
		@return noise modification factor
	*/
	virtual double get_noise_factor () const;
	virtual vector2 get_engine_noise_source () const;

	// needed for launching torpedoes
	pair<angle, double> bearing_and_range_to(const sea_object* other) const;
	angle estimate_angle_on_the_bow(angle target_bearing, angle target_heading) const;

	virtual void display(void) const;
	double get_bounding_radius(void) const { return width + length; }	// fixme: could be computed more exact

	virtual sensor* get_sensor ( sensor_system ss );
	virtual const sensor* get_sensor ( sensor_system ss ) const;
};

#endif
