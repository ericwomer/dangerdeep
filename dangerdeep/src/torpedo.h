// torpedoes
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef TORPEDO_H
#define TORPEDO_H

#include "ship.h"
#include <map>
using namespace std;

#define G7A_HITPOINTS 4	// maximum
#define TORPEDO_SAFE_DISTANCE 250.0f // minimum distance
#define TORPEDO_ACTIVATION_DISTANCE 400.0f // minimum distance for t4/t5/t11

// fixme: maybe make heirs of this class for each torpedo type. would be much nicer code.
// unnecessary work. instead maybe use same code as for ships/subs with XML.
// torpedo should heir from ship, as it has the same rudders/code as a ship.

class torpedo : public ship
{
 private:
	torpedo();
	torpedo& operator=(const torpedo& other);
	torpedo(const torpedo& other);

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
	// what happens if an influence fuse torpedo hits a ship hull?
	// does that ignite the fuse because of the magnetism or did they have an additional
	// contact fuse?? fixme
	enum types { none, T1, T2, T3, T3a, T4, T5, T11, T1FAT, T3FAT, T6LUT, reloading=0xffffffff };

protected:
	double run_length, max_run_length;	// this could be done with fuel simulation...
	types type;
	bool influencefuse;	// determined by type
	
	// add flags for FAT/LUT here? maybe make .xml files more generic
	// e.g. one file for T1 with flags FAT/LUT/influence-impact fuse etc.
	
	// configured by the player
	unsigned primaryrange;	// 1600...3200
	unsigned secondaryrange;// 800/1600
	unsigned initialturn;	// 0-1 (left or right, for FAT/LUT)
	unsigned turnangle;	// (0...180 degrees, for LUT, FAT has 180)
	unsigned torpspeed;	// torpspeed (0-2 slow-fast, only for G7a torps)
	double rundepth;	// depth the torpedo should run at

	double temperature;	// only useful for electric torpedoes

	// specific damage here:
	virtual void create_sensor_array ( types t );
	
	static double get_speed_by_type(types t);

	bool use_simple_turning_model(void) const { return true; }
	
	virtual bool causes_spray(void) const { return false; }
	
public:
	torpedo(game& gm_);
	virtual ~torpedo();
	void load(istream& in);
	void save(ostream& out) const;
	
	// additional FAT/LUT values as indices (0-16,0-1,0-1,0-1)
	torpedo(game& gm_, sea_object* parent, types type_, bool usebowtubes, angle headto_,
		const class tubesetup& stp);
	virtual void simulate(double delta_time);
	virtual void display(void) const;

	// compute gyro lead angle and expected run time of torpedo
	static pair<angle, bool> lead_angle(types torptype, double target_speed,
		angle angle_on_the_bow);
	static double expected_run_time(types torptype, angle lead_angle,
	        angle angle_on_the_bow, double target_range);

	// can torpedo gyroscope be adjusted to this target?
	// note that parent is a ship, no sea_object. airplanes can also launch torpedoes
	// but target computing would be done in a different way.
	// fixme: this function assumes that torpedoes are launched from bow or stern
	// what is not always right for ships.
	static pair<angle, bool> compute_launch_data(types torptype, const ship* parent,
		const sea_object* target, bool usebowtubes, const angle& manual_lead_angle);

	virtual unsigned get_hit_points () const;
};

#endif
