// An nautical angle (C)+(W) 2003 Markus Petermann

#ifndef WATER_SPLASH_H
#define WATER_SPLASH_H

class water_splash_element
{
protected:
	// Used texture.
	texture* tex;
	// Height of splash.
	double h;
	// Width of splash.
	double w;
	// Peak height of splash.
	double h_peak;
	// Rise time.
	double rise_time;
	// Decline time.
	double decline_time;
	double t;
	bool finished;

public:
	water_splash_element ();
	water_splash_element ( texture* tex, double h_peak, double w,
		double rise_time, double decline_time );
	virtual ~water_splash_element ();
	virtual void simulate ( class game& gm, double delta_time );
	virtual void display () const;
	virtual bool is_finished () const { return ( finished == true ); }
};

typedef list<water_splash_element*> water_splash_element_list;
typedef list<water_splash_element*>::iterator water_splash_element_list_iterator;
typedef list<water_splash_element*>::const_iterator water_splash_element_list_const_iterator;

class water_splash : public sea_object
{
	water_splash_element_list water_splashes;

protected:
	water_splash();
	water_splash& operator=(const water_splash& other);
	water_splash(const water_splash& other);
	virtual void spawn_water_splash_element ( water_splash_element* new_element )
	{ water_splashes.push_back ( new_element ); }

public:
	water_splash ( const vector3& position );
	virtual ~water_splash ();
	virtual float surface_visibility(const vector2& watcher) const;
	virtual void simulate ( class game& gm, double delta_time );
	virtual void display () const;
};

class torpedo_water_splash : public water_splash
{
	void init ();

protected:
	torpedo_water_splash ();
	torpedo_water_splash& operator= ( const torpedo_water_splash& other );
	torpedo_water_splash ( const torpedo_water_splash& other );

public:
	torpedo_water_splash ( const vector3& position );
	virtual ~torpedo_water_splash () {};
};

class gun_shell_water_splash : public water_splash
{
	void init ();

protected:
	gun_shell_water_splash ();
	gun_shell_water_splash& operator= ( const gun_shell_water_splash& other );
	gun_shell_water_splash ( const gun_shell_water_splash& other );

public:
	gun_shell_water_splash ( const vector3& position );
	virtual ~gun_shell_water_splash () {};
};

class depth_charge_water_splash : public water_splash
{
	void init ();

protected:
	depth_charge_water_splash ();
	depth_charge_water_splash& operator= ( const depth_charge_water_splash& other );
	depth_charge_water_splash ( const depth_charge_water_splash& other );

public:
	depth_charge_water_splash ( const vector3& position );
	virtual ~depth_charge_water_splash () {};
};

#endif /* WATER_SPLASH_H */
