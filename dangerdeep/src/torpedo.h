// torpedoes
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef TORPEDO_H
#define TORPEDO_H

#include "sea_object.h"
#include <map>
using namespace std;

#define G7A_HITPOINTS 100

class torpedo : public sea_object
{
protected:
	// Types: G7a, G7e, G7e FAT, G7e acustic (TV)
	double run_length, max_run_length;
	sea_object* parent;
	unsigned type;
	
	torpedo() {};
	torpedo& operator=(const torpedo& other);
	torpedo(const torpedo& other);
public:
	enum types { none, T1, T3, T5, T3FAT, T6LUT, T11, reloading=0xffffffff };

	virtual ~torpedo() {};
	torpedo(sea_object* parent_, unsigned type_, bool usebowtubes);
	virtual void simulate(class game& gm, double delta_time);
	virtual void hit(sea_object* other);	// action when object is hit
	virtual void display(void) const;

	// compute gyro lead angle and expected run time of torpedo
	pair<angle, bool> lead_angle(double target_speed,
		angle angle_on_the_bow) const;
	double expected_run_time(angle lead_angle,
	        angle angle_on_the_bow, double target_range) const;

	// adjust heading of torpedo, returns false if impossible
	bool adjust_head_to(const sea_object* target, bool usebowtubes);
};

#endif
