// particle (C)+(W) 2004 Thorsten Jordan

#include "particle.h"
#include "game.h"
#include "texture.h"

#include "global_data.h" //for smoke texture, fixme

unsigned particle::init_count = 0;
vector<texture*> particle::tex_smoke;
vector<texture*> particle::explosionbig;
vector<texture*> particle::explosionsml;

#define NR_OF_SMOKE_TEXTURES 16

vector<float> particle::interpolate_func;

vector<unsigned char> particle::make_2d_smoothed_noise_map(unsigned wh)
{
	vector<unsigned char> tmp(wh * wh);
	for (unsigned i = 0; i < wh * wh; ++i)
		tmp[i] = (unsigned char)(rand() % 256);
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
	vector<unsigned char> tmp2(wh * wh);
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
			unsigned char r2 = (unsigned char)(r);
			tmp2[y * wh + x] = r2;
			if (r2 > rmax) rmax = r2;
			if (r2 < rmin) rmin = r2;
		}
	}
	for (unsigned y = 0; y < wh; ++y) {
		for (unsigned x = 0; x < wh; ++x) {
			unsigned r = tmp2[y * wh + x];
			tmp[y * wh + x] = (unsigned char)((r - rmin) * 256 / (rmax - rmin + 1));
		}
	}
	return tmp;
}

unsigned particle::interpolate_2d_map(const vector<unsigned char>& mp, unsigned res, unsigned x, unsigned y, unsigned res2)
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

vector<unsigned char> particle::make_2d_perlin_noise(unsigned wh, unsigned highestlevel)
{
	unsigned whlevel = 0;
	while (wh > unsigned(1 << whlevel))
		++whlevel;
	// prepare lookup maps
	unsigned levels = whlevel - highestlevel + 1;
	vector<vector<unsigned char> > lookup_maps;
	lookup_maps.reserve(levels);
	for (unsigned i = 0; i < levels; ++i) {
		unsigned res = 1<<(highestlevel + i);
		lookup_maps.push_back(make_2d_smoothed_noise_map(res));
	}
	// generate result
	vector<unsigned char> result(wh * wh);
	for (unsigned y = 0; y < wh; ++y) {
		for (unsigned x = 0; x < wh; ++x) {
			unsigned r = 0;
			for (unsigned i = 0; i < levels; ++i) {
				r += interpolate_2d_map(lookup_maps[i], 1 << (highestlevel + i), x, y, wh)
					* 65536 / (1 << (i + 1));
			}
			r /= 65536;
			if (r > 255) r = 510 - r;
			result[y * wh + x] = (unsigned char)(r);
		}
	}
	return result;
}



/*
#include <fstream>
#include <sstream>
*/
void particle::init(void)
{
	if (++init_count != 1) return;

	interpolate_func.resize(256);
	for (unsigned i = 0; i < 256; ++i)
		interpolate_func[i] = 0.5f - 0.5f * cos(i * M_PI / 256);
	
	// compute random smoke textures here.
	// just random noise with smoke color gradients and irregular outline
	// resolution 64x64, outline 8x8 scaled, smoke structure 8x8 or 16x16
	//fixme: we need some sort of plasma here, clouds too, etc. make a class for it
	//perlin noise for smoke particles would also be good...
	tex_smoke.resize(NR_OF_SMOKE_TEXTURES);
	vector<unsigned char> smoketmp(64*64*2);
	for (unsigned i = 0; i < NR_OF_SMOKE_TEXTURES; ++i) {
		vector<unsigned char> noise = make_2d_perlin_noise(64, 3);

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
				smoketmp[2*(y*64+x)+0] = (unsigned char)r;
				smoketmp[2*(y*64+x)+1] = (r < 64) ? 0 : r - 64;
			}
		}
		tex_smoke[i] = new texture(&smoketmp[0], 64, 64, GL_LUMINANCE_ALPHA, GL_LINEAR_MIPMAP_LINEAR, GL_REPEAT, false);
	}
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
	for (unsigned i = 0; i < tex_smoke.size(); ++i)
		delete tex_smoke[i];
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

smoke_particle::smoke_particle(const vector3& pos_) : particle(pos_), texnr(rand() % NR_OF_SMOKE_TEXTURES)
{


}

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
	return 2.0 * SMOKE_PARTICLE_ASCEND_SPEED * SMOKE_PARTICLE_PRODUCE_TIME;
}



void smoke_particle::set_texture(class game& gm) const
{
	glColor4f(0.5f, 0.5f, 0.5f, live);
	tex_smoke[texnr]->set_gl_texture();
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
	unsigned f = unsigned(EXPL_FRAMES * (1.0f - live));
	if (f < 0 || f >= EXPL_FRAMES) f = EXPL_FRAMES-1;
	// switch on type, fixme
	explosionbig[f]->set_gl_texture();
}
