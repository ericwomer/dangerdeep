// particle (C)+(W) 2004 Thorsten Jordan

#include "particle.h"
#include "game.h"
#include "texture.h"

#include "global_data.h" //for smoke texture, fixme

unsigned particle::init_count = 0;
texture* particle::tex_smoke = 0;
vector<texture*> particle::explosionbig;
vector<texture*> particle::explosionsml;

void particle::init(void)
{
	if (++init_count != 1) return;
	tex_smoke = new texture(get_texture_dir() + "smoke.png" , GL_LINEAR, GL_CLAMP_TO_EDGE);
	// read in explosions
#define EXPL_FRAMES 15
	explosionbig.resize(EXPL_FRAMES);
	for (unsigned i = 0; i < EXPL_FRAMES; ++i) {
		char tmp[20];
		sprintf(tmp, "exbg%04u.png", i+1);
		explosionbig[i] = new texture(get_texture_dir() + "explosion01/" + tmp, GL_LINEAR, GL_CLAMP_TO_EDGE, false);
	}
	explosionsml.resize(EXPL_FRAMES);
	for (unsigned i = 0; i < EXPL_FRAMES; ++i) {
		char tmp[20];
		sprintf(tmp, "exsm%04u.png", i+1);
		explosionsml[i] = new texture(get_texture_dir() + "explosion02/" + tmp, GL_LINEAR, GL_CLAMP_TO_EDGE, false);
	}
}



void particle::deinit(void)
{
	if (--init_count != 0) return;
	delete tex_smoke;
	for (unsigned i = 0; i < explosionbig.size(); ++i)
		delete explosionbig[i];
	for (unsigned i = 0; i < explosionsml.size(); ++i)
		delete explosionsml[i];
}



void particle::display_all(const list<particle*>& pts, const vector3& viewpos, class game& gm)
{
	glDepthMask(GL_FALSE);
	matrix4 mv = matrix4::get_gl(GL_MODELVIEW_MATRIX);
	vector3 mvtrans = -mv.inverse().column(3);

	vector<particle_dist> pds;
	pds.reserve(pts.size());
	for (list<particle*>::const_iterator it = pts.begin(); it != pts.end(); ++it) {
		vector3 pp = (mvtrans + (*it)->pos - viewpos);
		pds.push_back(particle_dist(*it, pp.square_length(), pp));
	}
	sort(pds.begin(), pds.end());

	glNormal3f(0, 0, 1);

	// generate coordinates
	vector<vector3> coords(4*pds.size());
	for (unsigned i = 0; i < pds.size(); ++i) {
		const particle& part = *(pds[i].pt);
		const vector3& z = -pds[i].projpos;
		vector3 y = vector3(0, 0, 1);
		vector3 x = y.cross(z).normal();
//		if (!part.is_z_up())//fixme
			y = z.cross(x).normal();
		double w2 = part.get_width()/2;
		double h2 = part.get_height()/2;
		vector3 pp = part.pos - viewpos;
		coords[4*i+0] = pp + x * -w2 + y * h2;
		coords[4*i+1] = pp + x * -w2 + y * -h2;
		coords[4*i+2] = pp + x * w2 + y * -h2;
		coords[4*i+3] = pp + x * w2 + y * h2;
	}

	// draw arrays
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_DOUBLE, sizeof(vector3), &coords[0].x);
	glClientActiveTexture(GL_TEXTURE0);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	
	vector2f texcs[4] = { vector2f(0, 0), vector2f(0, 1), vector2f(1, 1), vector2f(1, 0) };
	for (unsigned i = 0; i < pds.size(); ++i) {
		pds[i].pt->set_texture(gm);//not valid between glBegin and glEnd
		glBegin(GL_QUADS);
		for (unsigned j = 0; j < 4; ++j) {
			glTexCoord2fv(&texcs[j].x);
			glArrayElement(4 * i + j);
		}
		glEnd();
	}
	glDisableClientState(GL_VERTEX_ARRAY);

	glDepthMask(GL_TRUE);
}



// smoke

#define SMOKE_PARTICLE_LIFE_TIME 120.0	// seconds
#define SMOKE_PARTICLE_ASCEND_SPEED 1.0 // m/s

#define SMOKE_PARTICLE_SIZE_MIN 2.0	// meters
#define SMOKE_PARTICLE_SIZE_MAX 30.0	// meters

#define SMOKE_PARTICLE_PRODUCE_TIME 5.0

void smoke_particle::simulate(game& gm, double delta_t)
{
	pos.z += SMOKE_PARTICLE_ASCEND_SPEED * delta_t;
	live -= delta_t/SMOKE_PARTICLE_LIFE_TIME;
}



double smoke_particle::get_width(void) const
{
	return SMOKE_PARTICLE_SIZE_MIN * live + SMOKE_PARTICLE_SIZE_MAX * (1.0f - live);
}



double smoke_particle::get_height(void) const
{
	return 1.5 * SMOKE_PARTICLE_ASCEND_SPEED * SMOKE_PARTICLE_PRODUCE_TIME;
}



void smoke_particle::set_texture(class game& gm) const
{
	glColor4f(0.5f, 0.5f, 0.5f, live);
	tex_smoke->set_gl_texture();
}



double smoke_particle::get_produce_time(void)
{
	return SMOKE_PARTICLE_PRODUCE_TIME;
}



// explosion(s)

#define EXPLOSION_PARTICLE_LIFE_TIME 2.0

explosion_particle::explosion_particle(const vector3& pos_) : particle(pos_)
{
	extype = 0; // rnd(1); //fixme
}



void explosion_particle::simulate(game& gm, double delta_t)
{
	live -= delta_t/EXPLOSION_PARTICLE_LIFE_TIME;
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
	unsigned f = EXPL_FRAMES * (1.0f - live);
	if (f < 0 || f >= EXPL_FRAMES) f = EXPL_FRAMES-1;
	// switch on type, fixme
	explosionbig[f]->set_gl_texture();
}
