// sea objects
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef SEA_OBJECT_H
#define SEA_OBJECT_H

#include "vector3.h"
#include "angle.h"
#include "parser.h"
#include <list>
using namespace std;

#define SINK_SPEED 0.5  // m/sec
#define MAXPREVPOS 20

#define CROSS_SECTION_VIS_CONVOY   2.0f         // Convoy
#define CROSS_SECTION_VIS_CV       1.0f         // Aircraft carrier (the largest ships)
#define CROSS_SECTION_VIS_BB_BC    0.95f        // Battleships / Battlecruiser
#define CROSS_SECTION_VIS_HCR_LFR  0.90f        // Heavy Cruiser, Large Freighters
#define CROSS_SECTION_VIS_LCR      0.85f        // Light Cruiser
#define CROSS_SECTION_VIS_NFR      0.80f        // Normal Freighter
#define CROSS_SECTION_VIS_DD_SMFR  0.75f        // Destroyer, Small Freighter
#define CROSS_SECTION_VIS_DE       0.70f        // Destroyer Escorts
#define CROSS_SECTION_VIS_LSUB     0.50f        // Large Submarines (?)
#define CROSS_SECTION_VIS_NSUB     0.40f        // Normal Submarines (?)
#define CROSS_SECTION_VIS_MSUB_AC  0.20f        // Midget Submarines (?), Aircrafts
#define CROSS_SECTION_VIS_PERIS    0.10f        // Periscope
#define CROSS_SECTION_VIS_TORPWB   0.05f        // Torpedo w/ bubble tail (?)
#define CROSS_SECTION_VIS_NULL     1.0f         // Gun shell, torpedos w/o bubble tail

class ship;
class submarine;
class gun_shell;
class convoy;
class airplane;
class torpedo;
class depth_charge;
class Sensor;

class sea_object
{
public:
	enum alive_status { defunct, dead, sinking, alive };
	enum throttle_status { reverse, stop, aheadlisten, aheadsonar, aheadslow,
		aheadhalf, aheadfull, aheadflank };
	enum damage_status { nodamage, lightdamage, mediumdamage, heavydamage, wrecked };
	enum rudder_status { rudderfullleft, rudderleft, ruddermid, rudderright,
		rudderfullright };

	// some useful functions needed for sea_objects

	inline static double kts2ms(const double& knots) { return knots*1852.0f/3600.0f; }
	inline static double ms2kts(const double& meters) { return meters*3600.0f/1852.0f; }
	inline static double kmh2ms(const double& kmh) { return kmh/3.6f; }
	inline static double ms2kmh(const double& meters) { return meters*3.6f; }

protected:
	vector3 position;
	angle heading;
	double speed, max_speed, max_rev_speed;	// m/sec
	throttle_status throttle;
	double acceleration;

	// -1 <= head_chg <= 1
	// if head_chg is != 0 the object is turning with head_chg * turn_rate angles/sec.
	// until heading == head_to or permanently.
	// if head_chg is == 0, nothing happens.
	bool permanent_turn;
	double head_chg;
	int rudder; // rudder state
	angle head_to;
	angle turn_rate;	// in angle/(time*speed) = angle/m
	// this means angle change per forward
	// movement in meters
	double length, width;	// computed from model
	/** Cross section factor needed for detection purposes.
		<pre>
		1.0f         Aircraft carrier (the largest ships)
		0.95         Battleships / Battlecruiser
		0.90f        Heavy Cruiser, Large Freighters
		0.85f        Light Cruiser
		0.80f        Normal Freighter
		0.70f        Destroyer, Small Freighter
		0.60f        Destroyer Escorts
		0.20f        Large Submarines (?)
		0.15f        Normal Submarines (?)
		0.05f        Midget Submarines (?), Aircrafts
		0.01f        Torpedo w/ bubble tail (?)
		0.0f         Gun shell, torpedos w/o bubble tail
		</pre>
	*/
	float vis_cross_section_factor; 

	// an object is alive until it is sunk or killed.
	// it will sink to some depth then it is killed.
	// it will be dead for exactly one simulation cycle, to give other
	// objects a chance to erase their pointers to this dead object, in the next
	// cycle it will be defunct and erased by its owner.
	alive_status alive_stat;

	list<vector2> previous_positions;

	// Sensor systems.
	vector<Sensor*> sensors;

	bool parse_attribute(parser& p);        // returns false if invalid token found

	sea_object();
	sea_object& operator=(const sea_object& other);
	sea_object(const sea_object& other);

	virtual void change_rudder (const int& dir);
	virtual void set_sensors ( vector<Sensor*> sensors );
	/**
		This method calculates the visible profile value of the target.
		@param d location vector of the detecting object
		@return profile factor. This is normally a value between 0.3 and 1.0
	*/
	virtual double getProfileFactor ( const vector2& d ) const;
   
public:
	virtual ~sea_object();

	// detail: 0 - category, 1 - finer category, >=2 - exact category
	virtual string get_description(unsigned detail) const { return "UNKNOWN"; }

	virtual void simulate(class game& gm, double delta_time);
	virtual bool is_collision(const sea_object* other);
	virtual bool is_collision(const vector2& pos);
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

	virtual void set_course_to_pos(const vector2& pos);

	virtual void remember_position(void);
	virtual list<vector2> get_previous_positions(void) const { return previous_positions; }

	virtual vector3 get_pos(void) const { return position; };
	virtual double get_depth () const { return -position.z; };
	virtual angle get_heading(void) const { return heading; };
	virtual angle get_turn_rate(void) const { return turn_rate; };
	virtual double get_length(void) const { return length; };
	virtual double get_width(void) const { return width; };
	virtual double get_speed(void) const { return speed; };
	virtual double get_max_speed(void) const { return max_speed; };
	virtual double get_throttle_speed(void) const;
	virtual int get_rudder (void) const { return rudder; }
	virtual float get_vis_cross_section_factor (void) const { return vis_cross_section_factor; }
	virtual void set_vis_cross_section_factor ( const float& factor ) { vis_cross_section_factor = factor; }
	virtual float surface_visibility(const vector2& watcher) const;

	// needed for launching torpedoes
	pair<angle, double> bearing_and_range_to(const sea_object* other) const;
	angle estimate_angle_on_the_bow(angle target_bearing, angle target_heading) const;

	virtual void display(void) const = 0;
	double get_bounding_radius(void) const { return width + length; }	// fixme: could be computed more exact

	virtual Sensor* get_sensor ( const int& s );
	virtual const Sensor* get_sensor ( const int& s ) const;

	// Convert function.
	virtual ship* get_ship_ptr () { return 0; }
	virtual const ship* get_ship_ptr () const { return 0; }
	virtual submarine* get_submarine_ptr () { return 0; }
	virtual const submarine* get_submarine_ptr () const { return 0; }
	virtual gun_shell* get_gun_hell_ptr () { return 0; }
	virtual const gun_shell* get_gun_hell_ptr () const { return 0; }
	virtual convoy* get_convoy_ptr () { return 0; }
	virtual const convoy* get_convoy_ptr () const { return 0; }
	virtual airplane* get_airplane_ptr () { return 0; }
	virtual const airplane* get_airplane_ptr () const { return 0; }
	virtual torpedo* get_torpedo_ptr () { return 0; }
	virtual const torpedo* get_torpedo_ptr () const { return 0; }
	virtual depth_charge* get_depth_charge_ptr () { return 0; }
	virtual const depth_charge* get_depth_charge_ptr () const { return 0; }
};

#endif
