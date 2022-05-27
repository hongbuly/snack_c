#include "myMap.h"

myMap::myMap() {
	for (int i = 0; i < 21; i++) {
		for (int j = 0; j < 21; j++) {
			if (i == 0 || i == 20)
				map[i][j] = 1;
			else {
				map[i][j] = 1;
				j += 19;
			}
		}
	}
	map[0][0] = 2;
	map[20][20] = 2;
	map[20][0] = 2;
	map[0][20] = 2;

	map[10][9] = 3;
	map[10][10] = 4;
	map[10][11] = 4;
}

void myMap::drawMap() {
	for (int i = 0; i < 21; i++) {
		for (int j = 0; j < 21; j++) {
			if (map[i][j] == 1 || map[i][j] == 2)
				mvprintw(i, j, "\u2B1B");
			else if (map[i][j] == 0)
				mvprintw(i, j, "\u2B1C");
			else if (map[i][j] == 3) {
				start_color();
				init_pair(1, COLOR_YELLOW, COLOR_BLACK);

				attron(COLOR_PAIR(1));
				mvprintw(i, j, "\u2B1B");
				attroff(COLOR_PAIR(1));
			}
			else if (map[i][j] == 4) {
				start_color();
				init_pair(2, COLOR_MAGENTA, COLOR_BLACK);

				attron(COLOR_PAIR(2));
				mvprintw(i, j, "\u2B1B");
				attroff(COLOR_PAIR(2));
			}

		}
	}
	refresh();
}