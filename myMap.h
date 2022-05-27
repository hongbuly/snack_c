#ifndef _MYMAP_H_
#define _MYMAP_H_
#include <clocale>
#include <ncurses.h>

class myMap {
protected:
	int map[21][21];
public:
	myMap();
	void drawMap();
};

#endif _MYMAP_H_
