#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "curses.h"

#define SCREEN_W 80
#define SCREEN_H 25

#define CELL_LIVING '#'
#define CELL_DEAD ' '

typedef struct Field_t {
	int w;
	int h;
	int * data;
} Field;

#define FIELD_W_MIN 1
#define FIELD_W_MAX 256
#define FIELD_W_DEF 80
#define FIELD_H_MIN 1
#define FIELD_H_MAX 256
#define FIELD_H_DEF 25

int confirmDimensions( int w, int h ) {
	return (w >= FIELD_W_MIN && h >= FIELD_H_MIN && w <= FIELD_W_MAX - 1 && h <= FIELD_H_MAX - 1);
}

void wrapCoordinates( int * x, int * y, int w, int h ) {
    int xx = *x;
    int yy = *y;

    if (xx < 0 ) {
        xx = w - ( abs(xx) % w );
    }
    if (xx >= w ) {
        xx = xx % w;
    }
    if (yy < 0 ) {
        yy = h - ( abs(yy) % h );
    }
    if (yy >= h ) {
        yy = yy % h;
    }

    *x = xx;
    *y = yy;
}

void setCell( int cell_type, int x, int y, Field * f ) {
	int *px = &x;
	int *py = &y;
	wrapCoordinates( px, py, f->w, f->h );
	
	f->data[ x + y * f->w ] = cell_type;
}

int getCell( int x, int y, Field * f ) {
    int *px = &x;
    int *py = &y;
    wrapCoordinates( px, py, f->w, f->h );

	return f->data[ x + y * f->w ];
}

void fieldWipe( Field * f ) {
    int i, j;

    for(i = 0; i < f->w; i++ ) {
        for(j = 0; j < f->h; j++) {
			setCell( CELL_DEAD, i, j, f );
        }
    }
}

Field * fieldInit( int w, int h ) {
	// Fail upon invalid supplied dimensions
	if( !confirmDimensions( w, h ) ) {
		return NULL;
	}

	Field * f = malloc( sizeof(Field) );
	if( !f ) {
		return NULL;
	}
	
	f->w = w;
	f->h = h;

	f->data = malloc( sizeof(int) * w * h );
	if( !f->data ) {
		return NULL;
	}

	fieldWipe( f );

	return f;
}

void fieldTearDown( Field * f ) {
	if( f ) {
		free( f->data );
		free( f );
	}
}

int countNeighbours( int sx, int sy, Field * f ) {
    int tally = 0;
	
	int x, y;

	for( x = -1; x < 2; x++ ) {
		for( y = -1; y < 2; y++ ) {
			if( x == 0 && y == 0 ) {
				continue;
			}
			if( getCell( sx + x, sy + y, f ) == '#' ) {
				tally++;
			}
		}
	}

    return tally;
}

void plotGlider( Field * f ) {
	int breakpoint = 'y';
    int x, y;

    x = rand() % f->w;
    y = rand() % f->h;
	
	setCell( CELL_LIVING, x,     y,     f );
	setCell( CELL_LIVING, x + 1, y + 1, f );
	setCell( CELL_LIVING, x + 1, y + 2, f );
	setCell( CELL_LIVING, x,     y + 2, f );
	setCell( CELL_LIVING, x - 1, y + 2, f );
}


// Curses drawing
void fieldDraw( Field * f ) {
	int x, y;
	clear();	// TODO: Seems to cause flickering in Windows 10 / PDCurses
    for( x = 0; x < f->w; x++ ) {
        for( y = 0; y < f->h; y++ ) {
				mvaddch( y, x, getCell( x, y, f ) );
        }
    }
	refresh();
}

int main( int argc, char *argv[] ) {
    srand(time(NULL));		// Seed randomizer
    initscr();  			// Initialize Curses window
    raw();      			// Disable line buffering and control characters (CTRL+C / CTRL+Z)
    noecho();   			// Do not show characters typed by user into terminal
    keypad(stdscr, TRUE);   // enable input from function keys and arrow keys
    curs_set(FALSE);    	// Turn off the blinking cursor
	halfdelay(1);			// Update at about 10 frames per second

    int run = 1;
	int countdown_refresh = 100;
    int countdown = countdown_refresh;
	int start_gliders = 3;
	int key_in = ERR;

	int width = FIELD_W_DEF;
	int height = FIELD_H_DEF;

	int arg_walk = 0;
	int arg_action = 0;
	#define ARG_SPAN 32
	#define ARG_NOTHING 0
	#define ARG_SET_WIDTH 1
	#define ARG_SET_HEIGHT 2

	if( argc > 1 ) {
		for( arg_walk = 0; arg_walk < argc; arg_walk++ ) {
			if( arg_action == ARG_NOTHING ) {
				if( strncmp( argv[arg_walk], "-w", ARG_SPAN ) == 0 ) {
					arg_action = ARG_SET_WIDTH;
					continue;
				}
				if( strncmp( argv[arg_walk], "-h", ARG_SPAN ) == 0 ) {
					arg_action = ARG_SET_HEIGHT;
					continue;
				}
			}
			if( arg_action == ARG_SET_WIDTH ) {
				width = atoi( argv[arg_walk] );
				arg_action = ARG_NOTHING;
				continue;
			}
			if( arg_action == ARG_SET_HEIGHT ) {
				height = atoi( argv[arg_walk] );
				arg_action = ARG_NOTHING;
				continue;
			}
		}
	}
	
	if( !confirmDimensions( width, height ) ) {
		printf( "Error: Couldn't use dimensions: w %d, h %d\n", width, height );
	}

	Field * c_map = fieldInit( width, height );
	if( !c_map ) {
		printf( "malloc() failed on c_map.\n" );
		exit(1);
	}
	Field * n_map = fieldInit( width, height );
	if( !n_map ) {
		printf( "malloc() failed on n_map.\n" );
		exit(1);
	}
	
    int i, j;

    // Add some gliders to start
    for(i = 0; i < start_gliders; i++ ) {
        plotGlider(c_map);
    }

	/* Main loop */
    while(run) {

		countdown--;
        
		if(countdown <= 0) {
            plotGlider(c_map);
            countdown = countdown_refresh;
        }

        // Process cells
        int n_neighbours = 0;
        int c_glyph = 0;

        for(i = 0; i < SCREEN_W; i++ ) {
            for( j = 0; j < SCREEN_H; j++ ) {
                n_neighbours = countNeighbours( i, j, c_map );
				c_glyph = getCell( i, j, c_map );

                if( c_glyph == '#' ) {
                    // a) Living cells with fewer than two neighbours die
                    if( n_neighbours < 2 ) {
						setCell( CELL_DEAD, i, j, n_map );
                    }

                    // b) Living cells with 2 or 3 neighbours persist
                    else if( n_neighbours  >= 2 && n_neighbours <= 3 ) {
						setCell( CELL_LIVING, i, j, n_map );
                    }

                    // c) Living cells with more than 3 neighbours die
                    else if( n_neighbours  > 3 ) {
						setCell( CELL_DEAD, i, j, n_map );
                    }
                }
                // d) Dead cells with 3 neighbours come to life
                else if( c_glyph == ' ') {
                    if( n_neighbours == 3 ) {
						setCell( CELL_LIVING, i, j, n_map );
                    }
                    // Otherwise, dead cells remain dead
                    else {
                        setCell( CELL_DEAD, i, j, n_map );
                    }
                }
            }
        }

        // Copy contents of n_map to c_map
        for(i = 0; i < c_map->w; i++ ) {
            for(j = 0; j < c_map->h; j++ ) {
				setCell( getCell( i, j, n_map ), i, j, c_map );
            }
        }
        fieldWipe( n_map );

		fieldDraw( c_map );

        // Quit if any input detected
        key_in = getch();
        if( key_in != ERR ) {
            run = 0;
        }
    }

	/* Shutdown */ 

	fieldTearDown( c_map );
	fieldTearDown( n_map );

    endwin();
    return 0;
}
