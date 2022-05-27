//#include <locale.h>
//#include "./curses.h"
//#include "./panel.h"
//#pragma comment(lib, "./pdcurses.lib")
#include "myMap.h"
using namespace std;

int main() {
	myMap mp;

	setlocale(LC_ALL, "");
	char key;
	char userName[8];

	initscr();

	mp.drawMap();
	
	// keypad(stdscr, TRUE);
	curs_set(0);
	noecho();
	// scanw("%s", userName);
	
	getch();
	endwin();
	return 0;
}