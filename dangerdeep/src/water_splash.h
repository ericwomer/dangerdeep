// An nautical angle (C)+(W) 2003 Markus Petermann

#ifndef WATER_SPLASH_H
#define WATER_SPLASH_H

class water_splash : public sea_object
{
public:
	enum water_splash_type { torpedo, gun_shell, depth_charge };

private:
	// Type of splash.
	water_splash_type type;
	// The water splash is a time depended movement.
	double t;
	// Peak height of splash.
	double h_peak;
	// Width of splash.
	double w;
	// Height of splash.
	double h;
	// Used textures.
	texture* tex;

	void init ();
	
protected:
	water_splash();
	water_splash& operator=(const water_splash& other);
	water_splash(const water_splash& other);

public:
	water_splash ( const vector3& position, water_splash_type type );
	virtual ~water_splash () {};
	virtual float surface_visibility(const vector2& watcher) const;
	virtual void simulate ( class game& gm, double delta_time );
	virtual void display () const;
};

#endif /* WATER_SPLASH_H */
