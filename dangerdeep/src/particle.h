// particle (C)+(W) 2004 Thorsten Jordan

#ifndef PARTICLE_H
#define PARTICLE_H

#include "vector3.h"
#include <list>
using namespace std;

// particles: smoke, water splahes, fire, explosions, spray caused by ship's bow
// fire particles can produce smoke particles!

class particle
{
protected:
	//fixme: store textures somewhere! static variables?!

	vector3 pos;
	float live;	// 0...1, 0 = faded out
	particle() : live(1.0f) {}
	particle(const particle& other);
	particle& operator= (const particle& other);

	// returns wether particle is shown parallel to z-axis (true), or 3d billboarding always (false)
	virtual bool is_z_up(void) const { return true; }

	// helper struct for depth sorting
	struct particle_dist {
		const particle* pt;
		double dist;
		vector3 projpos;
		particle_dist(const particle* p, double d, const vector3& pp) : pt(p), dist(d), projpos(pp) {}
		bool operator< (const particle_dist& other) const { return dist >= other.dist; }
	};

public:
	particle(const vector3& pos_) : pos(pos_), live(1.0f) {}
	virtual ~particle() {}

	// class game is given so that particles can spawn other particles (fire->smoke)
	virtual void simulate(class game& gm, double delta_t) = 0;

	static void display_all(const list<particle*>& pts, const vector3& viewpos, class game& gm);

	// return width/height (in meters) of particle (length of quad edge)
	virtual double get_width(void) const = 0;
	virtual double get_height(void) const = 0;

	virtual bool is_dead(void) const { return live <= 0.0f; }
	
	// set opengl texture by particle type or e.g. game time etc.
	virtual void set_texture(class game& gm) const = 0;
};



class smoke_particle : public particle
{
	bool is_z_up(void) const { return false; }
public:
	smoke_particle(const vector3& pos_) : particle(pos_) {}
	~smoke_particle() {}
	void simulate(class game& gm, double delta_t);
	double get_width(void) const;
	double get_height(void) const;
	void set_texture(class game& gm) const;
	static double get_produce_time(void);
};

#endif
