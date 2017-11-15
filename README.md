# curses-life-gliders

Generates 'Conway's Game of Life' gliders within a terminal using Curses.

A new glider will spawn every 10 seconds or so, traveling southeast.

##### Options:
-w [number] Set playfield width (min 1, max 256, default 80)

-h [number] Set playfield height (min 1, max 256, default 25)

-start-gliders [number] Set number of gliders on the first tick (min 0, max 100, default 3)

-spawn-rate [number] Set the amount of time in tenths of a second before spawning a new glider (min 1, max 32767, default 100)


Ex:

./life

./life -w 100 -h 100

./life -w 150 -h 40 -start-gliders 8 -spawn-rate 40


##### Building:

###### Linux

gcc main.c -o life -lncurses

###### Windows
*TODO*
