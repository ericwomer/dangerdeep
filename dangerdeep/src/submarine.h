// submarines
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef SUBMARINE_H
#define SUBMARINE_H

#include "ship.h"
#include "torpedo.h"
#include "parser.h"
#include <vector>

#define SUBMARINE_SUBMERGED_DEPTH 2.0f // meters

#define CROSS_SECTION_ACT_SONAR_LSUB     0.20f        // Large Submarines (?)
#define CROSS_SECTION_ACT_SONAR_NSUB     0.15f        // Normal Submarines (?)
#define CROSS_SECTION_ACT_SONAR_MSUB_AC  0.05f        // Midget Submarines (?), Aircrafts
#define CROSS_SECTION_ACT_SONAR_TORPWB   0.01f        // Torpedo

class submarine : public ship
{
public:
	struct stored_torpedo {
		unsigned type;
		unsigned status;	// 0 empty 1 reloading 2 unloading 3 loaded
		unsigned associated;	// reloading from/unloading to
		double remaining_time;	// remaining time until work is finished
		stored_torpedo() : type(0), status(0), associated(0), remaining_time(0) {}
		stored_torpedo(unsigned t) : type(t), status(3), associated(0), remaining_time(0) {}
	};

protected:
	double dive_speed, dive_acceleration, max_dive_speed, max_depth, dive_to;
	bool permanent_dive;
	double max_submerged_speed;

	// stored torpedoes (including tubes)
	// special functions calculate indices for bow/stern tubes etc., see below
	vector<stored_torpedo> torpedoes;

	bool scopeup;	// fixme: maybe simulate time for moving scope up/down
	double periscope_depth;
	bool electric_engine; // true when electric engine is used.
	double snorkel_depth; // -1 when snorkel not available.
	float sonar_cross_section_factor;
    
	submarine();
	submarine& operator= (const submarine& other);
	submarine(const submarine& other);

	// fixme: time that is needed depends on sub type and how many torpedoes
	// are already in transfer. So this argument is nonesense. fixme
	// returns true if transfer was initiated.
	// fixme: make virtual?
	bool transfer_torpedo(unsigned from, unsigned to, double timeneeded = 120);
	int find_stored_torpedo(bool usebow);	// returns index or -1 if none

	bool parse_attribute(parser& p);	// returns false if invalid token found

public:
	// fixme type II has some subtypes.
	enum types { typeII, typeVII, typeVIIb, typeVIIc, typeVIIc41,
		typeIX, typeIXb, typeIXc, typeIXc40, typeIXd2, typeXXI, typeXXIII };
	virtual ~submarine() {}
	static submarine* create(types type_);
	static submarine* create(parser& p);
	virtual void simulate(class game& gm, double delta_time);
	virtual void display(void) const = 0;

	const vector<stored_torpedo>& get_torpedoes(void) const { return torpedoes; }
	// get first index of storage and first index after it.
	virtual pair<unsigned, unsigned> get_bow_tube_indices(void) const = 0;
	virtual pair<unsigned, unsigned> get_stern_tube_indices(void) const = 0;
	virtual pair<unsigned, unsigned> get_bow_storage_indices(void) const = 0;
	virtual pair<unsigned, unsigned> get_stern_storage_indices(void) const = 0;
	virtual pair<unsigned, unsigned> get_bow_top_storage_indices(void) const = 0;
	virtual pair<unsigned, unsigned> get_stern_top_storage_indices(void) const = 0;

	// The simulation of acceleration when switching between electro and diesel
	// engines is done via engine simulation. So the boat "brakes" until
	// it reaches its submerged speed. This is not correct, because speed decreases
	// too fast, but it should be satisfying for now. fixme
	virtual double get_max_speed(void) const;

	// compute probabilty that sub can be seen (determined by depth, speed,
	// state: periscope state, snorkeling etc., shape)
	virtual float surface_visibility(const vector2& watcher) const;
	virtual float sonar_visibility ( const vector2& watcher ) const;
	virtual double get_noise_factor () const;

	virtual bool is_scope_up(void) const { return scopeup; }
	virtual double get_periscope_depth() const { return periscope_depth; }
	virtual bool is_submerged () const { return get_depth() > SUBMARINE_SUBMERGED_DEPTH; }
	virtual double get_max_depth () const { return max_depth; }
	virtual bool is_electric_engine (void) const { return (electric_engine == true); }
    
	// command interface for subs
	virtual void scope_up(void) { scopeup = true; };	// fixme
	virtual void scope_down(void) { scopeup = false; };		
	virtual void planes_up(double amount);
	virtual void planes_down(double amount);
	virtual void planes_middle(void);
	virtual void dive_to_depth(unsigned meters);
	// give tubenr -1 for any loaded tube, or else 0-5
	virtual bool fire_torpedo(class game& gm, int tubenr, sea_object* target);
};

#endif
