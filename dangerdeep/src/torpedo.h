// torpedoes
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef TORPEDO_H
#define TORPEDO_H

#include "sea_object.h"
#include <map>
using namespace std;

#define G7A_HITPOINTS 4	// maximum
#define TORPEDO_SAVE_DISTANCE 250.0f // minimum distance

class torpedo : public sea_object
{
public:
	enum types { none, T1, T3, T5, T3FAT, T6LUT, T11, reloading=0xffffffff };

protected:
	// Types: G7a, G7e, G7e FAT, G7e acustic (TV)
	double run_length, max_run_length;
	sea_object* parent;
	unsigned type;

	torpedo();
	torpedo& operator=(const torpedo& other);
	torpedo(const torpedo& other);

	// specific damage here:
	virtual void create_sensor_array ( types t );

public:
	virtual ~torpedo() {};
	torpedo(sea_object* parent_, unsigned type_, bool usebowtubes);
	virtual void simulate(class game& gm, double delta_time);
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
