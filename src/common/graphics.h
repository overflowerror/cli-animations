#ifndef GRAPHICS_H
#define GRAPHICS_H

struct dimensions {
	int width;
	int height;
};

void moveCursorRelative(int x, int y);
void moveCursorAbsolute(int x, int y);
void eraseLine();
void eraseScreen();

void hideCursor();
void showCursor();

struct dimensions getScreenSize();

#endif
