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
//#include <sstream> // for testing, fixme

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

   fixme: handle noise strengths a bit different! do not cut off at 0 dB!
   0 dB = Intensity 1, but sensor sensitivity can be much smaller, up to
   -18 dB = 0.015 N/m² and thus even much weaker signals are detected...
   I_0 = 10^-6 Pa/m²
   Propagation: loss off 100dB on 10000m, or factor 10^-10
*/

const double noise::dB_base = 1.25892541179;
const double noise::cavitation_noise = 2;

const double noise::frequency_band_lower_limit[NR_OF_FREQUENCY_BANDS] = { 20, 1000, 3000, 6000 };
const double noise::frequency_band_upper_limit[NR_OF_FREQUENCY_BANDS] = { 1000, 3000, 6000, 7000 };
const double noise::background_noise[NR_OF_FREQUENCY_BANDS] = { 8, 10, 5, 2 };
const double noise::seastate_factor[NR_OF_FREQUENCY_BANDS] = { 60, 50, 40, 30 };
// noise absorption: dB per m, Harpoon3 uses 1/6, 1, 3 for L/M/H for range of 1sm
// so divide these by 1852
// from another source: the formula is (2.1*10^-10 * (T-38)^2 + 1.3*10^-7) * f^2  (dB/m)
// whereas T is water temperature in centigrade and f is frequency in kHz, thus
// take f = real_f * 10^-3, and f^2 = real_f^2 * 10^-6 and so
// (2.1 * 10^-16 * (T-38)^2 + 1.3*10^-13)
// if we assume 10.38° for water in open ocean this gives (2.1 * 10^-16 * 10^6 + 1.3*10^-13) = (2.1*10^-10 + 1.3*10^-13)
// = 2101.3 * 10^-13, thus absorption in dB/m is 2101.3 * 10^-13 * real_f^2
// this gives for 100Hz: 2.1013e-6, 500Hz: 5.25325e-5, 2kHz: 8.4052e-4, 7kHz: 0.01029637 
// fixme: compute with formula, maybe build medium over frequency range!
// these values here are hand-computed for 10° temperature with medium values.
const double noise::noise_absorption[NR_OF_FREQUENCY_BANDS] = { 0.000066147, 0.000842717, 0.004083938, 0.002744233 };

// if we want to make 900Hz sound detectable at 10sm, what would noise_absorption have to be?
// take a merchant with 8 knots, basic noise is 100dB + 1dB/ m/s = 104.1 dB
// propagation loss is -20*log10(distance) = -85.4 dB
// absorption = 10*log(db^104.1 - db^85.4) / 10sm = 104.04 dB / 18520m = 0.00562

const double noise_signature::typical_noise_signature[NR_OF_SHIP_CLASSES][noise::NR_OF_FREQUENCY_BANDS] = {
	{ 200, 150, 60, 20 },	// warship, very strong, rather low frequencies
	{ 100, 120, 80, 60 },	// escort, strong, many high frequencies too
	{ 120, 100, 80, 40 },	// merchant, medium signal, various frequencies
	{ 50, 50, 40, 30 },	// submarine, weak-medium, low+high frequencies
	{ 10, 40, 80, 70 }	// torpedo, weak, many high frequencies
};

const double noise::typical_frequency[NR_OF_FREQUENCY_BANDS] = { 900, 2500, 5000, 6800 };

noise noise::compute_ambient_noise_strength(double seastate)
{
	noise result;
	for (unsigned band = 0; band < NR_OF_FREQUENCY_BANDS; ++band) {
		result.frequencies[band] = 
			pow(dB_base, background_noise[band] + seastate_factor[band] * seastate);
	}
	return result;
}



noise noise_signature::compute_signal_strength(double distance, double speed, bool caviation) const
{
	noise result;
	for (unsigned band = 0; band < noise::NR_OF_FREQUENCY_BANDS; ++band) {
		// noise source caused noise
		double L_base = band_data[band].basic_noise_level + band_data[band].speed_factor * speed;
		if (caviation)
			L_base += noise::cavitation_noise;
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
		double L_absorb = -noise::noise_absorption[band] * distance;
		// sum up noise source noise
		double L_source = L_base + L_prop + L_absorb;
		// avoid extreme values (would give NaN otherwise, when converting to real values and back)
		if (L_source < -100)
			L_source = -100;

		//  	printf("L_base=%f L_prop=%f L_abs=%f L_src=%f\n",
		//  	       L_base, L_prop, L_absorb, L_source);
		result.frequencies[band] = noise::dB_to_absolute(L_source);
	}
	return result;
}



const double noise::frequency_band_strength_factor[NR_OF_FREQUENCY_BANDS] = { 1.0, 0.9, 0.8, 0.7 };

double noise::compute_total_noise_strength() const
{
	double sum = 0;
	for (unsigned band = 0; band < NR_OF_FREQUENCY_BANDS; ++band) {
		sum += frequency_band_strength_factor[band] * frequencies[band];
	}
	return sum;
}



shipclass noise::determine_shipclass() const
{
	// normalize noise by highest value of all frequencies.
	// do the same for noise signatures of ship classes to compare them better
	double strength_normalizer = frequencies[0];
	for (unsigned i = 1; i < NR_OF_FREQUENCY_BANDS; ++i) {
		strength_normalizer = std::max(strength_normalizer, frequencies[i]);
	}

	shipclass myclass = NONE;
	double minerror = 1e30;
	for (unsigned k = 0; k < NR_OF_SHIP_CLASSES; ++k) {
		const double* typical_signature = noise_signature::typical_noise_signature[k];
		double class_strength_normalizer = typical_signature[0];
		for (unsigned i = 1; i < NR_OF_FREQUENCY_BANDS; ++i) {
			class_strength_normalizer = std::max(class_strength_normalizer, typical_signature[i]);
		}

		double error = 0;
		//printf("trying ship class %u\n", k);
		for (unsigned i = 0; i < NR_OF_FREQUENCY_BANDS; ++i) {
			double diff = frequencies[i]/strength_normalizer
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
#if 1
	double cosang = std::max(0.0, (signal_angle - apparatus_angle).cos());
//	printf("siga=%f appa=%f diff=%f cos=%f\n",signal_angle.value(), apparatus_angle.value(), (signal_angle -apparatus_angle).value(), cosang);
	// use 280 as exponent for 7kHz, so f * 0.04 = exponent.
	return pow(cosang, frequency * 0.04);
#else
	// global constants
	static const double speed_of_sound_in_water = 1465.0;	// m/s

	// GHG constants
	static const double strip_delay = 0.000017;		// 17µs
	static const angle hydrophone_fov = 180.0; // 180.0;		// degrees
	static const unsigned nr_hydrophones = 24; // 12;
	static const double distance_hydro = 0.2;		// m
	static const angle hydrophone_fov_center_first = 21.0; // 24.0;	// degrees
	static const angle hydrophone_fov_center_delta = 6.0; // 12.0;	// degrees
	static const unsigned nr_of_strips = 99;	// use an odd number here!

	/* note! the constants depend on each other.
	   total delay is strip_delay * nr_of_strips = 1.683ms
	   the hydrophones must be arranged on a line NOT LONGER than the range
	   the sound travels in that time. So 1465m/s * 1.683ms = 2.465595m.
	   This number must be greater than (nr_hydrophones-1) * distance_hydro,
	   which it is here (0.2m * (12-1) = 2.2m)
	*/

	static const double delta_t = distance_hydro / speed_of_sound_in_water;
	static const double distance_contact = delta_t / strip_delay;
	static const double height_of_all_contacts = distance_contact * (nr_hydrophones - 1);

	//printf("comp sign str signang=%f freq=%f appang=%f\n", signal_angle.value(), frequency, apparatus_angle.value());
	bool app_on_port = (apparatus_angle.value() >= 180.0);

	double time_scale_fac = 2*M_PI*frequency;
	double delta_t_signal = signal_angle.cos() * delta_t;
	double sum_cos = 0.0, sum_sin = 0.0;
	double amplitude[nr_hydrophones];
	double phase_shift[nr_hydrophones];
	double max_strength = 0.0;
	for (unsigned i = 0; i < nr_hydrophones; ++i) {
		// hydrophones are numbered from bow to stern,
		// where strip lines and y-coordinates of contacts on them
		// go from "stern" to "bow", that is bottom to top.
		angle fov_center = hydrophone_fov_center_first + hydrophone_fov_center_delta * double(i);
		if (app_on_port) fov_center = -fov_center;
		angle rel_angle = signal_angle - fov_center;
		amplitude[i] = rel_angle.cos();
 		if (fabs(rel_angle.value_pm180()) > hydrophone_fov.value()*0.5)
 			amplitude[i] = 0;
		max_strength += amplitude[i];
		double y = (height_of_all_contacts*0.5 - distance_contact*i) * apparatus_angle.cos();
		int y_line = int(floor(y + nr_of_strips*0.5));
		//printf("y=%f yp=%f ryp=%f y_line=%i\n", y, y + nr_of_strips*0.5, floor(y + nr_of_strips*0.5), y_line);
		double delay_i = strip_delay * y_line;
		// note that delay_i is the full delay to the output to the headphones.
		// if the first strip line with a contact has number N and not 0,
		// all signals are shifted by N * strip_delay. This doesn't change
		// the computation result though. In fact it is like the GHG worked.
		//printf("delay_i=%f delta_t_signal=%f i=%u tsf=%f sum=%f\n",
		//       delay_i, delta_t_signal, i, time_scale_fac, (delay_i + delta_t_signal * i));
		phase_shift[i] = (delay_i + delta_t_signal * i) * time_scale_fac;
		sum_cos += amplitude[i] * cos(phase_shift[i]);
		sum_sin += amplitude[i] * sin(phase_shift[i]);
		//printf("i=%u ampl=%f phs=%f\n",i,amplitude[i],phase_shift[i]);
	}
	double x_extr = atan(sum_cos / sum_sin);
	// Note: the output of this function seems to contain a bit jitter,
	// the computed strength oscillates a bit around the exact value.
	// This effect seems to be amplified by the snapping to integer strip line
	// numbers, but can be seen also with infinite small strips. A rounding error?
	// a result of the sidelobe phenomenon?! ghg phenomenon, disappears with smaller strip lines.

	// Note2: a signal seems to cause a second signal with 90° offset... sidelobe?!
	// ghsot signal. appears only with higher frequencies, ghg phenomenon.
	// maybe the arrangement of the hydrophones, their fov and listening direction is
	// not fully correct, causing or amplifying these phenomenons, fixme!
	// each hydrophone must listen to a certain direction, if all listen to 90°
	// then the output is unusable, showing peaks in direction of each hydrophone.
	// If each hydrophone has a fov < 180° the output is also much worse, causing
	// many ghost signals and so on. The same is true for fov > 180°.

	//printf("max_strength=%f sum_cos=%f sum_sin=%f, x_extr=%f\n",max_strength,sum_cos,sum_sin,x_extr);
	//std::ostringstream oss; oss << "plot [x=" << -0.5*M_PI << ":" << 0.5*M_PI << "] ";
	// now the signal has an extreme value for x = x_extr, compute signal strength
	double signalstrength = 0.0;
	for (unsigned i = 0; i < nr_hydrophones; ++i) {
		signalstrength += amplitude[i] * sin(x_extr + phase_shift[i]);
		//oss << amplitude[i] << "*sin(x+" << phase_shift[i] << ")+";
	}
	//oss << "0\n";
	//printf(oss.str().c_str());
	signalstrength = fabs(signalstrength);

	//printf("result: signstr=%f perc=%f appang=%f\n", signalstrength, signalstrength/max_strength, apparatus_angle.value());
	// now compute percentage of max. strength
	//fixme: try this: don't divide here!!! doesn't help much, but a bit though, seems to lessen jitter...
	return signalstrength; //  / max_strength;
#endif
}
