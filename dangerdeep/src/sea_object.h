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

// sea objects
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef SEA_OBJECT_H
#define SEA_OBJECT_H

#include <string>
#include "ptrvector.h"
#include <new>

#include "vector3.h"
#include "angle.h"
#include "quaternion.h"
#include "xml.h"
#include "ai.h"
#include "countrycodes.h"
#include "sensors.h"
#include "sonar.h"

/*
fixme: global todo (2004/06/26):
-> move much code from sea_object to ship. -> PARTLY DONE
-> maybe remove silly reference counting. -> DONE?
-> split AI in several children
-> maybe introduce C++ exceptions. -> PARTLY DONE
-> fix load/save for sea_object and heirs -> MOSTLY DONE
-> fix simulate/acceleration code for all sea_objects and heirs. -> DONE
-> replace silly head_chg code by real rudder position simulation code -> PARTLY DONE
*/

class game;
class sensor;

///\brief Base class for all physical objects in the game world. Simulates dynamics with position, velocity, acceleration etc.
class sea_object
{
public:
	// inactive means burning, sinking etc.
	// the two states are used for object handling.
	// when an object should be removed from the game, it is set to dead-state by kill()
	// the simulate function checks for that case and sets an object to defunct by destroy() if its dead
	// defunct objects are removed from storage (deleted).
	// this technique guarantees that dead objects exists at least one round, so other objects can clear
	// their pointers to this object avoiding a segfault.
	enum alive_status { defunct, dead, inactive, alive };

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

	// we need a struct for each part:
	// VARIABLE:
	// damage status
	// remaining repair time
	// INVARIABLE: (maybe dependent on sub type)
	// position inside sub
	// relative weakness (how sensitive is part to shock waves)
	// must be surfaced to get repaired
	// cannot be repaired at sea	
	// absolute time needed for repair
	// new: damage levels (some parts can only be ok/wrecked or ok/damaged/wrecked)
	// new: damagle from which direction/protected by etc.
	//      parts that get damaged absorb shock waves, protecing other parts
	//      either this must be calculated or faked via direction indicators etc.
	
	// addition damage for ships: armor, i.e. resistance against shells.
	// two shell types AP and HE (armor piercing and high explosive)
	// we would have only HE shells in Dftd, because we have no battleship simulator...
	// although PD could be also needed...
	
	struct damage_data_scheme {
		vector3f p1, p2;	// corners of bounding box around part, p1 < p2
					// coordinates in 0...1 relative to left,bottom,aft
					// corner of sub's bounding box.
		float weakness;		// weakness to shock waves
		unsigned repairtime;	// seconds
		bool surfaced;		// must sub be surfaced to repair this?
		bool repairable;	// is repairable at sea?
		damage_data_scheme(const vector3f& a, const vector3f& b, float w, unsigned t, bool s = false, bool r = true) :
			p1(a), p2(b), weakness(w), repairtime(t), surfaced(s), repairable(r) {}
	};
	
	std::vector<damage_data_scheme> damage_schemes;

	// each sea_object has some damageable parts.
	struct part {
		std::string id;		// id of part
		vector3f p1, p2;	// corners of bounding box around part, p1 < p2
					// coordinates in absolute values (meters)
		float strength;		// weakness to shock waves (1.0 = normal, 0.1 very weak), damage factor
		double status;		// damage in percent, negative means part is not existent.
		unsigned repairtime;	// seconds
		bool surfaced;		// must sub be surfaced to repair this?
		bool repairable;	// is repairable at sea?
		bool floodable;		// does part leak when damaged?
		// variable data
		float damage;		// 0-1, 1 wrecked, 0 ok
		double remainingtime;	// time until repair is completed
		float floodlevel;	// how much water is inside (0-1, part of volume, 1 = full)
		// METHODS
		part(double st = -1, unsigned int rt = 0) : status(st), repairtime(rt) {}
		//fixme: save to xml!
		//part(istream& in) { status = read_double(in); repairtime = read_double(in); }
		//void save(ostream& out) const { write_double(out, status); write_double(out, repairtime); }
	};

private:
	sea_object();
	sea_object& operator=(const sea_object& other);
	sea_object(const sea_object& other);

protected:
	// game exists before and after live of each sea_object, but calls to game are very common.
	// so store a ref to game here.	
	game& gm;

	// filename for specification .xml file, set in constructor
	std::string specfilename;

	// filename for model file (also used for modelcache requests), read from spec file
	std::string modelname;
	class model* mymodel;	// pointer to model object, store as quick lookup

	// model variants (layout / skin), read from spec file
	struct skin_variant {
		std::string name;
		std::list<std::string> regions;
		std::list<std::string> countries;
		date from, until;
	};
	std::list<skin_variant> skin_variants;

	// skin selection data of this object [SAVE]
	std::string skin_regioncode;
	countrycode skin_country;
	date skin_date;
	std::string skin_name;	// name of skin, computed from values above

	// computes name of skin variant name according to data above.
	std::string compute_skin_name() const;

	vector3 position;	// global position, [SAVE]
	vector3 velocity;	// local velocity, [SAVE]
	// maybe: vector3 impulse;
	quaternion orientation;	// global orientation, [SAVE]
	double turn_velocity;	// angular velocity around global z-axis (mathematical CCW), [SAVE]
	// later:
	// quaternion turn_impulse; or turn_velocity;
	angle heading;		// global z-orientation is stored additionally, [SAVE]
	// maybe (re)compute heading by orientation: let (0,1,0) rotate by orientation,
	// project to xy-plane, normalize and measure angle.
	// maybe compute and store vector2 heading_dir also, to save computations.

	vector3 global_velocity;// recomputed every frame by simulate() method
	
	virtual vector3 get_acceleration() const;	// drag must be already included!
	virtual double get_turn_acceleration() const;	// drag must be already included!
	// later:
	// virtual vector3 get_force() const;		// vector of force
	// virtual quaternion get_torque() const;	// angle/axis of torque

	vector3f size3d;		// computed from model, indirect read from spec file, width, length, height

	// double mass;	// total weight, later read from spec file

	/// Activity state of an object.	
	/// an object is alive until it is killed or inactive.
	/// killed (dead) objects exists at least one simulation step. All other objects must remove their
	/// pointers to an object, if it is dead.
	/// The next step it is set to disfunctional status (defunct) and removed the next step.
	alive_status alive_stat;	// [SAVE]

	/// Sensor systems, created after data in spec file
	ptrvector<sensor> sensors;
	
	// fixme: this is per model/type only. it is a waste to store it for every object
	std::string descr_near, descr_medium, descr_far;	// read from spec file

	std::auto_ptr<ai> myai;	// created from spec file, but data needs to be saved, [SAVE]

	/// pointer to target or similar object.
	/// used by airplanes/ships/submarines to store a reference to their target.
	/// automatically set to NULL by simulate() if target is inactive.
	sea_object* target;	// [SAVE]

	/// flag to toggle invulnerability. Note that this is only set or used for the editor
	/// or debugging purposes!
	bool invulnerable;

	/// Country code of object. Used for records or for AI to determine enemies. Or for camouflage
	/// schemes.
	countrycode country;

	/// Party (Axis/Allies/Neutral). Normally determined from country code. But countries changed
	/// party (Italy 1943, France 1940).
	partycode party;

	/// Detection time counter (counts down). When it reaches zero, detection of other objects is triggered.
	double redetect_time;
	/// list of visible objects, recreated regularly
	std::vector<sea_object*> visible_objects;
	/// list of radar detected objects, recreated regularly  , fixme: use some contact type here as well
	std::vector<sea_object*> radar_objects;
	/// list of heared/sonar detected objects, recreated regularly
	std::vector<sonar_contact> sonar_objects;

	virtual void set_sensor ( sensor_system ss, sensor* s );

	/**
		This method calculates the visible cross section of the target.
		@param d location vector of the detecting object
		@return cross section in square meters.
	*/
	virtual double get_cross_section ( const vector2& d ) const;

	// construct sea_object without spec file (for simple objects like DCs, shells, ...)
	// these models have no skin support, because there is no spec file.
	sea_object(game& gm_, const std::string& modelname_);

	// construct a sea_object. called by heirs
	sea_object(game& gm_, const xml_elem& parent);

	// wether objects of that class should call visible_sea_objects to detect other
	// objects. redefine if needed.
	virtual bool detect_other_sea_objects() const { return false; }

public:
	virtual ~sea_object();

	virtual void load(const xml_elem& parent);
	virtual void save(xml_elem& parent) const;

	// detail: 0 - category, 1 - finer category, >=2 - exact category
	virtual std::string get_description(unsigned detail) const;
	virtual std::string get_specfilename() const { return specfilename; }
	virtual std::string get_modelname() const { return modelname; }

	virtual void simulate(double delta_time);
//	virtual bool is_collision(const sea_object* other);
//	virtual bool is_collision(const vector2& pos);

	// damage an object. give a relative position (in meters) where the damage is caused,
	// the strength (and type!) of damage. (types: impact (piercing), explosion (destruction), shock wave)
	// the strength is proportional to damage_status, 0-none, 1-light, 2-medium...
	virtual bool damage(const vector3& fromwhere, unsigned strength); // returns true if object was destroyed

	virtual void set_target(sea_object* s) { if (s && s->is_alive()) target = s; }

	virtual unsigned calc_damage() const;	// returns damage in percent (100 means dead)
	virtual void set_inactive();
	virtual void kill();
	virtual void destroy();
	virtual bool is_defunct() const { return alive_stat == defunct; };
	virtual bool is_dead() const { return alive_stat == dead; };
	virtual bool is_inactive() const { return alive_stat == inactive; };
	virtual bool is_alive() const { return alive_stat == alive; };

	// command interface - no special commands for a generic sea_object

	virtual vector3 get_pos() const { return position; };
	virtual vector3 get_velocity() const { return velocity; };
	virtual double get_speed() const { return get_velocity().y; }
	virtual quaternion get_orientation() const { return orientation; };
	virtual double get_turn_velocity() const { return turn_velocity; };
	virtual double get_depth() const { return -position.z; };
	virtual float get_width() const { return size3d.x; };
	virtual float get_length() const { return size3d.y; };
	virtual float get_height() const { return size3d.z; };
	virtual float surface_visibility(const vector2& watcher) const;
	virtual angle get_heading() const { return heading; }
	virtual class ai* get_ai() { return myai.get(); }
	virtual const sea_object* get_target() const { return target; }
	bool is_invulnerable() const { return invulnerable; }
	countrycode get_country() const { return country; }
	partycode get_party() const { return party; }

	/* NOTE! the following function(s) are only to set up games!
	   They are used only in the editor or by class convoy while creating
	   custom convoy missions.
	   Do not call them from any other place!
	*/
	virtual void manipulate_position(const vector3& newpos);
	virtual void manipulate_speed(double localforwardspeed);
	virtual void manipulate_heading(angle hdg);
	virtual void manipulate_invulnerability(bool invul) { invulnerable = invul; }

	/**
		Noise modification for submarines. Submarines are using diesel engines
		that are fare less audible than the turbine engines of other ships.
		@return noise modification factor
	*/
	virtual double get_noise_factor () const { return 0; }
	virtual vector2 get_engine_noise_source () const;

	virtual void display() const;
	virtual void display_mirror_clip() const;
	double get_bounding_radius() const { return size3d.x+size3d.y; }	// fixme: could be computed more exact
	virtual void set_skin_layout(const std::string& layout);

	virtual sensor* get_sensor ( sensor_system ss );
	virtual const sensor* get_sensor ( sensor_system ss ) const;

	virtual const std::vector<sea_object*>& get_visible_objects() const { return visible_objects; }
	virtual const std::vector<sea_object*>& get_radar_objects() const { return radar_objects; }
	virtual const std::vector<sonar_contact>& get_sonar_objects() const { return sonar_objects; }

	// check for a vector of pointers if the objects are still alive
	// and remove entries of dead objects (do not delete the objects itself!)
	// and compress the vector afterwars.
	static void compress(std::vector<sea_object*>& vec);
	static void compress(std::list<sea_object*>& lst);
};

#endif
