#include <locale.h>
#include <math.h>
#include <stdbool.h>
#include <unistd.h>
#include <wchar.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "common/graphics.h"
#include "common/utils.h"

#define SUBSTITUDE_PRINTS
#include "common/compat.h"

#define MAX_VELOCITY (3)

#define ANIMATION_SLEEP_TIME (100000)
#define SCATTER_SLEEP_TIME (2000000)

#define OFFSET_X (-50)
#define OFFSET_Y (-15)

#define SCREEN_WIDTH (100)
#define SCREEN_HEIGHT (30)

#define ASPECT_RATIO (3.0/2.0)
#define FACTOR_X (1.0)
#define FACTOR_Y (FACTOR_X * ASPECT_RATIO)

#define BRAILLE_PREFIX (0x2800)
#define BRAILLE_TOP_LEFT (0x01)
#define BRAILLE_MIDDLE_LEFT (0x02)
#define BRAILLE_BOTTOM_LEFT (0x04)
#define BRAILLE_TOP_RIGHT (0x08)
#define BRAILLE_MIDDLE_RIGHT (0x10)
#define BRAILLE_BOTTOM_RIGHT (0x20)

struct vector {
	float x;
	float y;
};

struct point {
	struct vector position;
	struct vector source;
	struct vector target;
};

float distance(struct vector a, struct vector b) {
	float dx = b.x - a.x;
	float dy = b.y - a.y;
	
	return sqrt((dx * dx) + (dy * dy));
}

struct vector direction(struct vector a, struct vector b) {
	float dx = b.x - a.x;
	float dy = b.y - a.y;
	
	float r = sqrt((dx * dx) + (dy * dy));
	
	return (struct vector) {
		x: dx / r,
		y: dy / r
	};
	
}

void zeroVector(struct vector* vector) {
	vector->x = 0;
	vector->y = 0;
}

struct patternAtom {
	int x; // 0-1
	int y; // 0-2
};

wchar_t getPattern(struct patternAtom atoms[], int number) {
	if (number == 0) {
		return L' ';
	}

	wchar_t result = BRAILLE_PREFIX;

	for (int i = 0; i < number; i++) {
		if (atoms[i].x == 0) {
			if (atoms[i].y == 0) {
				result |= BRAILLE_TOP_LEFT;
			} else if (atoms[i].y == 1) {
				result |= BRAILLE_MIDDLE_LEFT;
			} else {
				result |= BRAILLE_BOTTOM_LEFT;
			}
		} else {
			if (atoms[i].y == 0) {
				result |= BRAILLE_TOP_RIGHT;
			} else if (atoms[i].y == 1) {
				result |= BRAILLE_MIDDLE_RIGHT;
			} else {
				result |= BRAILLE_BOTTOM_RIGHT;
			}
		}
	}

	return result;
}

struct point randomPoint() {
	struct point p = {
		source: {
			x: randomFloat() * 20 - 10,
			y: randomFloat() * 20 - 10
		},
		target: {
			x: randomFloat() * 20 - 10,
			y: randomFloat() * 20 - 10
		}
	};

	p.position = p.source;
	
	return p;
}

void initPoints(struct point points[], int number) {
	for (int i = 0; i < number; i++) {
		points[i] = randomPoint();
	}
}

float velocity(float x) {
	if (x > 1.0) {
		return MAX_VELOCITY * (-0.2)*(x - 1);
	} else if (x > 0.5) {
		return MAX_VELOCITY * (-2)*(x - 1);
	} else {
		return MAX_VELOCITY * 2*x + 0.2;
	}
}

bool tickPoint(struct point* p) {
	struct vector d = direction(p->position, p->target);

	float x = distance(p->source, p->target);
	if (fabs(x) < 0.01) {
		x = 1.0;
	} else {
		x = distance(p->source, p->position) / x;
	}

	float v = fabs(velocity(x));

	if (fabs(1 - x) < 0.01) {
		p->position = p->target;
		return true;
	} else {
		p->position.x += d.x * v;
		p->position.y += d.y * v;
		return false;
	}
}

bool tickPoints(struct point points[], int number) {
	bool finished = true;

	for (int i = 0; i < number; i++) {
		if (!tickPoint(&(points[i])))
			finished = false;
	}

	return finished;
}

struct screenCell {
	struct patternAtom* atoms;
	int number;
};

void displayPoints(struct point points[], int number) {
	struct screenCell screen[SCREEN_WIDTH][SCREEN_HEIGHT];
	for (int x = 0; x < SCREEN_WIDTH; x++) {
		for (int y = 0; y < SCREEN_HEIGHT; y++) {
			screen[x][y].atoms = NULL;
			screen[x][y].number = 0;
		}
	}

	for (int i = 0; i < number; i++) {
		struct point p = points[i];
		float x = p.position.x * FACTOR_X - OFFSET_X;
		float y = p.position.y * FACTOR_Y - OFFSET_Y;

 		int screenX = floor(x);
		int screenY = floor(y);

		if (screenX < 0 || screenX >= SCREEN_WIDTH) {
			continue;
		}
		if (screenY < 0 || screenY >= SCREEN_HEIGHT) {
			continue;
		}

		struct patternAtom atom = {
			x: floor((x - screenX) * 2),
			y: floor((y - screenY) * 3)
		};

		struct screenCell* cell = &(screen[screenX][screenY]);
		cell->atoms = realloc(cell->atoms, ++cell->number * sizeof(struct patternAtom));
		if (cell->atoms == NULL) {
			panic("couldn't allocate screen buffer: %s", strerror(errno));
		}
		cell->atoms[cell->number - 1] = atom;
	}

	for (int x = 0; x < SCREEN_WIDTH + 2; x++) {
		cprintf("-");
	}
	cprintf("\n");
	for (int y = 0; y < SCREEN_HEIGHT; y++) {
		cprintf("|");
		for (int x = 0; x < SCREEN_WIDTH; x++) {
			cprintf("%lc", getPattern(screen[x][y].atoms, screen[x][y].number));
		}
		cprintf("|\n");
	}
	for (int x = 0; x < SCREEN_WIDTH + 2; x++) {
		cprintf("-");
	}
	cprintf("\n");
}

void createPoint(struct point points[], int i, struct vector position) {
	if (points == NULL)
		return;

	struct vector source = {
		x: randomFloat() * SCREEN_WIDTH - SCREEN_WIDTH / 2,
		y: (randomFloat() * SCREEN_HEIGHT - SCREEN_HEIGHT / 2) / ASPECT_RATIO
	};

	points[i] = (struct point) {
		target: position,
		source: source,
		position: source
	};
}

struct vector add(struct vector vector, float x, float y) {
	return (struct vector) {
		x: vector.x + x,
		y: (vector.y + y) / ASPECT_RATIO
	};
}

int char2points(char c, struct point points[], struct vector position) {
	int i = 0;
	switch(c) {
		case 'A':
			createPoint(points, i++, add(position, 1.0, 0.0));
			createPoint(points, i++, add(position, 1.5, 0.0));
			createPoint(points, i++, add(position, 1.0, 0.4));
			createPoint(points, i++, add(position, 1.5, 0.4));
			createPoint(points, i++, add(position, 0.5, 0.7));
			createPoint(points, i++, add(position, 2.0, 0.7));
			createPoint(points, i++, add(position, 0.0, 1.0));
			createPoint(points, i++, add(position, 0.5, 1.0));
			createPoint(points, i++, add(position, 2.0, 1.0));
			createPoint(points, i++, add(position, 2.5, 1.0));
			createPoint(points, i++, add(position, 0.0, 1.4));
			createPoint(points, i++, add(position, 0.5, 1.4));
			createPoint(points, i++, add(position, 1.0, 1.4));
			createPoint(points, i++, add(position, 1.5, 1.4));
			createPoint(points, i++, add(position, 2.0, 1.4));
			createPoint(points, i++, add(position, 2.5, 1.4));
			createPoint(points, i++, add(position, 0.0, 1.7));
			createPoint(points, i++, add(position, 0.5, 1.7));
			createPoint(points, i++, add(position, 1.0, 1.7));
			createPoint(points, i++, add(position, 1.5, 1.7));
			createPoint(points, i++, add(position, 2.0, 1.7));
			createPoint(points, i++, add(position, 2.5, 1.7));
			createPoint(points, i++, add(position, 0.0, 2.0));
			createPoint(points, i++, add(position, 0.5, 2.0));
			createPoint(points, i++, add(position, 2.0, 2.0));
			createPoint(points, i++, add(position, 2.5, 2.0));
			createPoint(points, i++, add(position, 0.0, 2.4));
			createPoint(points, i++, add(position, 0.5, 2.4));
			createPoint(points, i++, add(position, 2.0, 2.4));
			createPoint(points, i++, add(position, 2.5, 2.4));
			createPoint(points, i++, add(position, 0.0, 2.7));
			createPoint(points, i++, add(position, 0.5, 2.7));
			createPoint(points, i++, add(position, 2.0, 2.7));
			createPoint(points, i++, add(position, 2.5, 2.7));
			return i;
		case 'C':
			return i;
		case 'D':
			return i;
		case 'E':
			return i;
		case 'F':
			return i;
		case 'G':
			return i;
		case 'H':
			createPoint(points, i++, add(position, 0.0, 0.0));
			createPoint(points, i++, add(position, 0.0, 0.4));
			createPoint(points, i++, add(position, 0.0, 0.7));
			createPoint(points, i++, add(position, 0.0, 1.0));
			createPoint(points, i++, add(position, 0.0, 1.4));
			createPoint(points, i++, add(position, 0.0, 1.7));
			createPoint(points, i++, add(position, 0.0, 2.0));
			createPoint(points, i++, add(position, 0.0, 2.4));
			createPoint(points, i++, add(position, 0.0, 2.7));
			createPoint(points, i++, add(position, 0.5, 0.0));
			createPoint(points, i++, add(position, 0.5, 0.4));
			createPoint(points, i++, add(position, 0.5, 0.7));
			createPoint(points, i++, add(position, 0.5, 1.0));
			createPoint(points, i++, add(position, 0.5, 1.4));
			createPoint(points, i++, add(position, 0.5, 1.7));
			createPoint(points, i++, add(position, 0.5, 2.0));
			createPoint(points, i++, add(position, 0.5, 2.4));
			createPoint(points, i++, add(position, 0.5, 2.7));
			createPoint(points, i++, add(position, 1.0, 1.0));
			createPoint(points, i++, add(position, 1.0, 1.4));
			createPoint(points, i++, add(position, 1.0, 1.7));
			createPoint(points, i++, add(position, 1.5, 1.0));
			createPoint(points, i++, add(position, 1.5, 1.4));
			createPoint(points, i++, add(position, 1.5, 1.7));
			createPoint(points, i++, add(position, 2.0, 0.0));
			createPoint(points, i++, add(position, 2.0, 0.4));
			createPoint(points, i++, add(position, 2.0, 0.7));
			createPoint(points, i++, add(position, 2.0, 1.0));
			createPoint(points, i++, add(position, 2.0, 1.4));
			createPoint(points, i++, add(position, 2.0, 1.7));
			createPoint(points, i++, add(position, 2.0, 2.0));
			createPoint(points, i++, add(position, 2.0, 2.4));
			createPoint(points, i++, add(position, 2.0, 2.7));
			createPoint(points, i++, add(position, 2.5, 0.0));
			createPoint(points, i++, add(position, 2.5, 0.4));
			createPoint(points, i++, add(position, 2.5, 0.7));
			createPoint(points, i++, add(position, 2.5, 1.0));
			createPoint(points, i++, add(position, 2.5, 1.4));
			createPoint(points, i++, add(position, 2.5, 1.7));
			createPoint(points, i++, add(position, 2.5, 2.0));
			createPoint(points, i++, add(position, 2.5, 2.4));
			createPoint(points, i++, add(position, 2.5, 2.7));
			return i;
		case 'I':
			return i;
		case 'J':
			return i;
		case 'K':
			return i;
		case 'L':
			createPoint(points, i++, add(position, 0.0, 0.0));
			createPoint(points, i++, add(position, 0.0, 0.4));
			createPoint(points, i++, add(position, 0.0, 0.7));
			createPoint(points, i++, add(position, 0.0, 1.0));
			createPoint(points, i++, add(position, 0.0, 1.4));
			createPoint(points, i++, add(position, 0.0, 1.7));
			createPoint(points, i++, add(position, 0.0, 2.0));
			createPoint(points, i++, add(position, 0.0, 2.4));
			createPoint(points, i++, add(position, 0.0, 2.7));
			createPoint(points, i++, add(position, 0.5, 0.0));
			createPoint(points, i++, add(position, 0.5, 0.4));
			createPoint(points, i++, add(position, 0.5, 0.7));
			createPoint(points, i++, add(position, 0.5, 1.0));
			createPoint(points, i++, add(position, 0.5, 1.4));
			createPoint(points, i++, add(position, 0.5, 1.7));
			createPoint(points, i++, add(position, 0.5, 2.0));
			createPoint(points, i++, add(position, 0.5, 2.4));
			createPoint(points, i++, add(position, 0.5, 2.7));
			createPoint(points, i++, add(position, 1.0, 2.4));
			createPoint(points, i++, add(position, 1.0, 2.7));
			createPoint(points, i++, add(position, 1.5, 2.4));
			createPoint(points, i++, add(position, 1.5, 2.7));
			createPoint(points, i++, add(position, 2.0, 2.4));
			createPoint(points, i++, add(position, 2.0, 2.7));
			createPoint(points, i++, add(position, 2.5, 2.4));
			createPoint(points, i++, add(position, 2.5, 2.7));
			return i;
		case 'M':
			return i;
		case 'N':
			return i;
		case 'O':
			createPoint(points, i++, add(position, 0.5, 0.0));
			createPoint(points, i++, add(position, 1.0, 0.0));
			createPoint(points, i++, add(position, 1.5, 0.0));
			createPoint(points, i++, add(position, 2.0, 0.0));
			createPoint(points, i++, add(position, 0.0, 0.4));
			createPoint(points, i++, add(position, 0.5, 0.4));
			createPoint(points, i++, add(position, 1.0, 0.4));
			createPoint(points, i++, add(position, 1.5, 0.4));
			createPoint(points, i++, add(position, 2.0, 0.4));
			createPoint(points, i++, add(position, 2.5, 0.4));
			createPoint(points, i++, add(position, 0.0, 0.7));
			createPoint(points, i++, add(position, 0.5, 0.7));
			createPoint(points, i++, add(position, 1.0, 0.7));
			createPoint(points, i++, add(position, 1.5, 0.7));
			createPoint(points, i++, add(position, 2.0, 0.7));
			createPoint(points, i++, add(position, 2.5, 0.7));
			createPoint(points, i++, add(position, 0.0, 1.0));
			createPoint(points, i++, add(position, 0.5, 1.0));
			createPoint(points, i++, add(position, 2.0, 1.0));
			createPoint(points, i++, add(position, 2.5, 1.0));
			createPoint(points, i++, add(position, 0.0, 1.4));
			createPoint(points, i++, add(position, 0.5, 1.4));
			createPoint(points, i++, add(position, 2.0, 1.4));
			createPoint(points, i++, add(position, 2.5, 1.4));
			createPoint(points, i++, add(position, 0.0, 1.7));
			createPoint(points, i++, add(position, 0.5, 1.7));
			createPoint(points, i++, add(position, 2.0, 1.7));
			createPoint(points, i++, add(position, 2.5, 1.7));
			createPoint(points, i++, add(position, 0.0, 2.0));
			createPoint(points, i++, add(position, 0.5, 2.0));
			createPoint(points, i++, add(position, 1.0, 2.0));
			createPoint(points, i++, add(position, 1.5, 2.0));
			createPoint(points, i++, add(position, 2.0, 2.0));
			createPoint(points, i++, add(position, 2.5, 2.0));
			createPoint(points, i++, add(position, 0.0, 2.4));
			createPoint(points, i++, add(position, 0.5, 2.4));
			createPoint(points, i++, add(position, 1.0, 2.4));
			createPoint(points, i++, add(position, 1.5, 2.4));
			createPoint(points, i++, add(position, 2.0, 2.4));
			createPoint(points, i++, add(position, 2.5, 2.4));
			createPoint(points, i++, add(position, 0.5, 2.7));
			createPoint(points, i++, add(position, 1.0, 2.7));
			createPoint(points, i++, add(position, 1.5, 2.7));
			createPoint(points, i++, add(position, 2.0, 2.7));
			return i;
		case 'P':
			return i;
		case 'Q':
			return i;
		case 'R':
			return i;
		case 'S':
			return i;
		case 'T':
			return i;
		case 'U':
			return i;
		case 'V':
			return i;
		case 'W':
			return i;
		case 'X':
			return i;
		case 'Y':
			return i;
		case 'Z':
			return i;
		default:	
			return i;		
	}
}

void string2points(const char* string, struct point points[]) {
	int length = strlen(string);

	struct vector position = (struct vector) {
		x: -(length * 4 / 2),
		y: -2
	};

	int index = 0;

	for (int i = 0; i < length; i++) {
		index += char2points(string[i], &(points[index]), position);

		position.x += 4;
	}
}

int numberPoints(const char* string) {
	int sum = 0;

	int length = strlen(string);
	for (int i = 0; i < length; i++) {
		sum += char2points(string[i], NULL, (struct vector) {0, 0});
	}

	return sum;
}

void scatter(struct point points[], int number) {
	for (int i = 0; i < number; i++) {
		points[i].source = points[i].position;

		float phi = randomFloat() * M_PI * 2;
		float r = sqrt(SCREEN_WIDTH * SCREEN_WIDTH + SCREEN_HEIGHT * SCREEN_HEIGHT) / 2;

		points[i].target = (struct vector) {
			x: r * cos(phi),
			y: r * sin(phi)
		};
	}
}

int main() {
	setlocale(LC_CTYPE, "");
	enable_compat_mode(PRINT_WCHAR);

	initRandom();

	const char* string = "HALLO";

	int number = numberPoints(string);

	struct point* points = malloc(number * sizeof(struct point));
	if (points == NULL)
		panic("could allocate point array: %s", strerror(errno));

	string2points(string, points);
	
	hideCursor();
	
	eraseScreen();

	bool scattered = false;

	while(true) {
		moveCursorAbsolute(0, 0);
		displayPoints(points, number);
		//fflush(stdout);
		usleep(ANIMATION_SLEEP_TIME);

		if (tickPoints(points, number)) {
			if (scattered) {
				return 0;
			} else {
				scatter(points, number);
				scattered = true;

				moveCursorAbsolute(0, 0);
				displayPoints(points, number);
				usleep(SCATTER_SLEEP_TIME);
			}	
		}
	}
}
