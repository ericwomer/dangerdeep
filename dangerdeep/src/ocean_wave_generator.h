// an ocean wave generator (C)+(W) 2003 by Thorsten Jordan
// realized algorithms of the paper "Simulating ocean water" by
// Jerry Tessendorf.

#ifndef OCEAN_WAVE_GENERATOR
#define OCEAN_WAVE_GENERATOR

#include "vector2.h"
#include <complex>
#include <vector>
using namespace std;

#include "fftw3.h"

#if 1	// use double fft?
#define FFT_COMPLEX_TYPE fftw_complex
#define FFT_REAL_TYPE double
#define FFT_PLAN_TYPE fftw_plan
#define FFT_CREATE_PLAN fftw_plan_dft_c2r_2d
#define FFT_DELETE_PLAN fftw_destroy_plan
#define FFT_EXECUTE_PLAN fftw_execute
// fixme: this doesn't work, the symbols are not found. why?!
#else	// use float fft?
#define FFT_COMPLEX_TYPE fftwf_complex
#define FFT_REAL_TYPE float
#define FFT_PLAN_TYPE fftwf_plan
#define FFT_CREATE_PLAN fftwf_plan_dft_c2r_2d
#define FFT_DELETE_PLAN fftwf_destroy_plan
#define FFT_EXECUTE_PLAN fftwf_execute
#endif

#define GRAVITY 9.806

template <class T>
class ocean_wave_generator
{
	int N;	// grid size
	vector2t<T> W;	// wind direction
	T v;		// wind speed m/s
	T a;		// wave height scalar
	T Lm;		// tile size in m
	vector<complex<T> > h0tilde;
	
	ocean_wave_generator& operator= (const ocean_wave_generator& );
	ocean_wave_generator(const ocean_wave_generator& );
	static T myrnd(void);
	static complex<T> gaussrand(void);
	T phillips(const vector2t<T>& K) const;
	complex<T> h0_tilde(const vector2t<T>& K) const;
	void compute_h0tilde(void);
	complex<T> h_tilde(const vector2t<T>& K, int kx, int ky, T time) const;
	
	FFT_COMPLEX_TYPE* fft_in;	// can't be a vector, since the type is an array
	FFT_REAL_TYPE* fft_out;		// for sake of uniformity
	FFT_PLAN_TYPE plan;
	
public:
	ocean_wave_generator(
		unsigned gridsize = 64,
		const vector2t<T>& winddir = vector2t<T>(T(0.6), T(0.8)),
		T windspeed = T(20.0),
		T waveheight = T(0.0001),
		T tilesize = T(100.0)
	);
	vector<T> compute_heights(T time) const;
	~ocean_wave_generator();
};

//const unsigned qmax = 8;
//const double w0 = 1;

template <class T>
T ocean_wave_generator<T>::myrnd(void)
{
	return T(rand())/RAND_MAX;
}

template <class T>
complex<T> ocean_wave_generator<T>::gaussrand(void)
{
	T x1, x2, w;
	do {
		x1 = T(2.0) * myrnd() - T(1.0);
		x2 = T(2.0) * myrnd() - T(1.0);
		w = x1 * x1 + x2 * x2;
	} while ( w >= T(1.0) );
	w = sqrt( (T(-2.0) * log( w ) ) / w );
	return complex<T>(x1 * w, x2 * w);
}

template <class T>
T ocean_wave_generator<T>::phillips(const vector2t<T>& K) const
{
	T v2 = v * v;
	T k2 = K.square_length();
	if (k2 == T(0)) return T(0);
	T v4 = v2 * v2;
	T k4 = k2 * k2;
	T g2 = GRAVITY * GRAVITY;
	// note: Khat = K.normal() * W should be taken, but we use K
	// and divide |K * W|^2 by |K|^2 = k2 later.
	T KdotW = K * W;
	T KdotWhat = KdotW*KdotW/k2;
	T eterm = exp(-g2 / (k2 * v4)) / k4;
	T l2 = v4/g2 * T(0.0005);	// damping of very small waves
	T result = a * eterm * KdotWhat * exp(-k2*l2);
	if (KdotW < T(0))	// filter out waves moving against the wind
		result *= T(0.25);
	return result;
}

template <class T>
complex<T> ocean_wave_generator<T>::h0_tilde(const vector2t<T>& K) const
{
//in comparison to the water engine by ??? here some randomization is missing.
//but this is like Tessendorff's paper it says.
	return gaussrand() * (sqrt(T(0.5) * phillips(K)));
}

template <class T>
void ocean_wave_generator<T>::compute_h0tilde(void)
{
	const T pi2 = T(2.0)*T(M_PI);
	for (int y = 0; y <= N; ++y) {
		for (int x = 0; x <= N; ++x) {
			vector2t<T> K(pi2*(x-N/2)/Lm, pi2*(y-N/2)/Lm);
			h0tilde[y*(N+1)+x] = h0_tilde(K);
		}
	}
}

template <class T>
complex<T> ocean_wave_generator<T>::h_tilde(const vector2t<T>& K, int kx, int ky, T time) const
{
	complex<T> h0_tildeK = h0tilde[ky*(N+1)+kx];
	complex<T> h0_tildemK = h0tilde[(N-ky)*(N+1)+(N-kx)];
//	unsigned q = unsigned(myrnd()*qmax);
// all frequencies should be multiples of one base frequency (see paper). fixme
//	T wK = q * w0;
	T wK = sqrt(GRAVITY * K.length());
	// fixme: replace by sin/cos terms? why should i?
	const complex<T> I(0,-1);
	return h0_tildeK * exp(I * wK * time) + conj(h0_tildemK * exp(-I * wK * time));
}

template <class T>
ocean_wave_generator<T>::ocean_wave_generator<T>(
		unsigned gridsize,
		const vector2t<T>& winddir,
		T windspeed,
		T waveheight,
		T tilesize )
	: N(int(gridsize)), W(winddir.normal()), v(windspeed), a(waveheight), Lm(tilesize)
{
	h0tilde.resize((N+1)*(N+1));
	compute_h0tilde();
	fft_in = new FFT_COMPLEX_TYPE[N*(N/2+1)];
	fft_out = new FFT_REAL_TYPE[N*N];
	plan = FFT_CREATE_PLAN(N, N, fft_in, fft_out, 0);
}

template <class T>
ocean_wave_generator<T>::~ocean_wave_generator<T>()
{
	FFT_DELETE_PLAN(plan);
	delete[] fft_in;
	delete[] fft_out;
}

template <class T>
vector<T> ocean_wave_generator<T>::compute_heights(T time) const
{
	const T pi2 = T(2.0)*T(M_PI);
//fixme: compute h0_tilde for all K's is not needed.
	for (int y = 0; y <= N/2; ++y) {
		for (int x = 0; x < N; ++x) {
			vector2t<T> K(pi2*(x-N/2)/Lm, pi2*(y-N/2)/Lm);
			complex<T> c = h_tilde(K, x, y, time);
			int ptr = x*(N/2+1)+y;
			fft_in[ptr][0] = c.real();
			fft_in[ptr][1] = c.imag();
		}
	}

	FFT_EXECUTE_PLAN(plan);
	
	// our kx,ky are in {-N/2...N/2}, but fft goes from {0...N-1}
	// so we have to add N/2 in the formulas, a term that can be seperated as exponential
	// term: exp(I*pi*(x+y)) that is equal to (-1)^(x+y), so we have to adjust
	// the sign at every second element in checkerboard form.
	// we copy the result to the output array in parallel.
	vector<T> waveheights(N*N);
	T signs[2] = { T(1), T(-1) };
	for (int y = 0; y < N; ++y)
		for (int x = 0; x < N; ++x)
			waveheights[y*N+x] = fft_out[y*N+x] * signs[(x + y) & 1];
	return waveheights;
}

#endif
