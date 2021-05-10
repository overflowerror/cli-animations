#include "graphics.h"

#include "compat.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

void moveCursorRelative(int x, int y) {
	if (x < 0) {
		cprintf("\033[%dD", -x);
	} else if (x > 0) {
		cprintf("\033[%dC", x);
	}
	if (y < 0) {
		cprintf("\033[%dA", -x);
	} else if (y > 0) {
		cprintf("\033[%dB", x);
	}
}

void moveCursorAbsolute(int x, int y) {
	cprintf("\033[%d;%dH", y + 1, x + 1);
}

void eraseLine() {
	cprintf("\033[K");
}

void eraseScreen() {
	cprintf("\033[2J");
}

void hideCursor() {
	cprintf("\033[?25l");
}

void showCursor() {
	cprintf("\033[?25h");
}

struct dimensions getScreenSize() {
	moveCursorAbsolute(999, 999);
	cprintf("\033[6n");

	char buffer[12];
	int i = 0;
	while (true) {
		cprintf("--\n");
		int m = fread(&(buffer[i++]), 1, 1, stdin);
		cprintf("%d\n", m);
		cprintf("%c\n", buffer[i - 1]);
		if (buffer[i - 1] == 'R')
			break;
	}
	buffer[i] = '\0';

	struct dimensions d;

	char* endptr = &(buffer[2]);
	int tmp = strtol(endptr, &endptr, 10);
	d.width = tmp;
	tmp = strtol(endptr, &endptr, 10);
	d.height = tmp;

	return d;
}
