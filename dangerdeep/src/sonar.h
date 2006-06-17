/*
Danger from the Deep - Open source submarine simulation
Copyright (C) 2003-2006  Thorsten Jordan, Luis Barrancos and others.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

// Sonar simulation
// subsim (C) + (W). See LICENSE

#include "vector3.h"
#include "angle.h"
#include <vector>

#ifndef SONAR_H
#define SONAR_H

enum shipclass
{
	NONE = -1,
	WARSHIP = 0,
	ESCORT,
	MERCHANT,
	SUBMARINE,
	TORPEDO,
	NR_OF_SHIP_CLASSES
};

struct sonar_contact
{
	vector2 pos;
	shipclass type;
	sonar_contact(const vector2& p, shipclass t) : pos(p), type(t) {}
};



///\brief This class groups all data for an underwater noise source
struct noise_signature
{
	static const unsigned NR_OF_SONAR_FREQUENCY_BANDS = 4;

	// transformation from dB to linear noise needs a base.
	// base^dB_val = real noise, dB = 10 * log10(real) -> dB/10 = log10(real)
	// -> 10^(dB/10) = real -> (10^0.1)^dB = real,  10^0.1 = dB_base
	// so dB_base^dB = real
	static const double dB_base = 1.25892541179;

	// limits of frequency bands in Hertz
	static const double frequency_band_lower_limit[NR_OF_SONAR_FREQUENCY_BANDS];
	static const double frequency_band_upper_limit[NR_OF_SONAR_FREQUENCY_BANDS];

	// noise signatures for ship classes. later stored per ship class,
	// and read from the spec file. so this is only a temporary statement.
	static const double typical_noise_signature[NR_OF_SHIP_CLASSES][NR_OF_SONAR_FREQUENCY_BANDS];

	// mix factors for total strength
	static const double frequency_band_strength_factor[NR_OF_SONAR_FREQUENCY_BANDS];

	// background ("ambient") noise strength for each frequency band, in dB
	static const double background_noise[NR_OF_SONAR_FREQUENCY_BANDS];
	// factor for sea state dependant noise, values here are for max. wave heights, in dB
	static const double seastate_factor[NR_OF_SONAR_FREQUENCY_BANDS];
	// factor for noise absorption of sea water, in dB
	static const double noise_absorption[NR_OF_SONAR_FREQUENCY_BANDS];
	// factor for wave interference in shallow water ( < 250m, 125m in Mediterr.), in dB
	//static double wave_interference[NR_OF_SONAR_FREQUENCY_BANDS] = { 10, 8, 4, 2 };
	// typical frequencies for angle related strength factor
	static const double typical_frequency[NR_OF_SONAR_FREQUENCY_BANDS];

	// additional extra noise constant for cavitation, when running at full/flank speed, in dB
	static const double cavitation_noise = 2;

	struct band_noise_data
	{
		double basic_noise_level;	// in dB    try some values, maybe 10
		double speed_factor;		// in dB    0.541 per m/s   (in theory per throttle, not speed...)
	};

	band_noise_data band_data[NR_OF_SONAR_FREQUENCY_BANDS];

	static double dB_to_absolute(double dB) {
		return (dB < 0) ? 0.0 : pow(dB_base, dB);
	}

	static double absolute_to_dB(double a) {
		return (a < 1.0) ? 0.0 : 10.0*log10(a);
	}

	///\brief returns background noise (ambient noise) of environment, flat, not in dB
	/** @param	band		noise band number
	    @param	seastate	roughness of sea (1.0 = highest storm, 0.2=normal)
	*/
	// fixme: seastate is stored in class game... flow of information is unclear
	static double compute_ambient_noise_strength(unsigned band, double seastate = 0.2);

	///\brief returns total noise of source (background + artificial noise), flat, not in dB
	/** @param	band		noise band number
	    @param	distance	distance to source in meters
	    @param	speed		speed of source in m/s
	    @param	cavitation	wether target causes caviation
	    @param	seastate	roughness of sea (1.0 = highest storm, 0.2=normal)
	*/
	double compute_signal_strength(unsigned band, double distance, double speed,
				       bool caviation = false) const;

	///\brief compute medium total strength of noise from all frequency bands, flat, not in dB
	/** @param	strengths	strengths for each frequency band (in dB)
	 */
	static double compute_total_noise_strength(const std::vector<double>& strengths);

	///\brief compute shipclass of signal (can be none)
	/** @param	strengths	strengths for each frequency band (in dB)
	 */
	static shipclass determine_shipclass_by_signal(const std::vector<double>& strengths);
};

// move to a GHG class later, fixme
double compute_signal_strength_GHG(angle signal_angle, double frequency, angle apparatus_angle);

class sonar_operator_simulation
{
 protected:
	enum states {
		find_growing_signal,
		find_upper_limit,
		find_lower_limit,
		track_signal
	};
	states state;
	angle current_angle;
	angle turn_speed;
	double current_signal_strength;
	angle lower_limit, upper_limit;
//	std::vector<sonar_contact> contacts;	// store angle, strength and type?

	static const double turn_speed_fast = 6.0;	// degrees per second.
	static const double turn_speed_slow = 2.0;	// degrees per second.

 public:
	sonar_operator_simulation();
	virtual ~sonar_operator_simulation() {}
	virtual void simulate(class game& gm, double delta_t);
//	const std::vector<> get_contacts() const { return contacts; }
};

#endif /* SONAR_H */
