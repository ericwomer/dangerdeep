// particle (C)+(W) 2004 Thorsten Jordan

#include "particle.h"
#include "game.h"
#include "texture.h"
#include "oglext/OglExt.h"

#include "global_data.h" //for smoke texture, fixme

unsigned particle::init_count = 0;
vector<texture*> particle::tex_smoke;
texture* particle::tex_spray = 0;
vector<texture*> particle::tex_fire;
vector<texture*> particle::explosionbig;
vector<texture*> particle::explosionsml;
vector<texture*> particle::watersplashes;

#define NR_OF_SMOKE_TEXTURES 16
#define NR_OF_FIRE_TEXTURES 64

vector<float> particle::interpolate_func;

vector<Uint8> particle::make_2d_smoothed_noise_map(unsigned wh)
{
	vector<Uint8> tmp(wh * wh);
	for (unsigned i = 0; i < wh * wh; ++i)
		tmp[i] = (Uint8)(rand() % 256);
	for (unsigned i = 0; i < wh; ++i) {
		tmp[i] = 0;
		if (rand() % 2 == 0)
			tmp[wh + i] = 0;
		tmp[wh * i] = 0;
		if (rand() % 2 == 0)
			tmp[wh * i + 1] = 0;
		tmp[wh * i + wh - 1] = 0;
		if (rand() % 2 == 0)
			tmp[wh * i + wh - 2] = 0;
		tmp[(wh - 1) * wh + i] = 0;
		if (rand() % 2 == 0)
			tmp[(wh - 2) * wh + i] = 0;
	}
	vector<Uint8> tmp2(wh * wh);
	unsigned rmin = 255, rmax = 0;
	for (unsigned y = 0; y < wh; ++y) {
		unsigned y1 = (y + wh - 1) & (wh - 1);
		unsigned y2 = (y + 1) & (wh - 1);
		for (unsigned x = 0; x < wh; ++x) {
			unsigned x1 = (x + wh - 1) & (wh - 1);
			unsigned x2 = (x + 1) & (wh - 1);
			unsigned r = (unsigned(tmp[y1 * wh + x1])
				      + unsigned(tmp[y1 * wh + x2])
				      + unsigned(tmp[y2 * wh + x1])
				      + unsigned(tmp[y2 * wh + x2])) / 16
				+ (unsigned(tmp[y * wh + x1])
				   + unsigned(tmp[y * wh + x2])
				   + unsigned(tmp[y1 * wh + x])
				   + unsigned(tmp[y2 * wh + x])) / 8
				+ unsigned(tmp[y * wh +x]) / 4;
			Uint8 r2 = (Uint8)(r);
			tmp2[y * wh + x] = r2;
			if (r2 > rmax) rmax = r2;
			if (r2 < rmin) rmin = r2;
		}
	}
	for (unsigned y = 0; y < wh; ++y) {
		for (unsigned x = 0; x < wh; ++x) {
			unsigned r = tmp2[y * wh + x];
			tmp[y * wh + x] = (Uint8)((r - rmin) * 256 / (rmax - rmin + 1));
		}
	}
	return tmp;
}

unsigned particle::interpolate_2d_map(const vector<Uint8>& mp, unsigned res, unsigned x, unsigned y, unsigned res2)
{
	unsigned fac = res2 / res;
	unsigned xi = x / fac;
	unsigned yi = y / fac;
	unsigned x1 = xi & (res - 1);
	unsigned x2 = (xi + 1)  & (res - 1);
	unsigned y1 = yi & (res - 1);
	unsigned y2 = (yi + 1)  & (res - 1);
	unsigned dx = 256 * (x - xi * fac) / fac;
	unsigned dy = 256 * (y - yi * fac) / fac;
	float fa = interpolate_func[dx];
	float fb = interpolate_func[dy];
	float f0 = (1 - fa) * (1 - fb);
	float f1 = fa * (1 - fb);
	float f2 = (1 - fa) * fb;
	float f3 = fa * fb;
	return unsigned(f0 * mp[y1 * res + x1] + f1 * mp[y1 * res + x2] + f2 * mp[y2 * res + x1] + f3 * mp[y2 * res + x2]);
}

vector<Uint8> particle::make_2d_perlin_noise(unsigned wh, unsigned highestlevel)
{
	unsigned whlevel = 0;
	while (wh > unsigned(1 << whlevel))
		++whlevel;
	// prepare lookup maps
	unsigned levels = whlevel - highestlevel + 1;
	vector<vector<Uint8> > lookup_maps;
	lookup_maps.reserve(levels);
	for (unsigned i = 0; i < levels; ++i) {
		unsigned res = 1<<(highestlevel + i);
		lookup_maps.push_back(make_2d_smoothed_noise_map(res));
	}

	// generate result
	vector<Uint8> result(wh * wh);
	for (unsigned y = 0; y < wh; ++y) {
		for (unsigned x = 0; x < wh; ++x) {
			unsigned r = 0;
			for (unsigned i = 0; i < levels; ++i) {
				r += interpolate_2d_map(lookup_maps[i], 1 << (highestlevel + i), x, y, wh)
					* 65536 / (1 << (i + 1));
			}
			r /= 65536;
			if (r > 255) r = 510 - r;
			result[y * wh + x] = (Uint8)(r);
		}
	}
	return result;
}



vector<Uint8> particle::compute_fire_frame(unsigned wh, const vector<Uint8>& oldframe)
{
	vector<Uint8> result = oldframe;
	for (unsigned y = 0; y < wh-2; ++y) {
		for (unsigned x = 1; x < wh-1; ++x) {
			unsigned sum = 0;
			for (unsigned yy = y; yy <= y+2; ++yy) {
				for (unsigned xx = x-1; xx <= x+1; ++xx) {
					sum += unsigned(oldframe[yy*wh+xx]);
				}
			}
			unsigned r = rand() % 64;
			sum = (r > sum) ? 0 : sum - r;
			sum = (sum * 28) / 256;
			if (sum > 255) sum = 511 - sum;
			result[y * wh + x] = Uint8(sum);
		}
	}

	for (unsigned k = 0; k < 2; ++k) {
		for (unsigned j = 0; j < wh/4; ++j) {
			unsigned x = (j < wh*7/32) ? (rand() % (wh/2) + wh/4) : (rand() % (wh-2)) + 1;
			Uint8 c;
			if (rand() % 4 == 0)
				c = 0;
			else
				c = rand() % 55 + 200;
			result[(wh-1-k) * wh + x] = c;
		}
	}
	return result;
}




#include <fstream>
#include <sstream>

void particle::init(void)
{
	if (++init_count != 1) return;

	interpolate_func.resize(256);
	for (unsigned i = 0; i < 256; ++i)
		interpolate_func[i] = 0.5f - 0.5f * cos(i * M_PI / 256);
	
	// compute random smoke textures here.
	// just random noise with smoke color gradients and irregular outline
	// resolution 64x64, outline 8x8 scaled, smoke structure 8x8 or 16x16
	tex_smoke.resize(NR_OF_SMOKE_TEXTURES);
	vector<Uint8> smoketmp(64*64*2);
	for (unsigned i = 0; i < NR_OF_SMOKE_TEXTURES; ++i) {
		vector<Uint8> noise = make_2d_perlin_noise(64, 2);

/*
		ostringstream oss;
		oss << "argh" << i << ".pgm";
		ofstream osg(oss.str().c_str());
		osg << "P5\n" << 64 << " " << 64 << "\n255\n";
		osg.write((const char*)(&noise[0]), 64*64);
*/

		for (unsigned y = 0; y < 64; ++y) {
			for (unsigned x = 0; x < 64; ++x) {
				unsigned r = noise[y*64+x];
				smoketmp[2*(y*64+x)+0] = (Uint8)r;
				smoketmp[2*(y*64+x)+1] = (r < 64) ? 0 : r - 64;
			}
		}
		tex_smoke[i] = new texture(smoketmp, 64, 64, GL_LUMINANCE_ALPHA, GL_LINEAR_MIPMAP_LINEAR, GL_CLAMP_TO_EDGE);
	}

	// compute spray texture here
	for (unsigned y = 0; y < 64; ++y) {
		for (unsigned x = 0; x < 64; ++x) {
			smoketmp[(y*64+x)*2] = 255;
		}
	}
	tex_spray = new texture(smoketmp, 64, 64, GL_LUMINANCE_ALPHA, GL_LINEAR_MIPMAP_LINEAR, GL_CLAMP_TO_EDGE);

	// compute random fire textures here.
	tex_fire.resize(NR_OF_FIRE_TEXTURES);
#define FIRE_RES 64
	vector<color> firepal(256);
	color firepal_p[9] = {
		color(  0,   0,   0,   0),
		color(255, 128,  32,  16),
		color(255, 128,  32,  32),
		color(255,   0,   0,  64),
		color(255,  64,  32, 128),
		color(255, 160,  16, 160),		
		color(255, 255,   0, 192),
		color(255, 255,  64, 192),
		color(255, 255, 255, 255)
	};
	for (unsigned i = 1; i < 256; ++i) {
		unsigned j = i / 32;
		firepal[i] = color(firepal_p[j], firepal_p[j+1], float(i%32)/32);
	}
	firepal[0] = firepal_p[0];
	vector<Uint8> firetmp(FIRE_RES*FIRE_RES);
	for (unsigned i = 0; i < NR_OF_FIRE_TEXTURES * 2; ++i) {
		firetmp = compute_fire_frame(FIRE_RES, firetmp);
/*
		vector<Uint8> tmp2(firetmp.size() * 3);
		for (unsigned j = 0; j < firetmp.size(); ++j) {
			firepal[firetmp[j]].store_rgb(&tmp2[3*j]);
		}
		ostringstream oss;
		oss << "argh" << i << ".ppm";
		ofstream osg(oss.str().c_str());
		osg << "P6\n" << FIRE_RES << " " << FIRE_RES << "\n255\n";
		osg.write((const char*)(&tmp2[0]), FIRE_RES*FIRE_RES*3);
*/
	}
	for (unsigned i = 0; i < NR_OF_FIRE_TEXTURES; ++i) {
		vector<Uint8> tmp(firetmp.size() * 4);
		for (unsigned j = 0; j < firetmp.size() - 2 * FIRE_RES; ++j) {
			firepal[firetmp[j]].store_rgba(&tmp[4*(j + 2*FIRE_RES)]);
		}
		tex_fire[i] = new texture(tmp, FIRE_RES, FIRE_RES, GL_RGBA, GL_LINEAR, GL_CLAMP_TO_EDGE);

/*
		vector<Uint8> tmp2(firetmp.size() * 3);
		for (unsigned j = 0; j < firetmp.size(); ++j) {
			firepal[firetmp[j]].store_rgb(&tmp2[3*j]);
		}
		ostringstream oss;
		oss << "argh" << NR_OF_FIRE_TEXTURES * 2 + i << ".ppm";
		ofstream osg(oss.str().c_str());
		osg << "P6\n" << FIRE_RES << " " << FIRE_RES << "\n255\n";
		osg.write((const char*)(&tmp2[0]), FIRE_RES*FIRE_RES*3);
*/

		firetmp = compute_fire_frame(FIRE_RES, firetmp);
	}

	// read in explosions
#define EXPL_FRAMES 15
	explosionbig.resize(EXPL_FRAMES);
	for (unsigned i = 0; i < EXPL_FRAMES; ++i) {
		char tmp[20];
		sprintf(tmp, "exbg%04u.png", i+1);
		explosionbig[i] = new texture(get_texture_dir() + "explosion01/" + tmp, GL_LINEAR, GL_CLAMP_TO_EDGE);
	}
	explosionsml.resize(EXPL_FRAMES);
	for (unsigned i = 0; i < EXPL_FRAMES; ++i) {
		char tmp[20];
		sprintf(tmp, "exsm%04u.png", i+1);
		explosionsml[i] = new texture(get_texture_dir() + "explosion02/" + tmp, GL_LINEAR, GL_CLAMP_TO_EDGE);
	}

	// read in water splash images (maybe replace later with run time generated images of water particles)
	watersplashes.resize(3);
	watersplashes[0] = new texture(get_texture_dir() + "torpedo_expl_water_splash.png" , GL_LINEAR);
	watersplashes[1] = new texture(get_texture_dir() + "torpedo_expl_water_splash_1.png" , GL_LINEAR);
	watersplashes[2] = new texture(get_texture_dir() + "torpedo_expl_water_splash_2.png" , GL_LINEAR);
}



void particle::deinit(void)
{
	if (--init_count != 0) return;
	for (unsigned i = 0; i < tex_smoke.size(); ++i)
		delete tex_smoke[i];
	delete tex_spray;
	for (unsigned i = 0; i < tex_fire.size(); ++i)
		delete tex_fire[i];
	for (unsigned i = 0; i < explosionbig.size(); ++i)
		delete explosionbig[i];
	for (unsigned i = 0; i < explosionsml.size(); ++i)
		delete explosionsml[i];
	for (unsigned i = 0; i < watersplashes.size(); ++i)
		delete watersplashes[i];
}



void particle::simulate(game& gm, double delta_t)
{
	vector3 acc = get_acceleration();
	position += velocity * delta_t + acc * (delta_t * delta_t * 0.5);
	velocity += acc * delta_t;
	life -= delta_t/get_life_time();
	if (life < 0.0) life = 0.0;
}



void particle::display_all(const list<particle*>& pts, const vector3& viewpos, class game& gm)
{
	glDepthMask(GL_FALSE);
	matrix4 mv = matrix4::get_gl(GL_MODELVIEW_MATRIX);
	vector3 mvtrans = -mv.inverse().column(3);

	vector<particle_dist> pds;
	pds.reserve(pts.size());
	for (list<particle*>::const_iterator it = pts.begin(); it != pts.end(); ++it) {
		vector3 pp = (mvtrans + (*it)->get_pos() - viewpos);
		pds.push_back(particle_dist(*it, pp.square_length(), pp));
	}
	sort(pds.begin(), pds.end());

	glDisable(GL_LIGHTING);
	//glNormal3f(0, 0, 1);

	// draw particles, generate coordinates on the fly
	for (vector<particle_dist>::iterator it = pds.begin(); it != pds.end(); ++it) {
		const particle& part = *(it->pt);
		const vector3& z = -it->projpos;
		vector3 y = vector3(0, 0, 1);
		vector3 x = y.cross(z).normal();
		if (!part.is_z_up())//fixme
			y = z.cross(x).normal();
		double w2 = part.get_width()/2;
		double h = part.get_height();
		double hb, ht;
		if (part.tex_centered()) {
			ht = h * 0.5;
			hb = -ht;
		} else {
			ht = h;
			hb = 0;
		}
		vector3 pp = part.get_pos() - viewpos;
		vector3 coord;
		part.set_texture(gm);//not valid between glBegin and glEnd
		glBegin(GL_QUADS);
		coord = pp + x * -w2 + y * ht;
		glTexCoord2f(0, 0);
		glVertex3dv(&coord.x);
		coord = pp + x * -w2 + y * hb;
		glTexCoord2f(0, 1);
		glVertex3dv(&coord.x);
		coord = pp + x * w2 + y * hb;
		glTexCoord2f(1, 1);
		glVertex3dv(&coord.x);
		coord = pp + x * w2 + y * ht;
		glTexCoord2f(1, 0);
		glVertex3dv(&coord.x);
		glEnd();
	}

	glEnable(GL_LIGHTING);
	glDepthMask(GL_TRUE);
}



// smoke

smoke_particle::smoke_particle(const vector3& pos) : particle(pos), texnr(rand() % NR_OF_SMOKE_TEXTURES)
{
	velocity.x = -1; // wind test, wind from NE, speed ~1.4m/s
	velocity.y = -1;
	velocity.z = 4.0;	// m/s
}



vector3 smoke_particle::get_acceleration(void) const
{
	return vector3(0, 0, -3.0/get_life_time());
}



double smoke_particle::get_width(void) const
{
	// min/max size in meters
	return 2.0 * life + 50.0 * (1.0 - life);
}



double smoke_particle::get_height(void) const
{
	double h = get_width();
	if (life > 0.9)
		h *= (life - 0.8) * 10;
	return h;
}



void smoke_particle::set_texture(class game& gm) const
{
	glColor4f(0.5f, 0.5f, 0.5f, life);
	tex_smoke[texnr]->set_gl_texture();
}



double smoke_particle::get_life_time(void) const
{
	return 30.0; // seconds
}



double smoke_particle::get_produce_time(void)
{
	return 0.6; // seconds
}



smoke_particle_escort::smoke_particle_escort(const vector3& pos) : smoke_particle(pos)
{
}



double smoke_particle_escort::get_width(void) const
{
	return 2.0 * life + 25.0 * (1.0 - life);
}



double smoke_particle_escort::get_life_time(void) const
{
	return 15.0; // seconds
}



double smoke_particle_escort::get_produce_time(void)
{
	return 0.3; // seconds
}



// explosion(s)

explosion_particle::explosion_particle(const vector3& pos) : particle(pos)
{
	extype = 0; // rnd(1); //fixme
}



double explosion_particle::get_width(void) const
{
	return 20.0; //fixme: depends on type
}



double explosion_particle::get_height(void) const
{
	return 20.0; //fixme: depends on type
}



void explosion_particle::set_texture(class game& gm) const
{
	glColor4f(1, 1, 1, 1);
	unsigned f = unsigned(EXPL_FRAMES * (1.0 - life));
	if (f < 0 || f >= EXPL_FRAMES) f = EXPL_FRAMES-1;
	// switch on type, fixme
	explosionbig[f]->set_gl_texture();
}



double explosion_particle::get_life_time(void) const
{
	return 2.0; // seconds
}



// fire

fire_particle::fire_particle(const vector3& pos) : particle(pos)
{
}



void fire_particle::simulate(game& gm, double delta_t)
{
	float lf = get_life_time();
	float l = myfrac(life * lf);
	if (l - lf * delta_t <= 0) {
		gm.spawn_particle(new smoke_particle(position));
	}
	particle::simulate(gm, delta_t);
	if (life <= 0.0) {
		life += 1.0;
	}
}



double fire_particle::get_width(void) const
{
	return 20.0; //fixme: depends on type
}



double fire_particle::get_height(void) const
{
	return 20.0; //fixme: depends on type
}



void fire_particle::set_texture(class game& gm) const
{
	glColor4f(1, 1, 1, 1);
	unsigned i = unsigned(tex_fire.size() * (1.0 - life));
	tex_fire[i]->set_gl_texture();
}



double fire_particle::get_life_time(void) const
{
	return 4.0; // seconds
}



// spray

spray_particle::spray_particle(const vector3& pos, const vector3& velo) : particle(pos, velo)
{
}



double spray_particle::get_width(void) const
{
	return (1.0 - life) * 6.0 + 2.0;
}



double spray_particle::get_height(void) const
{
	return get_width();
}



void spray_particle::set_texture(class game& gm) const
{
	glColor4f(1, 1, 1, life);
	tex_spray->set_gl_texture();
}



double spray_particle::get_life_time(void) const
{
	return 4.0; // seconds
}



// water splashes

torpedo_water_splash_particle::torpedo_water_splash_particle(const vector3& pos) : particle(pos)
{
}



double torpedo_water_splash_particle::get_width(void) const
{
	return 40.0;
}



double torpedo_water_splash_particle::get_height(void) const
{
	const double trd = 1.0/3.0;
	// 1/3 rise, 2/3 decline
	double x = (life > trd) ? 1.5 * M_PI * (life - 2 * trd) : life * 0.75 * M_PI;
	return 80.0 * sin(x);
}



void torpedo_water_splash_particle::set_texture(game& gm) const
{
	glColor4f(1, 1, 1, 0.5f + float(life) * 0.25f);
	watersplashes[2]->set_gl_texture();
}



double torpedo_water_splash_particle::get_life_time(void) const
{
	return 3.0;
}



gun_shell_water_splash_particle::gun_shell_water_splash_particle(const vector3& pos) : particle(pos)
{
}



double gun_shell_water_splash_particle::get_width(void) const
{
	return 10.0;
}



double gun_shell_water_splash_particle::get_height(void) const
{
	const double trd = 1.0/3.0;
	// 1/3 rise, 2/3 decline
	double x = (life > trd) ? 1.5 * M_PI * (life - 2 * trd) : life * 0.75 * M_PI;
	return 5.0 * sin(x);
}



void gun_shell_water_splash_particle::set_texture(game& gm) const
{
	glColor4f(1, 1, 1, 0.5f + float(life) * 0.25f);
	watersplashes[0]->set_gl_texture();
}



double gun_shell_water_splash_particle::get_life_time(void) const
{
	return 3.0;
}



depth_charge_water_splash_particle::depth_charge_water_splash_particle(const vector3& pos) : particle(pos)
{
}



double depth_charge_water_splash_particle::get_width(void) const
{
	return 40.0;
}



double depth_charge_water_splash_particle::get_height(void) const
{
	const double trd = 1.0/3.0;
	// 1/3 rise, 2/3 decline
	double x = (life > trd) ? 1.5 * M_PI * (life - 2 * trd) : life * 0.75 * M_PI;
	return 60.0 * sin(x);
}



void depth_charge_water_splash_particle::set_texture(game& gm) const
{
	glColor4f(1, 1, 1, 0.5f + float(life) * 0.25f);
	watersplashes[1]->set_gl_texture();
}



double depth_charge_water_splash_particle::get_life_time(void) const
{
	return 3.0;
}
