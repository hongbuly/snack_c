#include <clocale>
#include <ncurses.h>
#include <pthread.h>
#include <unistd.h>
#include <string>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <deque>
using namespace std;

int map[21][21]; // 보드 크기
int direction_x = -1; //left:-1, right:1
int direction_y; //up:-1, down:1

// 머리 위치
int head_y = 10; int head_x = 9;

// 몸통 크기
int tail_x[400]; int tail_y[400];
int tail_length = 2;
int tail_mission = 5;

// Item 위치
deque<int> item_x;
deque<int> item_y;
deque<int> item_time;

// Item 스코어
int itemG_score = 0;
int itemP_score = 0;
int itemG_mission = 2;
int itemP_mission = 0;

// Gate 위치
int gateA_x; int gateA_y;
int gateB_x; int gateB_y;

// Gate 스코어
int gate_score = 0;
int gate_mission = 1;

// Gate 통과중인지 여부
int passGate = 0;

// time
int t = 0;
int t_mission = 15;
int level = 1;

bool gameOver = false;

void setLevel() {
	if (level == 1)
		for (int i = 1; i < 20; i++) {
			for (int j = 1; j < 20; j++) {
				if ((i == 3 || i == 15) && j > 5 && j < 17)
					map[i][j] = 1;
				else if (i > 3 && i < 6 && (j == 5 || j == 17))
					map[i][j] = 1;
				else if (i == 3 && (j == 5 || j == 17))
					map[i][j] = 2;
			}
		}
	else if (level == 2)
		for (int i = 1; i < 20; i++) {
			for (int j = 1; j < 20; j++) {
				if ((i == 21 - j || i == j) && (i < 3 || i > 18))
					map[i][j] = 2;
				if ((i == 21 - j || i == j) && (i < 7 || i > 13))
					map[i][j] = 1;
			}
		}
	else if (level == 3) {
		// 초기 몸통 위치 head[5, 4], 0[5, 5], 1[5, 6]
		head_y = 5;
		head_x = 4;
		tail_y[0] = 5;
		tail_x[0] = 5;
		tail_y[1] = 5;
		tail_x[1] = 6;
		for (int i = 1; i < 20; i++) {
			map[10][i] = 1; map[i][10] = 1;
		}
		map[10][10] = 2;
	}
	else if (level == 4) {
		// 초기 몸통 위치 head[5, 4], 0[5, 5], 1[5, 6]
		head_y = 5;
		head_x = 4;
		tail_y[0] = 5;
		tail_x[0] = 5;
		tail_y[1] = 5;
		tail_x[1] = 6;
		for (int i = 3; i < 7; i++) {
			map[7][i] = 1; map[13][i] = 1;
		}
		for (int i = 7; i < 13; i++) {
			map[i][3] = 1;
		}
		for (int i = 8; i < 13; i++) {
			map[i][10] = 1; map[i][16] = 1;
		}
		for (int i = 8; i < 19; i++) {
			map[10][i] = 1;
		}
		map[10][13] = 0;
	}
}

void setMap() {
	item_x.clear();
	item_y.clear();
	item_time.clear();
	for (int i = 0; i < 21; i++)
		for (int j = 0; j < 21; j++) {
			if (i == 0 || i == 20)
				map[i][j] = 1;
			else if (j == 0 || j == 20) {
				map[i][j] = 1;
			}
			else
				map[i][j] = 0;
		}
	// 꼭지점
	map[0][0] = 2; map[20][20] = 2;
	map[20][0] = 2; map[0][20] = 2;

	// 초기 몸통 위치 head[10, 9], 0[10, 10], 1[10, 11]
	head_y = 10;
	head_x = 9;
	tail_y[0] = 10;
	tail_x[0] = 10;
	tail_y[1] = 10;
	tail_x[1] = 11;

	setLevel();

	// initialization
	direction_x = -1;
	direction_y = 0;
	tail_length = 2;
	itemG_score = 0;
	itemP_score = 0;
	gate_score = 0;
	passGate = 0;
	t = 0;
}

bool isTailPosition(int tail_position, int i, int j) {
	for (int k = tail_position; k < tail_length; k++) {
		if (tail_y[k] == i && tail_x[k] == j) {
			return true;
		}
	}
	return false;
}

void setMission() {
	if (tail_mission < 18) {
		tail_mission++;
	}
	itemG_mission++;
	itemP_mission++;
	gate_mission++;
	t_mission += 2;
}

bool isNextStage() {
	if (tail_length >= tail_mission && itemG_score >= itemG_mission
		&& itemP_score >= itemP_mission && gate_score >= gate_mission
		&& t / 2 >= t_mission)
		return true;
	return false;
}

void drawScore() {
	mvprintw(3, 24, "Score Board");
	mvprintw(4, 24, "B: ");
	const char* tail_len_str = (to_string(tail_length)).c_str();
	mvprintw(4, 27, tail_len_str);
	const char* tail_mission_str = (to_string(tail_mission)).c_str();
	mvprintw(4, 29, "/ ");
	mvprintw(4, 31, tail_mission_str);
	mvprintw(5, 24, "+: ");
	const char* itemG_str = (to_string(itemG_score)).c_str();
	mvprintw(5, 27, itemG_str);
	mvprintw(6, 24, "-: ");
	const char* itemP_str = (to_string(itemP_score)).c_str();
	mvprintw(6, 27, itemP_str);
	mvprintw(7, 24, "G: ");
	const char* gate_str = (to_string(gate_score)).c_str();
	mvprintw(7, 27, gate_str);
	mvprintw(8, 24, "T: ");
	const char* time_str = (to_string(t / 2)).c_str();
	mvprintw(8, 27, time_str);

	mvprintw(10, 24, "Mission");
	mvprintw(11, 24, "B: ");
	const char* tail_mission_str2 = (to_string(tail_mission)).c_str();
	mvprintw(11, 27, tail_mission_str2);
	mvprintw(12, 24, "+: ");
	const char* itemG_mission_str = (to_string(itemG_mission)).c_str();
	mvprintw(12, 27, itemG_mission_str);
	mvprintw(13, 24, "-: ");
	const char* itemP_mission_str = (to_string(itemP_mission)).c_str();
	mvprintw(13, 27, itemP_mission_str);
	mvprintw(14, 24, "G: ");
	const char* gate_mission_str = (to_string(gate_mission)).c_str();
	mvprintw(14, 27, gate_mission_str);
	mvprintw(15, 24, "T: ");
	const char* t_mission_str = (to_string(t_mission)).c_str();
	mvprintw(15, 27, t_mission_str);

	if (tail_mission <= tail_length)
		mvprintw(11, 30, "(V)");
	else
		mvprintw(11, 30, "( )");
	if (itemG_mission <= itemG_score)
		mvprintw(12, 30, "(V)");
	else
		mvprintw(12, 30, "( )");
	if (itemP_mission <= itemP_score)
		mvprintw(13, 30, "(V)");
	else
		mvprintw(13, 30, "( )");
	if (gate_mission <= gate_score)
		mvprintw(14, 30, "(V)");
	else
		mvprintw(14, 30, "( )");
	if (t_mission <= t / 2)
		mvprintw(15, 30, "(V)");
	else
		mvprintw(15, 30, "( )");
}

void drawMap() {
	clear();
	int tail_position = 0;
	for (int i = 0; i < 21; i++) {
		for (int j = 0; j < 21; j++) {
			// 아이템G
			if (map[i][j] == 5) {
				start_color();
				init_pair(5, COLOR_GREEN, COLOR_BLACK);

				attron(COLOR_PAIR(5));
				mvprintw(i, j, "\u2B1B");
				attroff(COLOR_PAIR(5));
			}
			// 아이템P
			else if (map[i][j] == 6) {
				start_color();
				init_pair(4, COLOR_RED, COLOR_BLACK);

				attron(COLOR_PAIR(4));
				mvprintw(i, j, "\u2B1B");
				attroff(COLOR_PAIR(4));
			}
			// 게이트
			else if (map[i][j] == 7 || map[i][j] == 8) {
				start_color();
				init_pair(3, COLOR_MAGENTA, COLOR_BLACK);

				attron(COLOR_PAIR(3));
				mvprintw(i, j, "\u2B1B");
				attroff(COLOR_PAIR(3));
			}
			// 벽
			if (map[i][j] == 1 || map[i][j] == 2)
				mvprintw(i, j, "\u2B1B");
			// 머리
			else if (head_y == i && head_x == j) {
				start_color();
				init_pair(1, COLOR_YELLOW, COLOR_BLACK);

				attron(COLOR_PAIR(1));
				mvprintw(i, j, "\u2B1B");
				attroff(COLOR_PAIR(1));
			}
			// 몸통
			else if (map[i][j] == 3)
			{
				tail_position++;
				start_color();
				init_pair(2, COLOR_BLUE, COLOR_BLACK);

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
	drawScore();
	refresh();
}

bool isGameOver() {
	if (gameOver)
		return true;
	if (map[head_y][head_x] == 1 || map[head_y][head_x] == 2) // snake가 벽에 닿을 시
		return true;
	if (tail_length < 2) return true; // 몸통의 길이가 2보다 작아질 경우
	for (int i = 0; i < tail_length; i++)
		if (head_y == tail_y[i] && head_x == tail_x[i]) // 머리가 몸에 닿았을 경우
			return true;
	return false;
}

// 뱀의 몸통이 앞 몸통을 따라 이동하는 함수
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

// 뱀의 이동경로 결정 및 머리 위치 이동
void setSnake(int gate_x, int gate_y, int x, int y) {
	direction_x = x;
	direction_y = y;
	head_x = gate_x + x;
	head_y = gate_y + y;
}

void moveGate(int gate_x, int gate_y) {
	passGate += tail_length;

	if (gate_x == 0) // (좌에서 우)
		setSnake(gate_x, gate_y, 1, 0);
	else if (gate_x == 20) // (우에서 좌)
		setSnake(gate_x, gate_y, -1, 0);
	else if (gate_y == 0) // (상에서 하)
		setSnake(gate_x, gate_y, 0, 1);
	else if (gate_y == 20) // (하에서 상)
		setSnake(gate_x, gate_y, 0, -1);
	else { // 벽이 아닌 곳에 게이트가 있을 경우
		int tmp_x = gate_x + direction_x;
		int tmp_y = gate_y + direction_y;

		if (map[tmp_y][tmp_x] == 1) { // 진출방향에 벽이 있다면
			if (direction_x == 1 && direction_y == 0) // 우
				if (map[gate_y + 1][gate_x] != 1) // 시계방향
					setSnake(gate_x, gate_y, 0, 1);
				else if (map[gate_y - 1][gate_x] != 1) // 반시계방향
					setSnake(gate_x, gate_y, 0, -1);
				else // 반대방향
					setSnake(gate_x, gate_y, -1, 0);
			else if (direction_x == -1 && direction_y == 0) // 좌
				if (map[gate_y - 1][gate_x] != 1)
					setSnake(gate_x, gate_y, 0, -1);
				else if (map[gate_y + 1][gate_x] != 1)
					setSnake(gate_x, gate_y, 0, 1);
				else
					setSnake(gate_x, gate_y, 1, 0);
			else if (direction_x == 0 && direction_y == 1) // 하
				if (map[gate_y][gate_x - 1] != 1)
					setSnake(gate_x, gate_y, -1, 0);
				else if (map[gate_y][gate_x + 1] != 1)
					setSnake(gate_x, gate_y, 1, 0);
				else
					setSnake(gate_x, gate_y, 0, -1);
			else if (direction_x == 0 && direction_y == -1) // 상
				if (map[gate_y][gate_x + 1] != 1)
					setSnake(gate_x, gate_y, 1, 0);
				else if (map[gate_y][gate_x - 1] != 1)
					setSnake(gate_x, gate_y, -1, 0);
				else
					setSnake(gate_x, gate_y, 0, 1);
		}
		else { // 진출방향에 벽이 없을경우
			head_x = tmp_x;
			head_y = tmp_y;
		}
	}
}

void moveSnake() {
	int prevX = head_x;
	int prevY = head_y;
	int removeX = tail_x[tail_length - 1];
	int removeY = tail_y[tail_length - 1];

	head_x += direction_x;
	head_y += direction_y;

	if (passGate > 0) {
		passGate--;
		// 뱀이 게이트를 다 지나면 점수
		if (passGate == 0)
			gate_score++;
	}

	// Head가 아이템G에 닿았을 때
	if (map[head_y][head_x] == 5) {
		itemG_score++;

		tail_length++;
		moveTails(prevX, prevY, removeX, removeY);
		// 아이템G 획득 시 몸의 길이가 진행방향으로 증가하므로 마지막 꼬리를 삭제 X
		map[removeY][removeX] = 3;
		map[head_y][head_x] = 0;

		auto it = find(item_x.begin(), item_x.end(), head_x);
		int idx = it - item_x.begin();
		item_x.erase(item_x.begin() + idx);
		item_y.erase(item_y.begin() + idx);
		item_time.erase(item_time.begin() + idx);
	}
	// Head가 아이템P에 닿았을 때
	else if (map[head_y][head_x] == 6) {
		itemP_score++;

		map[removeY][removeX] = 0;
		tail_length--;

		removeX = tail_x[tail_length - 1];
		removeY = tail_y[tail_length - 1];

		moveTails(prevX, prevY, removeX, removeY);
		map[head_y][head_x] = 0;

		auto it = find(item_x.begin(), item_x.end(), head_x);
		int idx = it - item_x.begin();
		item_x.erase(item_x.begin() + idx);
		item_y.erase(item_y.begin() + idx);
		item_time.erase(item_time.begin() + idx);
	}

	// Head가 게이트가 닿았을 때
	else if (map[head_y][head_x] == 7) {
		moveGate(gateB_x, gateB_y);
		moveTails(prevX, prevY, removeX, removeY);
	}
	else if (map[head_y][head_x] == 8) {
		moveGate(gateA_x, gateA_y);
		moveTails(prevX, prevY, removeX, removeY);
	}
	else { // 진출방향에 아무 것도 없을 시
		moveTails(prevX, prevY, removeX, removeY);
	}
}

void* getInput(void* arg) {
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

void setItem(int t, int num) {
	if (item_x.size() > 0)
		for (int i = 0; i < item_x.size(); i++) {
			if (t - item_time[i] >= 40) {
				map[item_y[i]][item_x[i]] = 0;
				item_x.erase(item_x.begin() + i);
				item_y.erase(item_y.begin() + i);
				item_time.erase(item_time.begin() + i);
			}
		}
	while (item_x.size() >= 3) {
		map[item_y.front()][item_x.front()] = 0;
		item_x.pop_front();
		item_y.pop_front();
		item_time.pop_front();
	}
	int x, y;
	srand((unsigned int)time(NULL));
	y = rand() % 20 + 1; if (y > 19) y = 19;
	x = rand() % 20 + 1; if (x > 19) x = 19;

	// 탈출조건 : 좌표값이 0이면서, 뱀의 머리와 겹치지않아야함
	while (map[y][x] != 0 || (x == head_x && y == head_y)) {
		y = rand() % 20 + 1; if (y > 19) y = 19;
		x = rand() % 20 + 1; if (x > 19) x = 19;
	}
	item_y.push_back(y);
	item_x.push_back(x);
	item_time.push_back(t);
	map[y][x] = num;
}

void setGate() {
	if (passGate > 0) return;
	if (gateA_y != 0 || gateA_x != 0) map[gateA_y][gateA_x] = 1;
	if (gateB_y != 0 || gateB_x != 0) map[gateB_y][gateB_x] = 1;
	int a, b, c, d;

	srand((unsigned int)time(NULL));
	a = rand() % 21; b = rand() % 21;
	while (map[b][a] != 1) {
		a = rand() % 21;
		b = rand() % 21;
	}
	gateA_y = b;
	gateA_x = a;
	map[b][a] = 7;

	c = rand() % 21; d = rand() % 21;
	while ((map[d][c] != 1) || (map[d][c] == map[b][a])) {
		c = rand() % 21;
		d = rand() % 21;
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

	while (!isGameOver()) {
		if (isNextStage()) {
			level++;
			if (level == 5)
				break;
			setMission();
			setMap();
		}

		if (t >= 10 && t % 10 == 0) {
			srand((unsigned int)time(NULL));
			int r = rand() % 3;
			if (r > 0) setItem(t, 5);
			else setItem(t, 6);
		}

		if (tail_length >= 4 && t % 20 == 0) { setGate(); }

		moveSnake();
		drawMap();
		usleep(500000);
		t += 1;
	}

	if (level == 5)
		mvprintw(22, 5, "GameClear!");
	else
		mvprintw(22, 5, "GameOver");

	getch();
	endwin();
	return 0;
}
