// gcc termbox_fire.c -o termbox_fire
#include "termbox.h"

#define AMOUNTOFRAIN 50 // The number of raindrops

/**
 *  Defines a raindrop.
 **/
typedef struct raindrop {
	short yLocation;/* Y co-ordinate. */
	short xLocation;/* X co-ordinate. */
} raindrop;

raindrop raindrops[AMOUNTOFRAIN];/* Contains an array of each raindrop. */
#if 0
WINDOW *sky;/* This will form the sky above the cities, the majority of the game will happen here. */
WINDOW *landscape;/* This will form the landscape at the bottom of the screen, including cities. */
bool house = false;/* Does the user want to see a house? */
bool pond = false;/* Does the user want to see a pond? */

/**
 * Structure for a house.
 **/
void addHouse()
{
	/* Draw the roof. */
	wattron(landscape, COLOR_PAIR(4));/* Initialise Black-on-red colour scheme. */
	mvwaddch(landscape, 0, (COLS / 3) * 2, ' ');
	mvwprintw(landscape, 1, (COLS / 3) * 2 - 1, "   ");
	mvwprintw(landscape, 2, (COLS / 3) * 2 - 2, "     ");
	/* Draw the rest of the building. */
	wattron(landscape, COLOR_PAIR(5));/* Initialise Black-on-yellow colour scheme. */
	mvwprintw(landscape, 3, (COLS / 3) * 2 - 1, "   ");
	mvwaddch(landscape, 4, (COLS / 3) * 2 - 1, ' ');
	mvwaddch(landscape, 4, (COLS / 3) * 2 + 1, ' ');
	wattron(landscape, COLOR_PAIR(2));/* Initialise Black-on-black colour scheme. */
	mvwaddch(landscape, 4, (COLS / 3) * 2, ' ');
}

/**
 * Structure for a pond.
 **/
void addPond()
{
	/* Draw the roof. */
	wattron(landscape, COLOR_PAIR(1));/* Initialise Black-on-blue colour scheme. */
	mvwprintw(landscape, 1, COLS / 3, "   ");
	mvwprintw(landscape, 2, COLS / 3 - 1, "     ");
	mvwprintw(landscape, 3, COLS / 3, "   ");
}

/**
 * Replaces the OS terminal colour scheme with our own, and declares the colour pairs we'll need.
 **/
void prepareScreen()
{
	init_pair(1, COLOR_BLACK, COLOR_BLUE);/* Set the first colour pair to black-on-blue. */
	init_pair(2, COLOR_BLACK, COLOR_BLACK);/* Set black-on-black colour scheme. */
	init_pair(3, COLOR_BLACK, COLOR_GREEN);/* Set black-on-green colour scheme. */
	init_pair(4, COLOR_BLACK, COLOR_RED);/* Set black-on-red colour scheme. */
	init_pair(5, COLOR_BLACK, COLOR_YELLOW);/* Set black-on-yellow colour scheme. */

	/* Clear sky to pair 1 */
	wattron(sky, COLOR_PAIR(2));
	for (short yAxis = 0; yAxis < LINES - 5; yAxis++) {
		for (short xAxis = 0; xAxis < COLS; xAxis++) {
			mvwaddch(sky, yAxis, xAxis, ' ');        /* Fill every character on the sky window with a black space. */
		}
	}
	wrefresh(sky);
	wattroff(sky, COLOR_PAIR(2));
	wattron(sky, COLOR_PAIR(1));/* Initialise Black-on-black colour scheme. */

	/* Clear landscape to pair 3 */
	wattron(landscape, COLOR_PAIR(3));/* Initialise Black-on-green colour scheme. */
	for (short yaxis = 0; yaxis < 5; yaxis++) {
		for (short xaxis = 0; xaxis < COLS; xaxis++) {
			mvwaddch(landscape, yaxis, xaxis, ' ');        /* Fill every character on the landscape window with a green space. */
		}
	}

	if (house) { /* If a house has been selected by the user, add one. */
		addHouse();
	}

	if (pond) { /* If a pond has been selected by the user, add one. */
		addPond();
	}

	wrefresh(landscape);
}
#endif

/**
 * Time-based Pseudo-random number generation.
 * @param min - The minimum possible number to choose from.
 * @param max - The maximum possible number to choose from.
 * @returns a pseudo-random number.
 **/
short rNG(short min, short max)
{
	return (rand() % (max - min + 1)) + min;
}

/**
 * Handles the animation and escape conditions.
 **/
void rain()
{
#define DOOM_STEPS 13
	static struct tb_cell fire[DOOM_STEPS] =
	{
		{' ', 9, 0}, // default
		{0x2591, 2, 0}, // red
		{0x2592, 2, 0}, // red
		{0x2593, 2, 0}, // red
		{0x2588, 2, 0}, // red
		{0x2591, 4, 2}, // yellow
		{0x2592, 4, 2}, // yellow
		{0x2593, 4, 2}, // yellow
		{0x2588, 4, 2}, // yellow
		{0x2591, 8, 4}, // white
		{0x2592, 8, 4}, // white
		{0x2593, 8, 4}, // white
		{0x2588, 8, 4}, // white
	};

	int w = tb_width();
	int h = tb_height();

	/* Set the x- and y-axis co-ordinates of each raindrop. */
	for (short drop = 0; drop < AMOUNTOFRAIN; drop++) {
		raindrops[drop].xLocation = rNG(0, w);
		raindrops[drop].yLocation = rNG(0, h - 3);
	}
	while (1) {
		struct tb_event ev;
		int t = tb_peek_event(&ev, 70);

		if (t == -1) break;
		if (t == TB_EVENT_KEY) break;

		tb_clear();
		struct tb_cell* buf = tb_cell_buffer();

		for (short drop = 0; drop < AMOUNTOFRAIN; drop++) {
			buf[w * raindrops[drop].yLocation + raindrops[drop].xLocation] = fire[10]; // Draw a raindrop

			/* Remove the raindrop from it's previous location. */
//			wattroff(sky, COLOR_PAIR(1));
//			wattron(sky, COLOR_PAIR(2));
			if (raindrops[drop].yLocation == 0) {
//				mvwaddch(sky, LINES - 6, raindrops[drop].xLocation, ' ');
				buf[w * (h-6) + raindrops[drop].xLocation] = fire[0]; // Draw a raindrop
			} else {
//				mvwaddch(sky, raindrops[drop].yLocation - 1, raindrops[drop].xLocation, ' ');
				buf[w * (raindrops[drop].yLocation -1) + raindrops[drop].xLocation] = fire[5]; // Draw a raindrop
			}
//			wattroff(sky, COLOR_PAIR(2));
//			wattron(sky, COLOR_PAIR(1));

			if (raindrops[drop].yLocation < h-6) {
				raindrops[drop].yLocation++;        /* If the raindrop isn't at the bottom of the window, move it down one pixel. */
			} else {
				raindrops[drop].yLocation = 0;        /* Move it to the top of the window to begin again. */
			}
		}

		tb_print("Rain demo!", 33, 1, TB_MAGENTA | TB_BOLD, TB_DEFAULT);
		tb_present();
	}
}

int main(int argc, char** argv)
{
#if 0
	if (argc > 1) { /* There are arguments */
		for (int args = 0; args < argc; args++) {
			if (argv[args][0] == '-') { /* Search for argument list. */
				for (short count = 1; count < strlen(argv[args]); count++) { /* Iterate through the list. */
					switch (argv[args][count]) {
					case 'h':
						house = true;/* The user has requested a house. */
						break;
					case 'p':
						pond = true;/* The user has requested a pond. */
						break;
					default:/* The user has input an invalid argument.  Return that to the user and fail. */
						printf("Invalid argument: %c\n", argv[args][count]);
						return EXIT_FAILURE;
					}
				}
			}
		}
	}
#endif
	tb_init();
	tb_select_output_mode(TB_OUTPUT_NORMAL);
	tb_clear();

	rain();

	tb_shutdown();

	return EXIT_SUCCESS;/* Close the program with a success signal. */
}
