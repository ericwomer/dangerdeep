// user interface for controlling a sea_object
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include <list>
#include <vector>
using namespace std;
#include "sea_object.h"
#include "global_data.h"

class user_interface
{
protected:
	user_interface() {};
	user_interface& operator= (const user_interface& other);
	user_interface(const user_interface& other);

public:	
	virtual ~user_interface() {};
	virtual bool user_quits(void) const = 0;
	virtual bool paused(void) const = 0;
	virtual unsigned time_scaling(void) const = 0;	// returns factor 1-x

	// display and input processing via "sys"
	virtual void display(class system& sys, class game& gm) = 0;
};

#endif
