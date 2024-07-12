#!/bin/bash

echo
echo
echo
echo
echo
date
gcc src/main.c -O0 -o bin/cwig.out && time bin/cwig.out
