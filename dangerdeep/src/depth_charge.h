// depth charges
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef DEPTH_CHARGE_H
#define DEPTH_CHARGE_H

#include "sea_object.h"

#define DEPTH_CHARGE_SINK_SPEED 4	// m/sec
#define DEADLY_DC_RADIUS_SURFACE 120	// meters
#define DEADLY_DC_RADIUS_200M 80

class depth_charge : public sea_object
{
protected:
	double explosion_depth;

	depth_charge();
	depth_charge& operator=(const depth_charge& other);
	depth_charge(const depth_charge& other);
public:
	virtual ~depth_charge() {};
	depth_charge(const sea_object& parent, double expl_depth);
	virtual void simulate(class game& gm, double delta_time);
	virtual void display(void) const;
    virtual depth_charge* get_depth_charge_ptr () { return this; }
    virtual const depth_charge* get_depth_charge_ptr () const { return this; }
};

#endif
