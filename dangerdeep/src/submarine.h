// submarines
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef SUBMARINE_H
#define SUBMARINE_H

#include "ship.h"
#include "torpedo.h"

class submarine : public ship
{
public:
	struct stored_torpedo {
		unsigned type;
		unsigned status;	// 0 empty 1 reloading 2 unloading 3 loaded
		unsigned associated;	// reloading from/unloading to
		double remaining_time;	// remaining time until work is finished
		stored_torpedo(unsigned t) : type(t), status(3), associated(0), remaining_time(0) {}
	};
	
protected:
	double dive_speed, dive_acceleration, max_dive_speed, max_depth, dive_to;
	bool permanent_dive;
	double max_submerged_speed;
	
	// stored torpedoes (including tubes)
	// special functions calculate indices for bow/stern tubes etc., see below
	vector<stored_torpedo> torpedoes;
	unsigned nr_bow_tubes, nr_stern_tubes, nr_bow_storage, nr_stern_storage,
		nr_bow_top_storage, nr_stern_top_storage;
		
	bool scopeup;	// fixme: maybe simulate time for moving scope up/down

	submarine() {};
	submarine& operator= (const submarine& other);
	submarine(const submarine& other);
	
	// fixme: time that is needed depends on sub type and how many torpedoes
	// are already in transfer. So this argument is nonesense. fixme
	// returns true if transfer was initiated.
	bool transfer_torpedo(unsigned from, unsigned to, double timeneeded = 120);
	int find_stored_torpedo(bool usebow);	// returns index or -1 if none
	
public:
	enum types { typeII, typeVII, typeVIIb, typeVIIc, typeVIIc41,
		typeIXb, typeIXc, typeIXd2, typeXXI, typeXXIII };
	virtual ~submarine() {}
	submarine(unsigned type_, const vector3& pos, angle heading);
	virtual void simulate(class game& gm, double delta_time);
	virtual void display(void) const;

	const vector<stored_torpedo>& get_torpedoes(void) const { return torpedoes; }
	// get first index of storage and first index after it.
	virtual pair<unsigned, unsigned> get_bow_tube_indices(void) const;
	virtual pair<unsigned, unsigned> get_stern_tube_indices(void) const;
	virtual pair<unsigned, unsigned> get_bow_storage_indices(void) const;
	virtual pair<unsigned, unsigned> get_stern_storage_indices(void) const;
	virtual pair<unsigned, unsigned> get_bow_top_storage_indices(void) const;
	virtual pair<unsigned, unsigned> get_stern_top_storage_indices(void) const;

	// The simulation of acceleration when switching between electro and diesel
	// engines is done via engine simulation. So the boat "brakes" until
	// it reaches its submerged speed. This is not correct, because speed decreases
	// too fast, but it should be satisfying for now. fixme
	virtual double get_max_speed(void) const;
	
	virtual bool is_scope_up(void) const { return scopeup; }

	// command interface for subs
	virtual void scope_up(void) { scopeup = true; };	// fixme
	virtual void scope_down(void) { scopeup = false; };		
	virtual void planes_up(double amount);
	virtual void planes_down(double amount);
	virtual void planes_middle(void);
	virtual void dive_to_depth(unsigned meters);
	// give tubenr -1 for any loaded tube, or else 0-5
	virtual bool fire_torpedo(class game& gm, bool usebowtubes, int tubenr,
		sea_object* target);
};

#endif
