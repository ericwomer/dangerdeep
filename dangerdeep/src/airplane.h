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

	airplane();
	airplane& operator=(const airplane& other);
	airplane(const airplane& other);
public:
	virtual ~airplane() {};

	// types:
	airplane(unsigned type_, const vector3& pos, double heading);
	virtual void display(void) const;
};

#endif
