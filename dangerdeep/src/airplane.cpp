// airplanes
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "airplane.h"
#include "model.h"

airplane::airplane(unsigned type_, const vector3& pos, double heading) : sea_object()
{
	type = type_;
	position = pos;
	this->heading = heading;
	head_to = heading;
	pitch = roll = 0;
//	turn_rate = deg2rad(5);
//	length = 7;
//	width = 1;
	switch (type_) {
/*
		case 3:
			speed = 8;
			max_speed = 17.6;
			max_rev_speed = 5;
			acceleration = 0.8;
			turn_rate = 1;
			break;
*/			
	}
	
	throttle = aheadfull;
}

void airplane::load(istream& in, class game& g)
{
	sea_object::load(in, g);
	type = read_u32(in);
	pitch = angle(read_double(in));
	roll = angle(read_double(in));
}

void airplane::save(ostream& out, const class game& g) const
{
	sea_object::save(out, g);
	write_u32(out, type);
	write_double(out, pitch.value());
	write_double(out, roll.value());
}

const model* airplane::get_model(void) const
{
	switch(type) {
		default:
			return std_plane;
	}
}
