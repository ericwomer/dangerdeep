// airplanes
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef AIRPLANE_H
#define AIRPLANE_H

#include "sea_object.h"
#include "global_data.h"
#include "quaternion.h"

class airplane : public sea_object
{
protected:
	quaternion rotation;	// local plane space to world space
	vector3 velocity;	//fixme: move to sea_object?
	double rollfac, pitchfac;

	airplane();
	airplane& operator=(const airplane& other);
	airplane(const airplane& other);
	
	virtual void parse_attributes(class TiXmlElement* parent);
	
public:
	// create empty object from specification xml file
	airplane(class TiXmlDocument* specfile);

//	airplane() { rotation = quaternion::neutral_rot(); velocity = vector3(0,100,0); } obsolete

	virtual ~airplane() {};

	void load(istream& in, class game& g);
	void save(ostream& out, const class game& g) const;

	virtual void simulate(class game& gm, double delta_time);

	virtual quaternion get_rotation(void) const { return rotation; }

	virtual double get_mass(void) const { return 4000.0; }	// 4 tons.
	virtual double get_engine_thrust(void) const { return 20000.0; }
	virtual double get_drag_factor(void) const { return 0.00005184; }
	virtual double get_antislide_factor(void) const { return 0.0025; }
	virtual double get_antilift_factor(void) const { return 0.04; }
	virtual double get_lift_factor(void) const { return 0.75; }
	virtual double get_roll_deg_per_sec(void) const { return 90.0; }
	virtual double get_pitch_deg_per_sec(void) const { return 45.0; }

	// command interface for airplanes
	virtual void roll_left(void);
	virtual void roll_right(void);
	virtual void roll_zero(void);
	virtual void pitch_down(void);
	virtual void pitch_up(void);
	virtual void pitch_zero(void);
};

#endif
