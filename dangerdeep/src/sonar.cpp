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

//#include "sea_object.h"
//#include "submarine.h"
#include "sonar.h"
#include "game.h"
#include "angle.h"
#include "error.h"
#include "submarine.h"

/* passive sonar, GHG etc */
/* how its done:
   for all noise sources within a sensible range (20 seamiles or so)
   compute signal strength for four frequency bands (0-1kHz,1-3kHz,3-6kHz,6-7kHz).
   compute direction of signal.
   compute sonar signal sensitivity (depends on sonar type - specific sensitivity - and of
     speed of receiver. Higher speeds decrease sensitivity, much more effect than speed of
     targets for target noise strength).
   depending on angle of ghg apparaturs reduce signal strength of all signals
   (cos(angle) between direction and ghg angle = normal vector from ghg angle).
   (NOTE: do this ^ with new reduction function!)
   handle background noise of own sub/ship
   compute and add background noise (ambient noise)
   subtract own noise + sensitivity, rest is signal strength
   make signal strengths discrete, depending on frequency, lower freqs -> larger steps !NO!
     discrete steps not in dB but depending on angle! lower freq -> bigger steps !NO!
   discretize ALL signals only to dB, independent of frequency!!!
   sum of strenghts is resulting noise strength, feed to user's headphones or to sonar operator simulator
   weighting of various frequency strengths depends on human perception and width of freq. bands
     1-3 kHz *0.9, 0-1 kHz *1, 3-6 kHz *0.7 6-7 kHz *0.5, total strength is weighted sum div. weight sum of used bands.
   => noise of ships must be stored in distributed frequencies or we need a online bandpass filter
      to weaken higher frequencies
   blind spots of GHG etc need to be simulated, BG is blind to aft, GHG to front/aft, KDB to ?
   NOTE: compute that with strength reduction formula!
   
   HOW THE GHG WORKS:
   Sound signals arrive at the hydrophones on different points of time.
   Thus, a signal function depends on time: s(t).
   The signal is the same for all hydrophones, but with varying time:
   s_i(t_i) = s_j(t_j) for i,j in 0,...11 for 12 hydrophones, and t_n = t_0 + n * delta_t
   delta_t can be computed, it is the the time a signal needs to travel in water to
   cover the distance between the two hydrophones. If the electrical delay caused by the
   strip lines of the strip line array inside the GHG match delta_t, then all signals
   are overlayed with the same time factor:
   strip line output = sum over i of s_i(t_i ) = sum over i of s_i(t_0 + i * delta_t)
   We know that a sum of signals gives the signal with maximum amplitude when there is
   no phase shift between them, e.g. sum_i=0..3 sin(x+i*m) for any small m is maximal
   when m is zero. The larger the absolute value of m is the weaker the signal gets.
   Thus, the strip line output is maximized, when the delay of each signal in water
   equals the electrical delay.
   (Note that the formulas above match a situation where a wave comes from the bow and the
   strip line array's membranes are perpendicular to the sound wave, thus the GHG points
   also to the bow. But it even works when the hydrophones are arranged in a line from
   bow to aft and the sound wave comes directly from starboard direction - in that case the
   sound hits all hydrophones at the same time, all signals go to one strip line and thus
   the output is maximized. When the strip line array is turned away then the hydrophone
   outputs are distributed over multiple lines, causing delay and phase shift of the summands
   of the signal, thus leading to a weaker total output.)
   Proof: the hydrophones are arranged in an array with roughly 2.2m length. To travel
   that distance the sound takes 1.5ms. The strip line array has 100 strips with 17µs
   delay between each strip given a total maximum delay of 1.7ms. This fits.
   When the strip line array is not facing the signal perpendicular, the electrical delay
   and the sound delay in water differs, leading to a phase shift of signals in any direction,
   weakening the total signal output. This is modelled by the fall-off function that is a
   function of the angle between the GHG apparatus angle and the incoming signal's angle and
   also of the frequency of the signal.
   We only need to compute an historically accurate fall-off function. This is done by
   simulating the phase shifts and thus a real GHG as close as possible.
   (at the moment the function compute_signal_strength_GHG() does this).
*/

const double noise_signature::frequency_band_lower_limit[NR_OF_SONAR_FREQUENCY_BANDS] = { 0, 1000, 3000, 6000 };
const double noise_signature::frequency_band_upper_limit[NR_OF_SONAR_FREQUENCY_BANDS] = { 1000, 3000, 6000, 7000 };
const double noise_signature::background_noise[NR_OF_SONAR_FREQUENCY_BANDS] = { 8, 10, 5, 2 };
const double noise_signature::seastate_factor[NR_OF_SONAR_FREQUENCY_BANDS] = { 60, 50, 40, 30 };
const double noise_signature::noise_absorption[NR_OF_SONAR_FREQUENCY_BANDS] = { 0.008, 0.01, 0.02, 0.03 };

const double noise_signature::typical_noise_signature[NR_OF_SHIP_CLASSES][NR_OF_SONAR_FREQUENCY_BANDS] = {
	{ 200, 150, 60, 20 },	// warship, very strong, rather low frequencies
	{ 120, 100, 80, 60 },	// escort, strong, many high frequencies too
	{ 100, 130, 80, 40 },	// merchant, medium signal, various frequencies
	{ 80, 80, 60, 60 },	// submarine, weak-medium, low+high frequencies
	{ 30, 40, 30, 30 }	// torpedo, weak, many high frequencies
};

const double noise_signature::typical_frequency[NR_OF_SONAR_FREQUENCY_BANDS] = { 900, 2500, 5000, 6500 };

double noise_signature::compute_ambient_noise_strength(unsigned band, double seastate)
{
	if (band >= NR_OF_SONAR_FREQUENCY_BANDS)
		throw error("illegal frequency band number");

	return background_noise[band] + seastate_factor[band] * seastate;
}



double noise_signature::compute_signal_strength(unsigned band, double distance, double speed,
						      bool caviation) const
{
	if (band >= NR_OF_SONAR_FREQUENCY_BANDS)
		throw error("illegal frequency band number");

	// noise source caused noise
	double L_base = band_data[band].basic_noise_level + band_data[band].speed_factor * speed;
	if (caviation)
		L_base += cavitation_noise;
	// compute propagation reduction
	// Sound intensity I = p^2 / (c * ro) with:
	// p = Pressure (N/m^2 = Pa)
	// c = speed of sound, 1465 m/s in sea water
	// ro = specific gravity of water (1000kg/m^3)
	// Intensity with propagation decreases with square of range, so:
	// I_prop = I / R^2     and   L = 10 * log_10 (I / I_0), here  L = 10 * log_10 (I)
	// so  L_prop = 10 * log_10 (I / R^2) = 10 * log_10 (I) - 10 * log_10 (R^2)
	//            = L - 20 * log_10 (R)
	if (distance < 1) distance = 1;
	double L_prop = -20 * log10(distance);
	// compute absorption
	double L_absorb = -noise_absorption[band] * distance;
	// sum up noise source noise
	double L_source = L_base + L_prop + L_absorb;
	// avoid extreme values (would give NaN otherwise, when converting to real values and back)
	if (L_source < -100)
		L_source = -100;

//  	printf("L_base=%f L_prop=%f L_abs=%f L_src=%f\n",
//  	       L_base, L_prop, L_absorb, L_source);

	return L_source;
}



const double noise_signature::frequency_band_strength_factor[NR_OF_SONAR_FREQUENCY_BANDS] = { 0.8, 1.0, 0.6, 0.4 };

double noise_signature::compute_total_noise_strength(const std::vector<double>& strengths)
{
	if (strengths.size() != NR_OF_SONAR_FREQUENCY_BANDS)
		throw error("illegal number of frequency bands");
	double sum = 0;
	for (unsigned i = 0; i < NR_OF_SONAR_FREQUENCY_BANDS; ++i) {
		sum += frequency_band_strength_factor[i] * pow(dB_base, strengths[i]);
	}
	return 10*log10(sum);
}



shipclass noise_signature::determine_shipclass_by_signal(const std::vector<double>& strengths)
{
	// normalize noise by highest value of all frequencies.
	// do the same for noise signatures of ship classes to compare them better
	double strength_normalizer = strengths[0];
	for (unsigned i = 1; i < noise_signature::NR_OF_SONAR_FREQUENCY_BANDS; ++i) {
		strength_normalizer = std::max(strength_normalizer, strengths[i]);
	}

	shipclass myclass = NONE;
	double minerror = 1e30;
	for (unsigned k = 0; k < NR_OF_SHIP_CLASSES; ++k) {
		const double* typical_signature = noise_signature::typical_noise_signature[k];
		double class_strength_normalizer = typical_signature[0];
		for (unsigned i = 1; i < noise_signature::NR_OF_SONAR_FREQUENCY_BANDS; ++i) {
			class_strength_normalizer = std::max(class_strength_normalizer, typical_signature[i]);
		}

		double error = 0;
		//printf("trying ship class %u\n", k);
		for (unsigned i = 0; i < noise_signature::NR_OF_SONAR_FREQUENCY_BANDS; ++i) {
			double diff = strengths[i]/strength_normalizer
				- typical_signature[i]/class_strength_normalizer;
			//printf("diff %u = %f\n", i, diff);
			error += diff * diff;
		}
		//printf("total error %f\n", error);
		
		if (error < minerror) {
			minerror = error;
			myclass = shipclass(k);
		}
	}
	// fixme: if error is above a certain level, type can't be determined, set to none...
	return myclass;
}



// translated from python script, refine later
double compute_signal_strength_GHG(angle signal_angle, double frequency, angle apparatus_angle)
{
	// global constants
	static const double speed_of_sound_in_water = 1465.0;	// m/s

	// GHG constants
	static const double strip_delay = 0.000017;		// 17µs
	static const angle hydrophone_fov = 180.0;		// degrees
	static const unsigned nr_hydrophones = 12;
	static const double distance_hydro = 0.2;		// m
	static const angle hydrophone_fov_center_first = 24.0;	// degrees
	static const angle hydrophone_fov_center_delta = 12.0;	// degrees
	static const unsigned nr_of_strips = 100;

	static const double delta_t = distance_hydro / speed_of_sound_in_water;
	static const double distance_contact = delta_t / strip_delay;
	static const double height_of_all_contacts = distance_contact * (nr_hydrophones - 1);

	//printf("comp sign str signang=%f freq=%f appang=%f\n", signal_angle.value(), frequency, apparatus_angle.value());

	// if apparatus_angle > 180 we're listening to port side, swap signs of angles in that case
	if (apparatus_angle.value_pm180() < 0) {
		apparatus_angle = -apparatus_angle;
		signal_angle = -signal_angle;
		// if signal comes from opposite site, strength is zero
		if (signal_angle.value_pm180() < 0) {
			//printf("signal from other side?! strange\n");
			return 0.0;
		}
		//printf("comp sign str NOW signang=%f freq=%f appang=%f\n", signal_angle.value(), frequency, apparatus_angle.value());
	}

	double time_scale_fac = 2*M_PI*frequency;
	double delta_t_signal = signal_angle.cos() * delta_t;
	double sum_cos = 0.0, sum_sin = 0.0;
	double amplitude[nr_hydrophones];
	double phase_shift[nr_hydrophones];
	double max_strength = 0.0;
	for (unsigned i = 0; i < nr_hydrophones; ++i) {
		angle fov_center = hydrophone_fov_center_first + hydrophone_fov_center_delta * double(i);
		angle rel_angle = signal_angle - fov_center;
		amplitude[i] = rel_angle.cos();
		if (fabs(rel_angle.value_pm180()) > hydrophone_fov.value()*0.5)
			amplitude[i] = 0;
		max_strength += amplitude[i];
		double y = (height_of_all_contacts*0.5 - distance_contact*i) * apparatus_angle.cos();
		int y_line = int(y + nr_of_strips/2);
		double delay_i = strip_delay * y_line;
		phase_shift[i] = (delay_i + delta_t_signal * i) * time_scale_fac;
		sum_cos += amplitude[i] * cos(phase_shift[i]);
		sum_sin += amplitude[i] * sin(phase_shift[i]);
		//printf("i=%u ampl=%f phs=%f\n",i,amplitude[i],phase_shift[i]);
	}
	double x_extr = atan(sum_cos / sum_sin);
	//printf("max_strength=%f sum_cos=%f sum_sin=%f, x_extr=%f\n",max_strength,sum_cos,sum_sin,x_extr);

	// now the signal has an extreme value for x = x_extr, compute signal strength
	double signalstrength = 0.0;
	for (unsigned i = 0; i < nr_hydrophones; ++i) {
		signalstrength += amplitude[i] * sin(x_extr + phase_shift[i]);
	}
	signalstrength = fabs(signalstrength);

	//printf("result: signstr %f perc %f\n", signalstrength, signalstrength/max_strength);
	// now compute percentage of max. strength
	return signalstrength / max_strength;
}



sonar_operator_simulation::sonar_operator_simulation()
	: state(find_growing_signal),
	  turn_speed(turn_speed_fast),
	  current_signal_strength(-1)
{
}



/* how the sonar operator works:
   He turns the apparatus at constant speed around the compass, with 2 or 3 degrees per
   simulation step, total speed of 6 degrees per second or so.
   When he detects a transition from growing strength to falling strengh of the signal,
   he turns the apparatus in opposite direction with slower speed (0.5-1 degree per
   simulation step) and detects the angle where the signal reaches its maximum
   (upper limit), turns back until signal gets weaker (lower limit), then
   reports the mid between the lower and upper limit as contact, turns the apparatus
   back to upper limit and starts over.
   If a signal is very strong and thus close, he switches to tracking mode.
   In tracking mode he turns the apparatus around +- 30 degrees around the strongest signal
   and redects its position with the mentioned algorithm until the signal gets weaker.
   Or he turns the apparatus away from current peak angle until signal gets weaker, then back.
   Always searching for the limit of the signal, following a direction only if signal
   gets stronger along the direction. Direction is constantly changed between clockwise
   and counter-clockwise in that case.
   fixme - when does the operator choose the frequency band switch? This must depend
   on how far/strong a signal is...
   So we have several states (circular mostly)
   1) Just turning and listening to strengths -> detect signal with growing strength
                                                 when new strength > old strength, then to 2)
   2) Following signal increase -> detect weakening of signal, then to 3)
   3) Detect upper limit by finer turning in opposite direction, when new signal is <= current
      signal. Equal -> 4), less -> 5)
   4) Detect lower limit of signal by finer turning, until new signal is weaker than current,
                                                     then 5)
   5) report mid of lower and upper as new contact, turn current angle to upper with normal speed
                                                    then back to 1)
   5) is same as one...
*/
void sonar_operator_simulation::simulate(game& gm, double delta_t)
{
	submarine* player = dynamic_cast<submarine*>(gm.get_player());
	// check to get sensible values for first run
	if (current_signal_strength < 0)
		current_signal_strength =
			floor(noise_signature::compute_total_noise_strength(gm.sonar_listen_ships(player, current_angle)));

	printf("sonar man sim, angle=%f str=%f stat=%i\n", current_angle.value(), current_signal_strength, state);

	angle next_angle = current_angle + turn_speed * delta_t;
	// fixme: floor is used as quantization hack...
	double nstr = floor(noise_signature::compute_total_noise_strength(gm.sonar_listen_ships(player, next_angle)));

	switch (state) {
	case find_growing_signal:
		if (nstr > current_signal_strength) {
			state = find_upper_limit;
		}
		break;
	case find_upper_limit:
		if (nstr < current_signal_strength) {
			upper_limit = next_angle;
			turn_speed = -2.0;//turn_speed_slow;//fixme? must be public?
			state = find_lower_limit;
		}
		break;
	case find_lower_limit:
		if (nstr > current_signal_strength) {
			upper_limit = next_angle;
		} else if (nstr < current_signal_strength) {
			lower_limit = current_angle;
			// replace signals in the vicinity...
			angle center = lower_limit + angle((upper_limit - lower_limit)).value() * 0.5;
			//report_signal(center, current_signal_strength);
			printf("sonar man sim, Peak of signal found at angle %f, strength %f\n",
			       center.value(), current_signal_strength);
			next_angle = current_angle;
			nstr = current_signal_strength;
			turn_speed = turn_speed_fast;
			state = find_growing_signal;
		}
		break;
	case track_signal:
		// fixme
		break;
	}

	current_angle = next_angle;
	current_signal_strength = nstr;
}
