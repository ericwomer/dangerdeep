// water splash
// subsim (C)+(W) Markus Petermann. SEE LICENSE

#include <GL/gl.h>
#include "sea_object.h"
#include "texture.h"
#include "water_splash.h"
#include "global_data.h"
#include "model.h"
#include "system.h"
#include "game.h"

#define WATER_SPLASH_RISE_TIME		1.0f
#define WATER_SPLASH_DECLINE_TIME	2.0f

water_splash::water_splash ( const vector3& position, water_splash_type type ) :
	sea_object (), type ( type )
{
	this->position = position;
	this->position.z - 4.0f;
	init ();
}

void water_splash::init ()
{
	switch ( type )
    {
		case torpedo:
			h_peak = 80.0f;
			w = 30.0f;
			tex = torp_expl_water_splash;
			break;
		case gun_shell:
			h_peak = 30.0f;
			w = 6.0f;
			tex = torp_expl_water_splash;
			break;
		case depth_charge:
			h_peak = 10.0f;
			w = 10.0f;
			tex = torp_expl_water_splash;
			break;
		default:
			h_peak = 10.0f;
			w = 1.0f;
			tex = torp_expl_water_splash;
			break;
    }
}

void water_splash::simulate ( class game& gm, double delta_time )
{
	t += delta_time;

	// The water splash rises within a second.
	if ( t < WATER_SPLASH_RISE_TIME )
	{
		h = h_peak * sin ( M_PI * t / ( 2.0f * WATER_SPLASH_RISE_TIME ) );
	}
#ifdef OLD
	else if ( t >= WATER_SPLASH_RISE_TIME )
	{
		h = h_peak * cos ( M_PI * ( t - WATER_SPLASH_RISE_TIME ) / ( 2.0f * WATER_SPLASH_DECLINE_TIME ) );

		if ( h <= 0.0f )
			kill ();
	}
#endif
}

void water_splash::display () const
{
	glColor3f ( 0, 0, 0 );
	glBindTexture ( GL_TEXTURE_2D, tex->get_opengl_name () );
	glBegin ( GL_QUADS );
	/*
	glTexCoord2f ( 0.0f, 1.0f );
	glVertex3f   ( -w, 0.0f, 0.0f );
	glTexCoord2f ( 0.0f, 0.0f );
	glVertex3f   ( -w, 0.0f, h );
	glTexCoord2f ( 1.0f, 0.0f );
	glVertex3f   (  w, 0.0f, h );
	glTexCoord2f ( 1.0f, 1.0f );
	glVertex3f   (  w, 0.0f, 0.0f );
	*/
	glTexCoord2f ( 0.0f, 1.0f );
	glVertex3f   ( 0.0f, -w, 0.0f );
	glTexCoord2f ( 0.0f, 0.0f );
	glVertex3f   ( 0.0f, -w, h );
	glTexCoord2f ( 1.0f, 0.0f );
	glVertex3f   ( 0.0f,  w, h );
	glTexCoord2f ( 1.0f, 1.0f );
	glVertex3f   ( 0.0f,  w, 0.0f );
	glEnd ();
}

float water_splash::surface_visibility(const vector2& watcher) const
{
	return 1.0f;
}
