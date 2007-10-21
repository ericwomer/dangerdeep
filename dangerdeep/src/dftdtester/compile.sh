#!/bin/bash

FOUT=gltest.bin

rm -f *.o $FOUT

g++ -c -g tests.cpp -Wall
g++ -c -g main.cpp -Wall
g++ -c -g x11.cpp -Wall

g++ -ldl -Wall -g tests.o main.o x11.o -o $FOUT
