#!/usr/bin/env bash

dftd="./build/linux/dangerdeep"
datadir="--datadir data"
options="--debug" # --maxfps 0"
sound="--nosound"

echo "Running [$dftd $options $datadir $sound]..."
$dftd $options $datadir $sound
