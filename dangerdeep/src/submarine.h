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

// submarines
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef SUBMARINE_H
#define SUBMARINE_H

#include "ship.h"
#include "tdc.h"
#include <vector>

//fixme: this is very ugly. replace this asap.
#define SUBMARINE_SUBMERGED_DEPTH 2.0f // meters

class game;
class torpedo;

class submarine : public ship
{
 private:
	submarine();
	submarine& operator= (const submarine& other);
	submarine(const submarine& other);

 public:

 	//fixme: later we have to add control data for each torpedo (gyro angle, fat/lut setup for each
 	//torpedo etc. we store pointers to class torpedo here instead of "type" to accomplish this)
	struct stored_torpedo {
		enum st_status { st_empty, st_reloading, st_unloading, st_loaded };
		// be careful with that...
		torpedo* torp;
		st_status status;	// 0 empty 1 reloading 2 unloading 3 loaded
		unsigned associated;	// reloading from/unloading to
		double remaining_time;	// remaining time until work is finished
		angle addleadangle;	// additional lead angle (only per tube) fixme: replace by lead angle reported from TDC
		bool preheating;	// preheating on? only used for torps in a tube
		stored_torpedo();
		stored_torpedo(game& gm, const std::string& type);
		void load(game& gm, const xml_elem& parent);
		void save(xml_elem& parent) const;
	};

	// submarine parts and their damages
	// fixme: replace german names by correct translations
	// lenzpumpe (water pump???) drain pump / drainage pump
	// bilge???
	// kitchen - kombuese?
	// balance tank - trimmtank?
	//fixme: it should not be necessary to store explicit variable names for that
	//parts. rather store xml files with data about the submarine types.
	enum damageable_parts_indices {
		// common parts
		rudder,
		screws,
		screw_shaft,
		stern_dive_planes,
		stern_water_pump,	//?
		stern_pressure_hull,
		stern_hatch,
		electric_engines,
		air_compressor,
		machine_water_pump,	//?
		machine_pressure_hull,
		aft_battery,
		diesel_engines,
		kitchen_hatch,		//?	// there was only one hatch at the stern!? fixme
		balance_tank_valves,	//?
		forward_battery,
		periscope,	// fixme: there were two...
		central_pressure_hull,
		bilge_water_pump,	//?
		conning_tower_hatch,
		listening_device,
		radio_device,
		inner_bow_tubes,
		outer_bow_tubes,
		bow_water_pump,		//?
		bow_hatch,
		bow_pressure_hull,	//fixme: damage view does not match 3d data or vice versa.
		bow_dive_planes,
		aa_gun,
		ammunition_depot,
		outer_fuel_tanks_left,
		outer_fuel_tanks_right,

		// parts specific to sub types
		outer_stern_tubes,
		inner_stern_tubes,
		snorkel,//fixme conflicts with bool snorkel;
		deck_gun,
		radio_detection_device,
		radar,
		
		nr_of_damageable_parts	// trick to count enum entries
	};

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
	
	static damage_data_scheme damage_schemes[nr_of_damageable_parts];

	struct damageable_part {
		double status;		// damage in percent, negative means part is not existent.
		double repairtime;
		damageable_part(double st = -1, double rt = 0) : status(st), repairtime(rt) {}
		//fixme: save to xml!
		//damageable_part(istream& in) { status = read_double(in); repairtime = read_double(in); }
		//void save(ostream& out) const { write_double(out, status); write_double(out, repairtime); }
	};

	enum front_rudder_status{
		rudder_down_30 = -3,
		rudder_down_20,
		rudder_down_10,
		rudder_center,
		rudder_up_10,
		rudder_up_20,
		rudder_up_30
	};

protected:
	double dive_speed;
	double dive_acceleration;	// read from spec file
	double max_dive_speed;		// read from spec file
	double max_depth;		// created with some randomness after spec file, must get stored!
	double dive_to;
	bool permanent_dive;

	unsigned int delayed_dive_to_depth;
	double delayed_planes_down;
	int bow_to;
	int stern_to;
	double bow_rudder;
	double stern_rudder;

	double max_submerged_speed;	// read from spec file

	// stored torpedoes (including tubes)
	// special functions calculate indices for bow/stern tubes etc., see below
	std::vector<stored_torpedo> torpedoes;
	unsigned number_of_tubes_at[6];	// read from spec file
	unsigned torp_transfer_times[5];// read from spec file

	bool scopeup;			// fixme: maybe simulate time for moving scope up/down
	double periscope_depth;		// read from spec file
	bool electric_engine;		// true when electric engine is used.
	bool hassnorkel;		// fixme: replace by (damageable_parts[snorkel] != unused)
	double snorkel_depth;		// read from spec file
	double alarm_depth;		// read from spec file
	bool snorkelup;
//	float sonar_cross_section_factor;

	// Charge level of battery: 0 = empty, 1 = fully charged
	double battery_level;
	double battery_value_a;		// read from spec file
	double battery_value_t;		// read from spec file
	double battery_recharge_value_a;// read from spec file
	double battery_recharge_value_t;// read from spec file
	unsigned battery_capacity;	// read from spec file
    
	std::vector<damageable_part> damageable_parts;	// read from data/spec file, fixme do that!

	// fixme: add: double temperature;	// overall temperature in submarine. used for torpedo preheating computation

	int find_stored_torpedo(bool usebow);	// returns index or -1 if none

	/**
		This method calculates the battery consumption rate. This value is needed
		for the simulate function to reduce the battery_level value. An
		exponential is used as a model basing on some battery consumption values.
		@return double hourly percentage battery consumption value
	*/
	virtual double get_battery_consumption_rate () const
	{ return battery_value_a * ( exp ( get_throttle_speed () / battery_value_t ) - 1.0f ); }
	/**
		This method method calculates the battery recharge rate.
		@return battery recharge rate
	*/
	virtual double get_battery_recharge_rate () const
	{ return ( 1.0f - ( battery_recharge_value_a *
    	exp ( - get_throttle_speed () / battery_recharge_value_t ) ) ); }

	virtual void calculate_fuel_factor ( double delta_time );
	
	virtual void gun_manning_changed(bool isGunManned);


	// in-hull temperature, depends on weather/latitude etc.
	// torpedo-temperature is set from this value for stored torpedoes (not in tubes)
	//double temperature;//maybe store for each torpedo and not here...

	tdc TDC;

public:
	// there were more types, I, X (mine layer), XIV (milk cow), VIIf, (and VIId)
	// and some experimental types. (VIIc42, XVIIa/b)
	// there were two IXd1 boats similar to type d2, but with different
	// engines.
/*
	enum types {
		typeIIa=256, typeIIb, typeIIc, typeIId,
		typeVIIa, typeVIIb, typeVIIc, typeVIIc41,
		typeIX, typeIXb, typeIXc, typeIXc40, typeIXd2,
		typeXXI,
		typeXXIII };
*/

	// create empty object from specification xml file
	submarine(game& gm_, const xml_elem& parent);

	~submarine();

	virtual void load(const xml_elem& parent);
	virtual void save(xml_elem& parent) const;
	
	virtual void simulate(double delta_time);

	void set_target(sea_object* s);

	// fill available tubes with common types depending on time period (used for custom missions)
	virtual void init_fill_torpedo_tubes(const class date& d);

	const std::vector<stored_torpedo>& get_torpedoes() const { return torpedoes; }

	// give number from 0-5 (bow tubes first)
	bool is_tube_ready(unsigned nr) const;

	// get number of tubes / stored reserve torpedoes
	virtual unsigned get_nr_of_bow_tubes() const { return number_of_tubes_at[0]; }
	virtual unsigned get_nr_of_stern_tubes() const { return number_of_tubes_at[1]; }
	virtual unsigned get_nr_of_bow_reserve() const { return number_of_tubes_at[2]; }
	virtual unsigned get_nr_of_stern_reserve() const { return number_of_tubes_at[3]; }
	virtual unsigned get_nr_of_bow_deckreserve() const { return number_of_tubes_at[4]; }
	virtual unsigned get_nr_of_stern_deckreserve() const { return number_of_tubes_at[5]; }

	// get first index of storage and first index after it (computed with functions above)
	pair<unsigned, unsigned> get_bow_tube_indices() const;
	pair<unsigned, unsigned> get_stern_tube_indices() const;
	pair<unsigned, unsigned> get_bow_reserve_indices() const;
	pair<unsigned, unsigned> get_stern_reserve_indices() const;
	pair<unsigned, unsigned> get_bow_deckreserve_indices() const;
	pair<unsigned, unsigned> get_stern_deckreserve_indices() const;
	unsigned get_location_by_tubenr(unsigned tn) const; // returns 1-6 as location number, 0 if not supported

	// The simulation of acceleration when switching between electro and diesel
	// engines is done via engine simulation. So the boat "brakes" until
	// it reaches its submerged speed. This is not correct, because speed decreases
	// too fast, but it should be satisfying for now. fixme
	virtual double get_max_speed() const;

	// compute probabilty that sub can be seen (determined by depth, speed,
	// state: periscope state, snorkeling etc., shape)
	virtual float surface_visibility(const vector2& watcher) const;
	virtual float sonar_visibility ( const vector2& watcher ) const;
	virtual double get_noise_factor () const;
	// return pointer to torpedo in tube or NULL if tube is empty
	virtual stored_torpedo& get_torp_in_tube(unsigned tubenr);
	virtual const stored_torpedo& get_torp_in_tube(unsigned tubenr) const;
	virtual bool is_scope_up() const { return ( scopeup == true ); }
	virtual double get_periscope_depth() const { return periscope_depth; }
	virtual bool is_submerged () const { return get_depth() > SUBMARINE_SUBMERGED_DEPTH; }
	virtual double get_max_depth () const { return max_depth; }
	virtual bool is_electric_engine () const { return (electric_engine == true); }
	virtual bool is_snorkel_up () const { return ( snorkelup == true ); }
	virtual bool has_snorkel () const { return ( hassnorkel == true ); }
	virtual double get_snorkel_depth () const { return snorkel_depth; }
	virtual double get_alarm_depth () const { return alarm_depth; }
	virtual double get_battery_level () const { return battery_level; }
	virtual const std::vector<damageable_part>& get_damage_status() const { return damageable_parts; }

	// get/compute torpedo transfer time and helper functions (uses functions below to compute)
	virtual double get_torp_transfer_time(unsigned from, unsigned to) const;
	virtual double get_bow_reload_time() const { return torp_transfer_times[0]; }
	virtual double get_stern_reload_time() const { return torp_transfer_times[1]; }
	virtual double get_bow_deck_reload_time() const { return torp_transfer_times[2]; }
	virtual double get_stern_deck_reload_time() const { return torp_transfer_times[3]; }
	virtual double get_bow_stern_deck_transfer_time() const { return torp_transfer_times[4]; }
	
	// damage is added if dc damages sub.
	virtual void depth_charge_explosion(const class depth_charge& dc);
    
	// command interface for subs
	virtual void scope_up() { scopeup = true; };	// fixme, do we need these?
	virtual void scope_down() { scopeup = false; };
	virtual void scope_to(double amount) {};	// set scope to "amount" (0-1) of full height, fixme
	virtual bool set_snorkel_up ( bool snorkel_up );	// fixme get rid of this
	virtual void snorkel_up() { set_snorkel_up(true); }
	virtual void snorkel_down() { set_snorkel_up(false); }
	virtual void planes_up(double amount);		// fixme: functions for both dive planes needed?
	virtual void planes_down(double amount);
	virtual void planes_middle();
	virtual void dive_to_depth(unsigned meters);
	virtual void transfer_torpedo(unsigned from, unsigned to);

	virtual void launch_torpedo(int tubenr, sea_object* target); // give tubenr -1 for any loaded tube, or else 0-5
	virtual void set_throttle(ship::throttle_status thr);
	// end of command interface

	virtual bool has_deck_gun() { return !gun_turrets.empty(); }
	
	virtual void start_throttle_sound();
	virtual void stop_throttle_sound();

	virtual tdc& get_tdc() { return TDC; }
	virtual const tdc& get_tdc() const { return TDC; }

	virtual double get_bow_rudder() const { return bow_rudder; }
	virtual double get_stern_rudder() const { return bow_rudder; } // stern is the same so far
	virtual void bow_pos(int state){ bow_to = state; permanent_dive = true; }
	virtual void stern_pos(int state){ bow_to = state; permanent_dive = true; } // stern is the same so far
};

#endif
