#!/usr/bin/python
# compute time delay for signal of each hydrophone and produce a
# gnuplot command line
from math import *
import os
# -------------- constants -------------------
# speed of sound in water
speedwater = 1465.0
# nr of hydrophones per side, phones are numbered from bow to aft 0...nr-1
nr_hydrophones = 12
# distance between hydrophone membrane centers, roughly 20cm
distance_hydro = 0.2
# electrical delay, 17µs
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
# nr of strip lines
# line 0 is at bottom! so it has delay 0, line 99 has max. delay (99*17µs)
# this means the signals arriving first, at the highest line, are delayed by the
# highest value.
# the total y-coordinate range of the strips goes from -nr_strips/2 to +nr_strips/2,
# because the strip line array is turned around its center.
nr_strips = 100
# distance between two hydrophone output contacts on strip line array, in lines
distance_contact = delta_t / strip_delay
print 'distance between contacts on line array = ' + str(distance_contact)
height_of_all_contacts = distance_contact * (nr_hydrophones - 1)

# -------------- variables from here on -------
# signal direction (angle)
signal_angle = 90.0
# frequency of wave to measure, in Hz
noise_freq = 1000
# signal comes from 0° --------------
# here give ghg apparatus angle
app_angle = 60.0
app_angle_rad = app_angle * pi / 180.0
# factor for input of signal function sin(x)
# a full period is done noise_freq times per second
# time t is in seconds
# signal(t) = sin(t * time_scale_fac)
# x = 0 at t = 0, x = 2*pi at t = 1/noise_freq
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
  y_line = int(y + nr_strips/2)
  print 'i=' + str(i) + ' y=' + str(y) + ' y_line=' + str(y_line)
  delay_i = strip_delay * y_line
  delta_t_signal = cos(signal_angle * pi / 180.0) * delta_t
  print 'delta_t=' + str(delta_t) + ' delta_t_signal=' + str(delta_t_signal) + ' *i=' + str(i*delta_t_signal)
  # fixme : signal delay is i * delta_t ONLY when signal comes from 0° !!!
  signaldelay = delay_i + i * delta_t_signal
  print 'delay_i=' + str(delay_i) + ' signaldelay=' + str(signaldelay)
  plotoutput += str(strength_factor) + '*sin(x'
  plotfac = time_scale_fac * signaldelay
  if (plotfac < 0.0):
    plotoutput += str(plotfac)
  else:
    plotoutput += '+' + str(plotfac)
  plotoutput += ')+'
  # measuring max. signal peak
  for x in range(0, samples):
    sinfac = float(x)*2*pi/samples + plotfac
    signal[x] += strength_factor * sin(sinfac)

plotoutput = 'plot [x=0:7] ' + str(max_strength) + '*sin(x),' + plotoutput

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
for x in range(0, samples):
  if signal[x] < minv:
    minv = signal[x]
  if signal[x] > maxv:
    maxv = signal[x]
print 'minv=' + str(minv) + ', maxv=' + str(maxv)
print 'percentage of full signal: min=' + str(minv/max_strength) + ', max=' + str(maxv/max_strength)

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
