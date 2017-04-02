#!/bin/bash

# plays '$2' matches
if (( $1 == 1 )); then
    awk "BEGIN {printf \"$1 4\"; while (c++ < $2 - 1) printf \"y\"; printf \"n\";}" | ./play
elif (( $1 == 2 )); then
    awk "BEGIN {printf \"$1 1\"; while (c++ < $2 - 1) printf \"y\"; printf \"n\";}" | ./play
fi

