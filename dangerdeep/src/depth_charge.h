// depth charges
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef DEPTH_CHARGE_H
#define DEPTH_CHARGE_H

#include "sea_object.h"

// fixme: these values depend on depth charge type.
#define DEPTH_CHARGE_SINK_SPEED 4	// m/sec
#define DEADLY_DC_RADIUS_SURFACE 120	// meters
#define DEADLY_DC_RADIUS_200M 80
#define DAMAGE_DC_RADIUS_SURFACE 480	// meters, fixme realistic values?
#define DAMAGE_DC_RADIUS_200M 320

class depth_charge : public sea_object
{
protected:
	double explosion_depth;

	depth_charge& operator=(const depth_charge& other);
	depth_charge(const depth_charge& other);
public:
	depth_charge() {};
	virtual ~depth_charge() {};
	void load(istream& in, class game& g);
	void save(ostream& out, const class game& g) const;
	depth_charge(const sea_object& parent, double expl_depth);
	virtual void simulate(class game& gm, double delta_time);
	virtual void display(void) const;
	virtual vector3 get_acceleration(void) const;
};

#endif
