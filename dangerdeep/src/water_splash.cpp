// water splash
// subsim (C)+(W) Markus Petermann. SEE LICENSE

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>
#else
#include <GL/gl.h>
#endif
#include "sea_object.h"
#include "texture.h"
#include "water_splash.h"
#include "global_data.h"
#include "model.h"
#include "system.h"
#include "game.h"



//
// Class water_splash
//
water_splash::water_splash () : sea_object ()
{}

water_splash::water_splash ( const vector3& position ) :
	sea_object ()
{
	this->position = position;
}

water_splash::~water_splash ()
{
	for ( water_splash_element_list_iterator it = water_splashes.begin ();
		it != water_splashes.end (); it ++ )
	{
		delete (*it);
	}
}

void water_splash::simulate ( class game& gm, double delta_time )
{
	for ( water_splash_element_list_iterator it = water_splashes.begin ();
		it != water_splashes.end (); )
	{
		water_splash_element_list_iterator it2 = it++;
		if ( !(*it2)->is_finished () )
		{
			(*it2)->simulate ( gm, delta_time );
		}
		else
		{
			delete (*it2);
			water_splashes.erase ( it2 );
		}
	}

	// When there are no active splashes left, mark the actual water_splash
	// object dead. It is removed later.
	if ( !water_splashes.size () )
		kill ();
}

void water_splash::display () const
{
	for ( water_splash_element_list_const_iterator it = water_splashes.begin ();
		it != water_splashes.end (); it ++ )
	{
		(*it)->display ();
	}
}

float water_splash::surface_visibility(const vector2& watcher) const
{
	return 100.0f;	// square meters... test hack
}



//
// Class water_splash::water_splash_element
//
water_splash_element::water_splash_element () : tex ( 0 )
{}

water_splash_element::water_splash_element ( texture* tex,
	double h_peak, double w, double rise_time, double decline_time ) :
	tex ( tex ), h_peak ( h_peak ), w ( w ), rise_time ( rise_time ),
	decline_time ( decline_time ), t ( 0.0f ), finished ( false )
{}

water_splash_element::~water_splash_element ()
{
	tex = 0;
}

void water_splash_element::simulate ( class game& gm, double delta_time )
{
	t += delta_time;
	if ( t >= 0.0f && t < rise_time )
		h = h_peak * sin ( M_PI * t / ( 2.0f * rise_time ) );
	else if ( t >= rise_time && t < ( rise_time + decline_time ) )
		h = h_peak * cos ( M_PI * ( t - rise_time ) / ( 2.0f * decline_time ) );
	else
		h = 0.0f;

	if ( t >= ( rise_time + decline_time ) )
		finished = true;
}

void water_splash_element::display () const
{
	if ( tex )
	{
		glColor4f ( 1.0f, 1.0f, 1.0f, 0.75f );
		glBindTexture ( GL_TEXTURE_2D, tex->get_opengl_name () );
		glBegin ( GL_QUADS );
		glTexCoord2f ( 0.0f, 1.0f );
		glVertex3f   ( 0.0f, -w, 0.0f );
		glTexCoord2f ( 0.0f, 0.0f );
		glVertex3f   ( 0.0f, -w, h );
		glTexCoord2f ( 1.0f, 0.0f );
		glVertex3f   ( 0.0f,  w, h );
		glTexCoord2f ( 1.0f, 1.0f );
		glVertex3f   ( 0.0f,  w, 0.0f );
		glEnd ();
		glColor3f ( 1.0f, 1.0f, 1.0f );
	}
}



//
// Class torpedo_water_splash
//
torpedo_water_splash::torpedo_water_splash ( const vector3& position ) :
	water_splash ( position )
{
	init ();
}

void torpedo_water_splash::init ()
{
	spawn_water_splash_element ( new water_splash_element (
		torp_expl_water_splash[1], 80.0f, 30.0f, 1.0f, 2.0f ) );
	spawn_water_splash_element ( new water_splash_element (
		torp_expl_water_splash[2], 80.0f, 40.0f, 1.0f, 1.5f ) );
}



//
// Class gun_shell_water_splash
//
gun_shell_water_splash::gun_shell_water_splash ( const vector3& position ) :
	water_splash ( position )
{
	init ();
}

void gun_shell_water_splash::init ()
{
	spawn_water_splash_element ( new water_splash_element (
		torp_expl_water_splash[0], 10.0f, 1.0f, 1.0f, 2.0f ) );
}



//
// Class depth_charge_water_splash
//
depth_charge_water_splash::depth_charge_water_splash ( const vector3& position ) :
	water_splash ( position )
{
	init ();
}

void depth_charge_water_splash::init ()
{
	spawn_water_splash_element ( new water_splash_element (
		torp_expl_water_splash[2], 20.0f, 40.0f, 2.0f, 2.0f ) );
}
