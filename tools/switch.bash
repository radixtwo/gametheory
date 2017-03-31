#!/bin/bash


mv src/connectn.c games/connectn/
mv src/play-connectn.c games/connectn/
mv lib/connectn.h games/connectn/

mv src/sim.c games/sim/
mv src/play-sim.c games/sim/
mv lib/sim.h games/sim/

mv src/zigzagzoe.c games/zigzagzoe/
mv src/play-zigzagzoe.c games/zigzagzoe/
mv lib/zigzagzoe.h games/zigzagzoe/

mv games/$1/*.h lib/
mv games/$1/*.c src/


