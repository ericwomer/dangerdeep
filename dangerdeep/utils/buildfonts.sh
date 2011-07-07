#!/bin/bash
./makefont ../arial.ttf 16 ../data/fonts/font_arial
./makefont ../myoldrem.ttf 21 ../data/fonts/font_typenr16
./makefont ../jphsl.ttf 24 ../data/fonts/font_jphsl
./makefont ../vtRemingtonPortable.ttf 10 ../data/fonts/font_vtremington8
./makefont ../vtRemingtonPortable.ttf 13 ../data/fonts/font_vtremington10
./makefont ../vtRemingtonPortable.ttf 16 ../data/fonts/font_vtremington12
./makefont ../vtRemingtonPortable.ttf 18 ../data/fonts/font_vtremington14
for i in font_arial font_typenr16 font_jphsl font_vtremington8 font_vtremington10 font_vtremington12 font_vtremington14; do convert ../data/fonts/$i.pgm ../data/fonts/$i.png; rm ../data/fonts/$i.pgm; done
