#include <clocale>
#include <ncurses.h>
#include <pthread.h>
#include <unistd.h>
#include <string>
#include <cstdlib>
#include <ctime>
#include <algorithm>
using namespace std;

int map[21][21]; // 보드 크기
int direction_x = -1; //left:-1, right:1
int direction_y; //up:-1, down:1

// 머리 위치
int head_y = 10; int head_x = 9; 

// 몸통 크기
int tail_x[400];
int tail_y[400];
int tail_length = 2;

bool gameOver = false;

// Item 위치
int itemG_x; int itemG_y;
int itemP_x; int itemP_y;

// Gate 위치
int gateA_x; int gateA_y;
int gateB_x; int gateB_y;

// Gate 통과중인지 여부
int passGate = 0;

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
	// 꼭지점
	map[0][0] = 2;
	map[20][20] = 2;
	map[20][0] = 2;
	map[0][20] = 2;
    // 초기 몸통 위치 0[10, 10], 1[10, 11]
	tail_y[0] = 10;
	tail_x[0] = 10;
	tail_y[1] = 10;
	tail_x[1] = 11;
}

bool isTailPosition(int tail_position, int i, int j) {
	for (int k = tail_position; k < tail_length; k++) {
		if (tail_y[k] == i && tail_x[k] == j) {
			return true;
		}
	}
	return false;
}

void drawMap() {
	clear();
	int tail_position = 0;
	for (int i = 0; i < 21; i++) {
		for (int j = 0; j < 21; j++) {
		    // 아이템G
		    if (map[i][j] == 5) {
				mvprintw(i, j, "\U0001F7E9");
			}
			// 아이템P
			else if (map[i][j] == 6) {
				mvprintw(i, j, "\U0001F7E5");
			}
			// 게이트
			else if (map[i][j] == 7 || map[i][j] == 8)
				mvprintw(i, j, "\U0001F7EA");
			// 외곽선
			if (map[i][j] == 1 || map[i][j] == 2)
				mvprintw(i, j, "\u2B1B");
			// 머리
			else if (head_y == i && head_x == j) {
				start_color();
				init_pair(1, COLOR_YELLOW, COLOR_BLACK);

				attron(COLOR_PAIR(1));
				mvprintw(i, j, "\U0001F7E8");
				attroff(COLOR_PAIR(1));
			}
			// 몸통
			else if (map[i][j] == 3)
			{
				tail_position++;
				start_color();
				init_pair(2, COLOR_MAGENTA, COLOR_BLACK);

				attron(COLOR_PAIR(2));
				mvprintw(i, j, "\u2B1B");
				attroff(COLOR_PAIR(2));
			}
			// 빈공간
			else if (map[i][j] == 0) {
				mvprintw(i, j, "\u2B1C");
			}
		}
	}
	refresh();
}

bool isGameOver() {
	if (gameOver)
		return true;
	if (head_x == 0 || head_x == 20 || head_y == 0 || head_y == 20) { // snake가 외벽에 닿을 시
		return true;
	}
	if (tail_length < 2) return true; // 몸통의 길이가 2보다 작아질 경우
	return false;
}

// moveSnake함수에 사용되는 함수
void moveTails(int prevX, int prevY, int removeX, int removeY) {
    for (int i = tail_length - 1; i > 0; i--) {
    	tail_x[i] = tail_x[i - 1];
	   	tail_y[i] = tail_y[i - 1];
    }
    tail_x[0] = prevX;
    tail_y[0] = prevY;
    
    for (int i = 0; i < tail_length; i++)
        map[tail_y[i]][tail_x[i]] = 3;
    map[removeY][removeX] = 0;
}

void moveSnake() {
	int prevX = head_x;
	int prevY = head_y;
	int removeX = tail_x[tail_length - 1];
	int removeY = tail_y[tail_length - 1];

	head_x += direction_x;
	head_y += direction_y;
	
	passGate--; if(passGate<=0) passGate = 0;
	
	// Head가 아이템G에 닿았을 때
	if(map[head_y][head_x] == 5) {
	    tail_length++;
	    for (int i = tail_length - 1; i > 0; i--) {
		    tail_x[i] = tail_x[i - 1];
		    tail_y[i] = tail_y[i - 1];
	    }
	    tail_x[0] = prevX;
	    tail_y[0] = prevY;
	    
	    for (int i = 0; i < tail_length; i++)
            map[tail_y[i]][tail_x[i]] = 3;
        // 아이템G 획득 시 몸의 길이가 진행방향으로 증가하므로 마지막 꼬리를 삭제 X
	    // map[removeY][removeX] = 0;
	    map[head_y][head_x] = 0;
	}
	// Head가 아이템P에 닿았을 때
	else if(map[head_y][head_x] == 6) {
	    map[removeY][removeX] = 0;
	    tail_length--;
	    
	    removeX = tail_x[tail_length - 1];
	    removeY = tail_y[tail_length - 1];
	    
	    moveTails(prevX, prevY, removeX, removeY);
        map[head_y][head_x] = 0;
	}
	// Head가 게이트가 닿았을 때
	else if(map[head_y][head_x] == 7) {
	    passGate += tail_length;
        if(gateB_x == 0) { // (좌에서 우)
            head_x = gateB_x + 1;
            head_y = gateB_y;
            direction_x = 1;
            direction_y = 0;
        }
        else if(gateB_x == 20) { // (우에서 좌)
            head_x = gateB_x - 1;
            head_y = gateB_y;
            direction_x = -1;
            direction_y = 0;
        }
        else if(gateB_y == 0) { // (상에서 하)
            head_x = gateB_x;
            head_y = gateB_y + 1;
            direction_x = 0;
            direction_y = 1;
        }
        else if(gateB_y == 20) { // (하에서 상)
            head_x = gateB_x;
            head_y = gateB_y - 1;
            direction_x = 0;
            direction_y = -1;
        }
        else { // 벽이 아닌 곳에 게이트가 있을 경우 진행 방향 그대로
            head_x = gateA_x + direction_x;
            head_y = gateA_y + direction_y;
        }
        moveTails(prevX, prevY, removeX, removeY);
	}
	else if(map[head_y][head_x] == 8) {
	    passGate += tail_length;
        if(gateA_x == 0) { // (좌에서 우)
            head_x = gateA_x + 1;
            head_y = gateA_y;
            direction_x = 1;
            direction_y = 0;
        }
        else if(gateA_x == 20) { // (우에서 좌)
            head_x = gateA_x - 1;
            head_y = gateA_y;
            direction_x = -1;
            direction_y = 0;
        }
        else if(gateA_y == 0) { // (상에서 하)
            head_x = gateA_x;
            head_y = gateA_y + 1;
            direction_x = 0;
            direction_y = 1;
        }
        else if(gateA_y == 20) { // (하에서 상)
            head_x = gateA_x;
            head_y = gateA_y - 1;
            direction_x = 0;
            direction_y = -1;
        }
        else { // 벽이 아닌 곳에 게이트가 있을 경우 진행 방향 그대로
            head_x = gateA_x + direction_x;
            head_y = gateA_y + direction_y;
        }
        moveTails(prevX, prevY, removeX, removeY);
	}
	else {
	    moveTails(prevX, prevY, removeX, removeY);
    }
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
	return arg;
}

void getItemG() {
    if(itemG_y != 0) map[itemG_y][itemG_x] = 0; // 보드에 이미 아이템G가 있으면 그 아이템 삭제
    int x, y;
    srand((unsigned int)time(NULL));
    
    y = rand() %20 + 1; if (y>19) y = 19;
    x = rand() %20 + 1; if (x>19) x = 19;
    
    while(count(tail_x, tail_x+tail_length, x) != 0 && count(tail_y, tail_y+tail_length, y) != 0) {
        y = rand() %20 + 1; if (y>19) y = 19;
        x = rand() %20 + 1; if (x>19) x = 19;
    }
    itemG_y = y;
    itemG_x = x;
    map[y][x] = 5;
}

void getItemP() {
    if(itemP_y != 0) map[itemP_y][itemP_x] = 0;
    int x, y;
    srand((unsigned int)time(NULL));
    
    y = rand() %20 + 1; if (y>19) y = 19;
    x = rand() %20 + 1; if (x>19) x = 19;
    
    while( (count(tail_x, tail_x+tail_length, x) != 0 && count(tail_y, tail_y+tail_length, y) != 0) || (x == itemG_x && y == itemG_y) ) {
        y = rand() %20 + 1; if (y>19) y = 19;
        x = rand() %20 + 1; if (x>19) x = 19;
    }
    itemP_y = y;
    itemP_x = x;
    map[y][x] = 6;
}

void setGate() {
    if(passGate > 0) return;
    if(gateA_y != 0 || gateA_x != 0) map[gateA_y][gateA_x] = 1;
    if(gateB_y != 0 || gateB_x != 0) map[gateB_y][gateB_x] = 1;
    int a, b, c, d;
    
    srand((unsigned int)time(NULL));
    a = rand() %21; b = rand() %21;
    while(map[b][a] != 1) {
        a = rand() %21;
        b = rand() %21;
    }
    gateA_y = b;
    gateA_x = a;
    map[b][a] = 7;
    
    c = rand() %21; d = rand() %21;
    while( (map[d][c] != 1) && (map[d][c] != map[b][a]) ) {
        c = rand() %21;
        d = rand() %21;
    }
    gateB_y = d;
    gateB_x = c;
    map[d][c] = 8;
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

    int t = 0;
    
	while (!isGameOver()) {
	    if (t >= 20 && t % 20 == 0) { getItemG(); getItemP(); }
	    if (tail_length >= 5 && t % 20 == 0) { setGate(); }
	    moveSnake();
		drawMap();
		usleep(500000);
		t += 1;
	}

	mvprintw(22, 5, "GameOver");

	getch();
	endwin();
	return 0;
}
