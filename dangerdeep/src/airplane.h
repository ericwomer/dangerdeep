// airplanes
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef AIRPLANE_H
#define AIRPLANE_H

#include "sea_object.h"
#include "global_data.h"

class airplane : public sea_object
{
protected:
	unsigned type;
	angle pitch, roll;

	airplane& operator=(const airplane& other);
	airplane(const airplane& other);
public:
	airplane() {}
	virtual ~airplane() {};
	void load(istream& in, class game& g);
	void save(ostream& out, const class game& g) const;

	// types:
	airplane(unsigned type_, const vector3& pos, double heading);

	const model* get_model (void) const;
};

#endif
