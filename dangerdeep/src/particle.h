/*
Danger from the Deep - Open source submarine simulation
Copyright (C) 2003-2006  Thorsten Jordan, Luis Barrancos and others.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

// particle (C)+(W) 2004 Thorsten Jordan

#ifndef PARTICLE_H
#define PARTICLE_H

#include "vector3.h"
#include <vector>

class game;
class texture;

// particles: smoke, water splashes, fire, explosions, spray caused by ship's bow
// fire particles can produce smoke particles!

typedef unsigned char Uint8;

///\brief Simulates and displays particles that are rendered as billboard images.
class particle
{
protected:
	vector3 position;
	vector3 velocity;
	double life;	// 0...1, 0 = faded out
	particle() : life(1.0) {}
	particle(const particle& other);
	particle& operator= (const particle& other);

	// returns wether particle is shown parallel to z-axis (true), or 3d billboarding always (false)
	virtual bool is_z_up() const { return true; }
	
	// returns wether image should be drawn above pos or centered around pos
	virtual bool tex_centered() const { return true; }

	// helper struct for depth sorting
	struct particle_dist {
		const particle* pt;
		double dist;
		vector3 projpos;
		particle_dist(const particle* p, double d, const vector3& pp) : pt(p), dist(d), projpos(pp) {}
		bool operator< (const particle_dist& other) const { return dist > other.dist; }
	};

	// particle textures (generated and stored once)
	static unsigned init_count;
	static std::vector<texture*> tex_smoke;
	static texture* tex_spray;
	static std::vector<texture*> tex_fire;
	static std::vector<texture*> explosionbig;
	static std::vector<texture*> explosionsml;
	static std::vector<texture*> watersplashes;

	// wh must be power of two (returns a square). 1 <= 2^low <= 2^high <= wh
	static std::vector<float> interpolate_func;
	static std::vector<Uint8> make_2d_smoothed_noise_map(unsigned wh);
	static unsigned interpolate_2d_map(const std::vector<Uint8>& mp, unsigned res, unsigned x, unsigned y, unsigned res2);

	// 1 <= highest_level <= log2(wh)
	static std::vector<Uint8> make_2d_perlin_noise(unsigned wh, unsigned highestlevel);
	static std::vector<Uint8> compute_fire_frame(unsigned wh, const std::vector<Uint8>& oldframe);

	virtual vector3 get_acceleration() const { return vector3(); }

public:
	particle(const vector3& pos, const vector3& velo = vector3()) : position(pos), velocity(velo), life(1.0) {}
	virtual ~particle() {}

	static void init();
	static void deinit();

	virtual vector3 get_pos() const { return position; }
	virtual void set_pos(const vector3& pos) { position = pos; }

	// class game is given so that particles can spawn other particles (fire->smoke)
	virtual void simulate(game& gm, double delta_t);

	static void display_all(const std::vector<particle*>& pts, const vector3& viewpos, game& gm);

	// return width/height (in meters) of particle (length of quad edge)
	virtual double get_width() const = 0;
	virtual double get_height() const = 0;

	virtual void kill() { life = 0.0; }
	virtual bool is_defunct() const { return life <= 0.0; }
	
	// set opengl texture by particle type or e.g. game time etc.
	virtual void set_texture(game& gm) const = 0;

	virtual double get_life_time() const = 0;
};



class smoke_particle : public particle
{
	bool is_z_up() const { return false; }
	unsigned texnr;
	vector3 get_acceleration() const;
public:
	smoke_particle(const vector3& pos);//set velocity by wind, fixme
	~smoke_particle() {}
	double get_width() const;
	double get_height() const;
	void set_texture(game& gm) const;
	double get_life_time() const;
	static double get_produce_time();
};



class smoke_particle_escort : public smoke_particle
{
public:
	smoke_particle_escort(const vector3& pos);//set velocity by wind, fixme
	~smoke_particle_escort() {}
	double get_width() const;
	double get_life_time() const;
	static double get_produce_time();
};



class explosion_particle : public particle
{
	unsigned extype;	// which texture
public:
	// is_z_up could be false for this kind of particle
	explosion_particle(const vector3& pos);
	~explosion_particle() {}
	double get_width() const;
	double get_height() const;
	void set_texture(game& gm) const;
	double get_life_time() const;
};



class fire_particle : public particle
{
//	unsigned firetype;	// which texture
public:
	// only particle where is_z_up should be true.
	fire_particle(const vector3& pos);
	~fire_particle() {}
	void simulate(game& gm, double delta_t);
	double get_width() const;
	double get_height() const;
	void set_texture(game& gm) const;
	double get_life_time() const;
};



class spray_particle : public particle
{
public:
	// is_z_up could be false for this kind of particle
	spray_particle(const vector3& pos, const vector3& velo);
	~spray_particle() {}
	double get_width() const;
	double get_height() const;
	void set_texture(game& gm) const;
	double get_life_time() const;
};



// obsolete from here...
class torpedo_water_splash_particle : public particle
{
	virtual bool tex_centered() const { return false; }
public:
	torpedo_water_splash_particle(const vector3& pos);
	~torpedo_water_splash_particle() {}
	double get_width() const;
	double get_height() const;
	void set_texture(game& gm) const;
	double get_life_time() const;
};



class gun_shell_water_splash_particle : public particle
{
	virtual bool tex_centered() const { return false; }
public:
	gun_shell_water_splash_particle(const vector3& pos);
	~gun_shell_water_splash_particle() {}
	double get_width() const;
	double get_height() const;
	void set_texture(game& gm) const;
	double get_life_time() const;
};



class depth_charge_water_splash_particle : public particle
{
	virtual bool tex_centered() const { return false; }
public:
	depth_charge_water_splash_particle(const vector3& pos);
	~depth_charge_water_splash_particle() {}
	double get_width() const;
	double get_height() const;
	void set_texture(game& gm) const;
	double get_life_time() const;
};

#endif
