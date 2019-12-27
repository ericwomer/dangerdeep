#!/bin/bash

FOUT=gltest.bin
DEBUG="-Os -m32"

rm -f *.o $FOUT

g++ -c $DEBUG tests.cpp -Wall
g++ -c $DEBUG main.cpp -Wall
g++ -c $DEBUG x11.cpp -Wall

g++ -ldl -Wall $DEBUG tests.o main.o x11.o -o $FOUT
