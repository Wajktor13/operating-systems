#include <ncurses.h>
#include <locale.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "grid.h"


int main()
{
	srand(time(NULL));
	setlocale(LC_CTYPE, "");
	initscr(); // Start curses mode

	char *foreground = create_grid();
	char *background = create_grid();
	char *tmp;

	init_grid(foreground);

	bool create_threads = true;

	while (true)
	{
		draw_grid(foreground);
		usleep(500 * 1000);

		// Step simulation
		threaded_update_grid(foreground, background, create_threads);
		tmp = foreground;
		foreground = background;
		background = tmp;

		create_threads = false;
	}

	endwin(); // End curses mode
	destroy_grid(foreground);
	destroy_grid(background);

	return 0;
}
