// Object to create and display the number and tonnage of sunk ships.
// subsim (C)+(W) Markus Petermann. SEE LICENSE

#include "user_display.h"

#ifndef SHIPS_SUNK_DISPLAY_H
#define SHIPS_SUNK_DISPLAY_H

class ship;
class model;

class ships_sunk_display : public user_display
{
	class destroyed_object
	{
		// This model pointer is needed for a symbolical display of the target.
		const model* mdl;
		unsigned tonnage;
		string class_name;

	public:
		destroyed_object () : mdl ( 0 ), tonnage ( 0 ) {};
		destroyed_object ( const model* mdl, unsigned tonnage,
			const string& class_name ) : mdl ( mdl ), tonnage ( tonnage ),
			class_name ( class_name ) {}
		destroyed_object ( const destroyed_object& d ) { copy ( d ); }
		virtual ~destroyed_object () { mdl = 0; }
		virtual void display () const { mdl->display (); }
		virtual void copy ( const destroyed_object& d )
		{
			this->mdl = d.mdl;
			this->tonnage = d.tonnage;
			this->class_name = d.class_name;
		}
		virtual destroyed_object& operator = ( const destroyed_object& d )
		{
			if ( this != &d )
				copy ( d );

			return *this;
		}
		const model* get_model () const { return mdl; }
		unsigned get_tonnage () const { return tonnage; }
		string get_class_name () const { return class_name; }
	};

	typedef map<unsigned, destroyed_object> destroyed_object_map;
	typedef destroyed_object_map::iterator destroyed_object_map_it;
	typedef destroyed_object_map::const_iterator destroyed_object_map_c_it;

	destroyed_object_map dom;
	// The ID of the first destroyed object on the current screen.
	int first_displayed_object;
	
public:
	ships_sunk_display ();
	virtual ~ships_sunk_display ();

	virtual void add_sunk_ship ( const ship* so );

	virtual void display ( class game& gm );
	virtual void check_key ( int keycode, class game& gm );
	virtual void check_mouse ( int x, int y, int mb );
	virtual void next_page ();
	virtual void previous_page ();
};

#endif /* SHIPS_SUNK_DISPLAY_H */
