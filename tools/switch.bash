#!/bin/bash


mv src/connectn.c games/connectn/
mv src/play-connectn.c games/connectn/
mv include/connectn.h games/connectn/

mv src/sim.c games/sim/
mv src/play-sim.c games/sim/
mv include/sim.h games/sim/

mv src/zigzagzoe.c games/zigzagzoe/
mv src/play-zigzagzoe.c games/zigzagzoe/
mv include/zigzagzoe.h games/zigzagzoe/

mv games/$1/*.h include/
mv games/$1/*.c src/


