// torpedoes
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef TORPEDO_H
#define TORPEDO_H

#include "sea_object.h"
#include <map>
using namespace std;

#define G7A_HITPOINTS 4	// maximum
#define TORPEDO_SAVE_DISTANCE 250.0f // minimum distance

// fixme: maybe make heirs of this class for each torpedo type. would be much nicer code.

class torpedo : public sea_object
{
public:
	// T1: G7a
	// T2: G7e with contact fuse
	// T3: G7e with influence fuse
	// T3a: range improved (150%) T3, available mid 1942
	// T4: G7e acoustic (Falke/Falcon) 7500m at 20kts. Used in march 1943
	// T5: G7e acoustic (Zaunkoenig/Gnat) 24kts
	// T11: like T5, less affected by "foxer" noise-making decoy
	// FAT: available for T1 or T3s
	// LUT: available for T3s (named T6 ?)
	// fixme: how widely was influence fuse used? there were severly
	// malfunctions, even later in the war. Influence fuse is not simulated yet
	// so T2 == T3
	enum types { none, T1, T2, T3, T3a, T4, T5, T11, T1FAT, T3FAT, T6LUT, reloading=0xffffffff };

protected:
	double run_length, max_run_length;
	sea_object* parent;
	unsigned type;
	bool influencefuse;	// determined by type
	
	unsigned primaryrange, secondaryrange;	// meters
	unsigned initialturn;	// 0/1 left/right
	unsigned searchpattern;	// 0/1 turn 180 or 90 degrees

	torpedo& operator=(const torpedo& other);
	torpedo(const torpedo& other);

	// specific damage here:
	virtual void create_sensor_array ( types t );

public:
	torpedo() {};
	virtual ~torpedo() {};
	void load(istream& in, class game& g);
	void save(ostream& out, const class game& g) const;
	
	// additional FAT/LUT values as indices (0-16,0-1,0-1,0-1)
	torpedo(sea_object* parent_, unsigned type_, bool usebowtubes,
		unsigned pr = 0, unsigned sr = 0, unsigned it = 0, unsigned sp = 0);
	virtual void simulate(class game& gm, double delta_time);
	virtual void display(void) const;

	// compute gyro lead angle and expected run time of torpedo
	pair<angle, bool> lead_angle(double target_speed,
		angle angle_on_the_bow) const;
	double expected_run_time(angle lead_angle,
	        angle angle_on_the_bow, double target_range) const;

	// adjust heading of torpedo, returns false if impossible
	bool adjust_head_to(const sea_object* target, bool usebowtubes,
		const angle& manual_lead_angle);
	virtual unsigned get_hit_points () const;
};

#endif
