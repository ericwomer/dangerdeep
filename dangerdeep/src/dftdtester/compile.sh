#!/bin/bash

FOUT=gltest.bin

rm -f *.o $FOUT

g++ -c tests.cpp -Wall
g++ -c main.cpp -Wall
g++ -ldl -Wall tests.o main.o -o $FOUT
