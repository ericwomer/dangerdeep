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

class water_splash : public sea_object
{
public:
	enum water_splash_type { torpedo, gun_shell, depth_charge };

private:
	list<water_splash_element*> water_splashes;
	void init ( water_splash_type type );
	
protected:
	water_splash();
	water_splash& operator=(const water_splash& other);
	water_splash(const water_splash& other);

public:
	water_splash ( const vector3& position, water_splash_type type );
	virtual ~water_splash ();
	virtual float surface_visibility(const vector2& watcher) const;
	virtual void simulate ( class game& gm, double delta_time );
	virtual void display () const;
};

#endif /* WATER_SPLASH_H */
