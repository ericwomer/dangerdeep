// particle (C)+(W) 2004 Thorsten Jordan

#ifndef PARTICLE_H
#define PARTICLE_H

#include "vector3.h"
#include <vector>
#include <list>
using namespace std;

class game;
class texture;

// particles: smoke, water splahes, fire, explosions, spray caused by ship's bow
// fire particles can produce smoke particles!

typedef unsigned char Uint8;

class particle
{
protected:
	//fixme: store textures somewhere! static variables?!

	vector3 position;
	vector3 velocity;
	float life;	// 0...1, 0 = faded out
	particle() : life(1.0f) {}
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

	// particle textures (generated and stored once)
	static unsigned init_count;
	static vector<texture*> tex_smoke;
	static texture* tex_spray;
	static vector<texture*> tex_fire;
	static vector<texture*> explosionbig;
	static vector<texture*> explosionsml;

	// wh must be power of two (returns a square). 1 <= 2^low <= 2^high <= wh
	static vector<float> interpolate_func;
	static vector<Uint8> make_2d_smoothed_noise_map(unsigned wh);
	static unsigned interpolate_2d_map(const vector<Uint8>& mp, unsigned res, unsigned x, unsigned y, unsigned res2);
	static vector<Uint8> make_2d_perlin_noise(unsigned wh, unsigned highestlevel);
	static vector<Uint8> compute_fire_frame(unsigned wh, const vector<Uint8>& oldframe);

	virtual vector3 get_acceleration(void) const { return vector3(); }

public:
	particle(const vector3& pos, const vector3& velo = vector3()) : position(pos), life(1.0f) {}
	virtual ~particle() {}

	static void init(void);
	static void deinit(void);

	virtual vector3 get_pos(void) const { return position; }
	virtual void set_pos(const vector3& pos) { position = pos; }

	// class game is given so that particles can spawn other particles (fire->smoke)
	virtual void simulate(game& gm, double delta_t);

	static void display_all(const list<particle*>& pts, const vector3& viewpos, game& gm);

	// return width/height (in meters) of particle (length of quad edge)
	virtual double get_width(void) const = 0;
	virtual double get_height(void) const = 0;

	virtual void kill(void) { life = 0.0f; }
	virtual bool is_dead(void) const { return life <= 0.0f; }
	
	// set opengl texture by particle type or e.g. game time etc.
	virtual void set_texture(game& gm) const = 0;

	virtual double get_life_time(void) const = 0;
};



class smoke_particle : public particle
{
	bool is_z_up(void) const { return false; }
	unsigned texnr;
	vector3 get_acceleration(void) const;
public:
	smoke_particle(const vector3& pos);//set velocity by wind, fixme
	~smoke_particle() {}
	double get_width(void) const;
	double get_height(void) const;
	void set_texture(game& gm) const;
	double get_life_time(void) const;
	static double get_produce_time(void);
};



class smoke_particle_escort : public smoke_particle
{
public:
	smoke_particle_escort(const vector3& pos);//set velocity by wind, fixme
	~smoke_particle_escort() {}
	double get_width(void) const;
	double get_life_time(void) const;
	static double get_produce_time(void);
};



class explosion_particle : public particle
{
	unsigned extype;	// which texture
public:
	explosion_particle(const vector3& pos);
	~explosion_particle() {}
	double get_width(void) const;
	double get_height(void) const;
	void set_texture(game& gm) const;
	double get_life_time(void) const;
};



class fire_particle : public particle
{
//	unsigned firetype;	// which texture
public:
	fire_particle(const vector3& pos);
	~fire_particle() {}
	void simulate(game& gm, double delta_t);
	double get_width(void) const;
	double get_height(void) const;
	void set_texture(game& gm) const;
	double get_life_time(void) const;
};



class spray_particle : public particle
{
public:
	spray_particle(const vector3& pos, const vector3& velo);
	~spray_particle() {}
	double get_width(void) const;
	double get_height(void) const;
	void set_texture(game& gm) const;
	double get_life_time(void) const;
};

#endif
