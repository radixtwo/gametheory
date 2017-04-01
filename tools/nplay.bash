#!/bin/bash

# plays '$1' matches
awk "BEGIN {printf \"4\"; while (c++ < $1 - 1) printf \"y\"; printf \"n\";}" | ./play

