#!/usr/bin/python
# compute time delay for signal of each hydrophone and produce a
# gnuplot command line
from math import *
import os
import sys
# -------------- constants -------------------
# speed of sound in water
speedwater = 1465.0
# nr of hydrophones per side, phones are numbered from bow to aft 0...nr-1
nr_hydrophones = 12
# distance between hydrophone membrane centers, roughly 20cm
distance_hydro = 0.2
# electrical delay, 17Âµs
strip_delay = 0.000017
# fov of a hydrophone
hydrophone_fov = 180.0
# center of fov for first hydrophone and delta
hydrophone_fov_center_first = 24.0
hydrophone_fov_center_delta = 12.0

# now a signal of hydrophone i is shifted by
# t_0 + i * delta_t
delta_t = distance_hydro / speedwater
print 'delta_t = ' + str(delta_t)
# nr of strip lines, use an odd number here!!! note, the number depends on strip_delay indirectly!!!
# line 0 is at bottom! so it has delay 0, line 98 has max. delay (98*17µs)
# this means the signals arriving first, at the highest line, are delayed by the
# highest value.
# the total y-coordinate range of the strips goes from -nr_strips/2 to +nr_strips/2,
# because the strip line array is turned around its center.
nr_strips = 99
# distance between two hydrophone output contacts on strip line array, in lines
distance_contact = delta_t / strip_delay
print 'distance between contacts on line array = ' + str(distance_contact)
height_of_all_contacts = distance_contact * (nr_hydrophones - 1)

# -------------- variables from here on -------
# signal direction (angle)
signal_angle = 45.0
# frequency of wave to measure, in Hz
noise_freq = 1000
# signal comes from 0° --------------
# here give ghg apparatus angle
app_angle = 150.0
if len(sys.argv) > 1:
  app_angle = int(sys.argv[1])
app_angle_rad = app_angle * pi / 180.0
# factor for input of signal function sin(x)
# a full period is done noise_freq times per second
# time t is in seconds
# signal(t) = sin(t * time_scale_fac)
# x = 0 at t = 0, x = 2*pi at t = 1/noise_freq
# so a period has length (in meters) of 1/noise_freq
time_scale_fac = 2*pi * noise_freq

# now output of phone i is signal(t_0 + i * delta_t)
# where t_0 is 0 and thus output is
# sin(i * delta_t * time_scale_fac)
# compute the line nr of the strip line for each output
# and delay it accordingly
# we compute backwards from output signal, thus we have
# to subtract the delay value to get the final output signal.

# try to measure max. signal peak
samples = 1000
signal = [0.0] * samples

# maximum strength is sum of all hydrophone output signal strengths.
max_strength = 0.0

plotoutput = ''
delta_t_signal = cos(signal_angle * pi / 180.0) * delta_t
print 'delta_t=' + str(delta_t) + ' delta_t_signal=' + str(delta_t_signal)

# if delta of phase shift would be constant (it is not because of the int() in computing the strip line number),
# then we can sum up the signals with simpler formulas:
# it is r*cos(phi)+r*i*sin(phi) = r*e^(i*phi)
# and thus
# f(x) = sum_j=0...n-1 a_j * sin(x + p_j)  where a_j, p_j are amplitude and phase shift values for each signal
# we find the maximum by differentiating f(x) and solving f'(x) = 0
# with p_j = p_0 + delta_p we have:
# f(x) = Im(sum_j=0...n-1 a_j * e^(i*(x+p_0+j*delta_p)))
# extracting the constant factors and by defining c_j := a_j * e^(i*j*delta_p) we get:
# f(x) = Im(e^(i*(x+p_0)) * sum_j=0...n-1 c_j)   and the last part is a constant! we define it as "d", thus
# f(x) = cos(x+p_0) * Im(d) + sin(x+p_0) * Re(d)
# f'(x) = -sin(x+p_0) * Im(d) + cos(x+p_0) * Re(d)
# f'(x) = 0 <=> Re(d)/Im(d) = tan(x+p_0) <= arctan(Re(d)/Im(d)) - p_0 = x
# so solve x, get f(x) and you have the maximum factor.
# remember, d := sum_j=0...n-1 a_j * e^(i*j*delta_p)
# BUT, IT CAN BE DONE SIMPLER, WITHOUT COMPLEX NUMBERS:
# f(x) = sum_j=0...n-1 a_j * sin(x + p_j)  thus f'(x):
# f'(x) = sum_j=0...n-1 a_j * cos(x + p_j)  = 0 if what? transform f'(x), with sin/cos theorems
# f'(x) = sum_j=0...n-1 a_j * cos(x) * cos(p_j) - a_j * sin(x) * sin(p_j)
#       = cos(x) * sum_j=0...n-1 a_j * cos(p_j)  -  sin(x) * sum_j=0...n-1 a_j * sin(p_j)
# thus f'(x) = 0 when  tan(x) = (sum_j=0...n-1 a_j * cos(p_j)) / (sum_j=0...n-1 a_j * sin(p_j))
# thus f'(x) = 0 when  x = atan((sum_j=0...n-1 a_j * cos(p_j)) / (sum_j=0...n-1 a_j * sin(p_j)))
# if you look at the gnuplot output is seems that f(x) = c * sin(x + d), so period length
# is kept! f(x) has its maximum obviosly when sin(x+d) = +- 1, so max. is c.
# We can compute c with f(x) and d, but we know neither c nor d.
# the resulting function f(x) has symmetric extrema, so |f(x_1)| = |(f_x2)| for any extremum at x_1,x_2
# so take |f(x)| with x is solution of the equation above as max. amplitude value.
sum_cos = 0.0
sum_sin = 0.0

for i in range(0, nr_hydrophones):
  fov_center = hydrophone_fov_center_first + i * hydrophone_fov_center_delta
  rel_angle = signal_angle - fov_center
  strength_factor = cos(pi*rel_angle/180.0)
  if abs(rel_angle) > hydrophone_fov/2:
    strength_factor = 0.0
  max_strength += strength_factor
  print 'hydro_nr=' + str(i) + ' fovcenter=' + str(fov_center) + ' rel_angle=' + str(rel_angle) + ' strfac=' + str(strength_factor)
  # compute strip line that contact is onto
  # this depends only on angle
  # at 0° y_0 is height_of_all_contacts/2, delta_y is -distance_contact
  # n° this is height_of_all_contacts*cos(n), delta_y is -distance_contact*cos(n)
  y = (float(height_of_all_contacts)/2 - distance_contact*i) * cos(app_angle_rad)
  y_line = int(floor(y + nr_strips*0.5))
  print 'i=' + str(i) + ' y=' + str(y) + ' y_line=' + str(y_line) + ' y+nr2=' + str(y + nr_strips*0.5)
  delay_i = strip_delay * y_line
  print 'delta_t_signal*i=' + str(i*delta_t_signal)
  # signal delay is in seconds (time)
  signaldelay = delay_i + i * delta_t_signal
  print 'delay_i=' + str(delay_i) + ' signaldelay=' + str(signaldelay)
  plotoutput += str(strength_factor) + '*sin(x'
  # we scale signal_delay to the length of one period. If we have 2kHz each period is 0.5ms long,
  # thus we scale time by 2000, a shift of e.g. 0.1ms would then be scaled to 0.2*2*Pi, which is
  # 20% of full "circle" (=2pi) which is the same quotient as 0.1/0.5ms, and this is correct.
  plotfac = time_scale_fac * signaldelay
  print 'delay_i='+str(delay_i)+' delta_t_signal='+str(delta_t_signal)+' i='+str(i)+' tsf='+str(time_scale_fac)+' sum='+str(signaldelay)
  print 'hydronr=' + str(i) + ' ampl=' + str(strength_factor) + ' phas=' + str(plotfac)
  sum_cos += strength_factor * cos(plotfac)
  sum_sin += strength_factor * sin(plotfac)
  # note! the plot shows only one period of the sine function, but we have as many periods
  # as we have as value for frequency... however that doesnt change the amplitude, so its of
  # no matter...
  if (plotfac < 0.0):
    plotoutput += str(plotfac)
  else:
    plotoutput += '+' + str(plotfac)
  plotoutput += ')+'
  # measuring max. signal peak
  for x in range(0, samples):
    sinfac = float(x)*(2*pi)/samples + plotfac  # x*time_scale_fac/noise_freq = x*(2*pi)
    signal[x] += strength_factor * sin(sinfac)

plotoutput = 'plot [x=0:' + str(2*pi) + '] ' + str(max_strength) + '*sin(x),' + plotoutput
sum_quot = sum_cos / sum_sin
x_extr = atan(sum_quot)
print 'x_extr_smpl=' + str(samples*x_extr/(2*pi))
print 'sum_cos=' + str(sum_cos) + ' sum_sin=' + str(sum_sin) + ' quot=' + str(sum_quot) + ' x_extr=' + str(x_extr)
while (x_extr < 0):
  x_extr += 2*pi
while (x_extr >= 2*pi):
  x_extr -= 2*pi
print 'signal[x_extr]=' + str(signal[int(x_extr * samples / (2*pi))])
# now measure max/min value
#gnuplot output seems dependant on absolute value of signal angle... strange, fixme
#the closer the signal angle gets to 90° the sharper the fall-off of the signal is.
#e.g. signal_angle=40°, app_angle=60° has not such a sharp fall-off as signal_angle=60°, app_angle=80°.
#this may be explained by the fact that the first hydrophone listens to angle 24°, and thus
#there is no additional input for angles less than 24°, decreasing total signal strength,
#blurring the measurement. Or, to describe it the other way round, for signals perpendicular to
#the hydrophone array, all hydrophones record the signal and add up to the total output, but for
#signals closer to bow/stern, less hydrophones record the noise and thus the total output gets weaker.
maxv = 0.0
minv = 0.0
maxvx = 0
minvx = 0
for x in range(0, samples):
  if signal[x] < minv:
    minv = signal[x]
    minvx = x
  if signal[x] > maxv:
    maxv = signal[x]
    maxvx = x
print 'minv=' + str(minv) + ', maxv=' + str(maxv) + ', minv at ' + str(minvx) + ', maxv at ' + str(maxvx)
print 'percentage of full signal: min=' + str(minv/max_strength) + ', max=' + str(maxv/max_strength)

# only if no cmd line given...
if len(sys.argv) == 1:
  plotoutput = plotoutput[:-1]
  f = open('gnuplotcmd.txt', 'wb')
  f.writelines([plotoutput, ''])
  f.close()
  print plotoutput
  os.system('gnuplot gnuplotcmd.txt -')

# summary: the fall-off is not very sharp with 1kHz, but gets much sharper at
# higher frequencies.
# 3kHz vs. 1kHz shows much difference.
# if we can compute the max. signal height automatically live would be easier...
# thats done. now we could pack in into one function, that computes
# the signal strength by frequency and angle.

# fixme: each hydrophone has a limited FOV, at least <= 180° and a different center of FOV,
# this is not yet simulated...
# signal gets weaker by cos(x) where x is angle difference between signal direction
# and membrane normal vector of hydrophone.
