// particle (C)+(W) 2004 Thorsten Jordan

#include "particle.h"
#include "game.h"
#include "texture.h"

#include "global_data.h" //for smoke texture, fixme


void particle::display_all(const list<particle*>& pts, const vector3& viewpos, class game& gm)
{
	glDepthMask(GL_FALSE);
	glDisable(GL_CULL_FACE);
	matrix4 mv = matrix4::get_gl(GL_MODELVIEW_MATRIX);
	vector3 mvtrans = -mv.inverse().column(3);

	vector<particle_dist> pds;
	pds.reserve(pts.size());
	for (list<particle*>::const_iterator it = pts.begin(); it != pts.end(); ++it) {
		vector3 pp = /*mv * */(mvtrans + (*it)->pos - viewpos);
		pds.push_back(particle_dist(*it, pp.square_length(), pp));
	}
	sort(pds.begin(), pds.end());

	// generate arrays
	vector<vector3> coords(4*pds.size());
	vector<vector2f> texcs(4*pds.size());
	for (unsigned i = 0; i < pds.size(); ++i) {
		const particle& part = *(pds[i].pt);
		const vector3& z = -pds[i].projpos;
		vector3 y = vector3(0, 0, 1);
		vector3 x = y.cross(z).normal();
//fixme: something here is not right
//		if (!part.is_z_up())
			y = z.cross(x).normal();
		double w2 = part.get_width()/2;
		double h2 = part.get_height()/2;
		vector3 pp = part.pos - viewpos;
		coords[4*i+0] = pp + x * -w2 + y * h2;
		coords[4*i+1] = pp + x * -w2 + y * -h2;
		coords[4*i+2] = pp + x * w2 + y * -h2;
		coords[4*i+3] = pp + x * w2 + y * h2;
		texcs[4*i+1].y = texcs[4*i+2].y = texcs[4*i+2].x = texcs[4*i+3].x = 1.0f;
	}

	// draw arrays
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_DOUBLE, sizeof(vector3), &coords[0].x);
	glClientActiveTexture(GL_TEXTURE0);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, sizeof(vector2f), &texcs[0]);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glBegin(GL_QUADS);
	for (unsigned i = 0; i < pds.size(); ++i) {
		pds[i].pt->set_texture(gm);
		for (unsigned j = 0; j < 4; ++j)
			glArrayElement(4 * i + j);
	}
	glEnd();
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glEnable(GL_CULL_FACE);//fixme testing
	glDepthMask(GL_TRUE);
}




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
	smoke->set_gl_texture();
}



double smoke_particle::get_produce_time(void)
{
	return SMOKE_PARTICLE_PRODUCE_TIME;
}
