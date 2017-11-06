#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "curses.h"

#define SCREEN_W 80
#define SCREEN_H 25

void wrap_coordinates( int * x, int * y ) {
    int xx = *x;
    int yy = *y;

    if (xx < 0 ) {
        xx = SCREEN_W - (abs(xx) % SCREEN_W);
    }
    if (xx >= SCREEN_W ) {
        xx = xx % SCREEN_W;
    }
    if (yy < 0 ) {
        yy = SCREEN_H - (abs(yy) % SCREEN_H);
    }
    if (yy >= SCREEN_H ) {
        yy = yy % SCREEN_H;
    }

    *x = xx;
    *y = yy;
}

int check_neighbour( int x, int y, char conmap[SCREEN_W][SCREEN_H]) {

    int *px = &x;
    int *py = &y;

    wrap_coordinates(px,py);
    if ( conmap[x][y] == '#' ) {
        return 1;
    }

    return 0;
}

int count_neighbours( int sx, int sy, char conmap[SCREEN_W][SCREEN_H]) {
    int tally = 0;

	int x, y;
	for( x = -1; x < 2; x++ ) {
		for( y = -1; y < 2; y++ ) {
			if( x == 0 && y == 0 ) {
				continue;
			}
			tally += check_neighbour( sx + x, sy + y, conmap );
		}
	}

    return tally;
}


int plot_glider( char conmap[SCREEN_W][SCREEN_H]) {
    int x, y, i, j, running;

    x = rand() % SCREEN_W;
    y = rand() % SCREEN_H;

    conmap[x][y]     = '#';
    conmap[x+1][y+1] = '#';
    conmap[x+1][y+2] = '#';
    conmap[x][y+2]   = '#';
    conmap[x-1][y+2] = '#';

}

void wipe_map(char conmap[SCREEN_W][SCREEN_H]) {
    int i, j;

    for(i = 0; i < SCREEN_W; i++ ) {
        for(j = 0; j < SCREEN_H; j++) {
            conmap[i][j] = ' ';
        }
    }
}

int main()
{
    srand(time(NULL));

    initscr();  // Initialize Curses window
    raw();      // Disable line buffering and control characters (CTRL+C / CTRL+Z)
    noecho();   // Do not show characters typed by user into terminal
    keypad(stdscr, TRUE);   // enable input from function keys and arrow keys
    curs_set(FALSE);    // Turn off the blinking cursor


    int run = 1;

    int countdown = 100;

    char cmap[SCREEN_W][SCREEN_H];  // Current map
    char nmap[SCREEN_W][SCREEN_H];  // New map, contents overwrite current map every cycle.

    // general purpose iterators.
    int i, j;

    wipe_map(cmap);
    wipe_map(nmap);

    // add a glider
    for(i = 0; i < 3; i++ ) {
        plot_glider(cmap);
    }

    halfdelay(1);	// Update at 10 frames per second

    int in = ERR;

	/* Main loop */
    while(run) {

        countdown--;

        if(countdown <= 0) {
            plot_glider(cmap);
            countdown = 100;
        }

        // Process cells

        int n_neighbours = 0;
        int c_glyph = 0;

        for(i = 0; i < SCREEN_W; i++ ) {
            for( j = 0; j < SCREEN_H; j++ ) {
                n_neighbours = count_neighbours(i,j,cmap);
                c_glyph = cmap[i][j];

                if( c_glyph == '#' ) {
                    // a) Living cells with fewer than two neighbours die
                    if( n_neighbours < 2 ) {
                        nmap[i][j] = ' ';
                    }

                    // b) Living cells with 2 or 3 neighbours persist
                    else if( n_neighbours  >= 2 && n_neighbours <= 3 ) {
                        nmap[i][j] = '#';
                    }

                    // c) Living cells with more than 3 neighbours die
                    else if( n_neighbours  > 3 ) {
                        nmap[i][j] = ' ';
                    }
                }
                // d) Dead cells with 3 neighbours come to life
                else if( c_glyph == ' ') {
                    if( n_neighbours == 3 ) {
                        nmap[i][j] = '#';
                    }
                    // Otherwise, dead cells remain dead
                    else {
                        nmap[i][j] = ' ';
                    }
                }
            }
        }

        // Copy contents of nmap to cmap

        for(i = 0; i < SCREEN_W; i++ ) {
            for(j = 0; j < SCREEN_H; j++ ) {
                cmap[i][j] = nmap[i][j];
            }
        }
        wipe_map(nmap);


        // Draw map
		clear();	// TODO: Seems to cause flickering in Windows 10 / PDCurses
        for(i = 0; i < SCREEN_W; i++) {
            for(j = 0; j < SCREEN_H; j++ ) {
                mvaddch(j,i,cmap[i][j]);

            }
        }
		refresh();

        // Quit if input detected

        in = getch();

        if( in != ERR ) {
            run = 0;
        }
    }

	/* Shutdown */ 
    endwin();
    return 0;
}
