//#include <locale.h>
//#include "./curses.h"
//#include "./panel.h"
//#pragma comment(lib, "./pdcurses.lib")
#include <clocale>
#include <ncurses.h>
#include <pthread.h>
#include <unistd.h>
#include <string>
using namespace std;

int map[21][21];
int direction_x = -1; //left:-1, right:1
int direction_y; //up:-1, down:1
int head_y = 10;
int head_x = 9;
int tail_x[400];
int tail_y[400];
int tail_length = 2;
bool gameOver = false;

void setMap() {
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

	tail_y[0] = 10;
	tail_x[0] = 10;
	tail_y[1] = 10;
	tail_x[1] = 11;
}

void drawMap() {
	clear();
	int tail_position = 0;
	for (int i = 0; i < 21; i++) {
		for (int j = 0; j < 21; j++) {
			if (map[i][j] == 1 || map[i][j] == 2)
				mvprintw(i, j, "\u2B1B");
			else if (head_y == i && head_x == j) {
				start_color();
				init_pair(1, COLOR_YELLOW, COLOR_BLACK);

				attron(COLOR_PAIR(1));
				mvprintw(i, j, "\u2B1B");
				attroff(COLOR_PAIR(1));
			}
			else if (tail_position < tail_length && tail_y[tail_position] == i && tail_x[tail_position] == j) {
				tail_position++;
				start_color();
				init_pair(2, COLOR_MAGENTA, COLOR_BLACK);

				attron(COLOR_PAIR(2));
				mvprintw(i, j, "\u2B1B");
				attroff(COLOR_PAIR(2));
			}
			else if (map[i][j] == 0) {
				mvprintw(i, j, "\u2B1C");
			}
		}
	}

	mvprintw(24, 5, to_string(tail_x[1]).c_str());
	mvprintw(25, 5, to_string(tail_y[1]).c_str());
	refresh();
}

bool isGameOver() {
	if (gameOver)
		return true;
	if (head_x == 0 || head_x == 20 || head_y == 0 || head_y == 20) {
		return true;
	}
	return false;
}

void moveSnake() {
	int prevX = head_x;
	int prevY = head_y;

	head_x += direction_x;
	head_y += direction_y;

	for (int i = tail_length - 1; i > 0; i--) {
		tail_x[i] = tail_x[i - 1];
		tail_y[i] = tail_y[i - 1];
	}
	tail_x[0] = prevX;
	tail_y[0] = prevY;
}

void* getInput(void *arg) {
	pthread_t tid;
	tid = pthread_self();
	keypad(stdscr, TRUE);

	while (!isGameOver()) {
		int input = getch();
		//72 up, 80 down, 77 right, 75 left
		if (input == KEY_UP) {
			if (direction_y == 1)
				gameOver = true;
			direction_y = -1;
			direction_x = 0;
		}
		else if (input == KEY_DOWN) {
			if (direction_y == -1)
				gameOver = true;
			direction_y = 1;
			direction_x = 0;
		}
		else if (input == KEY_RIGHT) {
			if (direction_x == -1)
				gameOver = true;
			direction_x = 1;
			direction_y = 0;
		}
		else if (input == KEY_LEFT) {
			if (direction_x == 1)
				gameOver = true;
			direction_x = -1;
			direction_y = 0;
		}
	}
}

int main() {
	setlocale(LC_ALL, "");
	char key;
	char userName[8];

	initscr();
	curs_set(0);
	noecho();

	setMap();
	pthread_t thread;
	int thr_id = pthread_create(&thread, NULL, getInput, NULL);

	while (!isGameOver()) {
		drawMap();
		moveSnake();
		usleep(500000);
	}

	mvprintw(22, 5, "GameOver");
	
	getch();
	endwin();
	return 0;
}