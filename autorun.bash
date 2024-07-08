#!/bin/bash

find . -name '*.c' -or -name '*.h' | entr ./run.bash
