// submarines
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef SUBMARINE_H
#define SUBMARINE_H

#include "ship.h"
#include "torpedo.h"

class submarine : public ship
{
protected:
	double dive_speed, dive_acceleration, max_dive_speed, max_depth, dive_to;
	bool permanent_dive;
	double max_submerged_speed;
	
	// store torpedo type, 0 empty, <0 reloading
	vector<unsigned> bow_tubes;
	vector<unsigned> stern_tubes;
	vector<unsigned> bow_storage;
	vector<unsigned> stern_storage;
	vector<unsigned> bow_top_storage;
	vector<unsigned> stern_top_storage;

	submarine() {};
	submarine& operator= (const submarine& other);
	submarine(const submarine& other);
public:
	enum types { typeII, typeVII, typeVIIb, typeVIIc, typeVIIc41,
		typeIXb, typeIXc, typeIXd2, typeXXI, typeXXIII };
	virtual ~submarine() {}
	submarine(unsigned type_, const vector3& pos, angle heading);
	virtual void simulate(class game& gm, double delta_time);
	virtual void display(void) const;

	const vector<unsigned>& get_bow_tubes(void) const { return bow_tubes; }
	const vector<unsigned>& get_stern_tubes(void) const { return stern_tubes; }
	const vector<unsigned>& get_bow_storage(void) const { return bow_storage; }
	const vector<unsigned>& get_stern_storage(void) const { return stern_storage; }
	const vector<unsigned>& get_bow_top_storage(void) const { return bow_top_storage; }
	const vector<unsigned>& get_stern_top_storage(void) const { return stern_top_storage; }

	// The simulation of acceleration when switching between electro and diesel
	// engines is done via engine simulation. So the boat "brakes" until
	// it reaches its submerged speed. This is not correct, because speed decreases
	// too fast, but it should be satisfying for now. fixme
	virtual double get_max_speed(void) const;
		
	virtual void planes_up(double amount);
	virtual void planes_down(double amount);
	virtual void planes_middle(void);
	virtual void dive_to_depth(unsigned meters);
	virtual bool fire_torpedo(class game& gm, bool usebowtubes, unsigned tubenr,
		sea_object* target);
};

#endif
