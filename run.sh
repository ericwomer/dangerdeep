#!/usr/bin/env bash

dftd="./build/src/dangerdeep"
datadir="--datadir data"
options="--debug" # --maxfps 0"
sound=""

echo "Running [$dftd $options $datadir $sound]..."
$dftd $options $datadir $sound
