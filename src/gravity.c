#include <locale.h>
#include <math.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "common/graphics.h"
#include "common/utils.h"

#define SUBSTITUDE_PRINTS
#include "common/compat.h"

#define G (10.0)

#define OFFSET_X (-50)
#define OFFSET_Y (-15)

#define SCREEN_WIDTH (100)
#define SCREEN_HEIGHT (30)

#define NUMBER_OF_POINTS (2)

#define TERMINAL_VELOCITY (100.0)

#define ASPECT_RATIO (3.0/2.0)
#define FACTOR_X (1.0)
#define FACTOR_Y (FACTOR_X * ASPECT_RATIO)

#define SLEEP_TIME (100000)

#define TICKS_PER_TIME (1)

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
	struct vector velocity;
	struct vector force;
	float radius;
	float mass;
};

float distance(struct point* a, struct point* b) {
	float dx = b->position.x - a->position.x;
	float dy = b->position.y - a->position.y;
	
	return sqrt((dx * dx) + (dy * dy));
}

struct vector direction(struct point* a, struct point* b) {
	float dx = b->position.x - a->position.x;
	float dy = b->position.y - a->position.y;
	
	float r = sqrt((dx * dx) + (dy * dy));
	
	return (struct vector) {
		x: dx / r,
		y: dy / r
	};
	
}

void tickPoint(struct point* point) {
	float factor = SLEEP_TIME / 1000000.0 * TICKS_PER_TIME;

	point->position.x += point->velocity.x * factor;
	point->position.y += point->velocity.y * factor;
}

void zeroVector(struct vector* vector) {
	vector->x = 0;
	vector->y = 0;
}

void merge(struct point* a, struct point* b) {
	a->position.x = (a->position.x + b->position.x) / 2;
	a->position.y = (a->position.y + b->position.y) / 2;
	b->position.x = a->position.x;
	b->position.y = a->position.y;

	a->velocity.x = (a->velocity.x * a->mass + b->velocity.x * b->mass) / (a->mass + b->mass);
	a->velocity.y = (a->velocity.y * a->mass + b->velocity.y * b->mass) / (a->mass + b->mass);
	b->velocity.x = a->velocity.x;
	b->velocity.y = a->velocity.y;
}

void applyGravityToForce(struct point* a, struct point* b) {
	float r = distance(a, b);
	
	cprintf("%f, ", r);

	if (r <= a->radius + b->radius) {
		//merge(a, b);
		return;
	}
	float f = G * (a->mass * b->mass) / (r * r);
	
	struct vector directionAtoB = direction(a, b);

	a->force.x += f * directionAtoB.x;
	a->force.y += f * directionAtoB.y;
	b->force.x += f * directionAtoB.x * (-1);
	b->force.y += f * directionAtoB.y * (-1);

	//cprintf("%f, %f\n", a->force.x, a->force.y);
}

void applyForceToVelocity(struct point* point) {
	point->velocity.x += point->force.x / point->mass;
	point->velocity.y += point->force.y / point->mass;
	
	float scalarVelocity = sqrt(point->velocity.x * point->velocity.x + point->velocity.y * point->velocity.y);

	if (scalarVelocity > TERMINAL_VELOCITY) {
		float capFactor = TERMINAL_VELOCITY / scalarVelocity;

		point->velocity.x *= capFactor;
		point->velocity.y *= capFactor;
	}

	zeroVector(&(point->force));
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
	return (struct point) {
		position: {
			x: randomFloat() * 20 - 10,
			y: randomFloat() * 20 - 10
		},
		velocity: {
			x: (randomFloat() - 0.5) / 1000,
			y: (randomFloat() - 0.5) / 1000
		},
		force: {
			x: 0,
			y: 0
		},
		radius: 0.1,
		mass: randomFloat() + 0.1
	};
}

void initPoints(struct point points[]) {
	for (int i = 0; i < NUMBER_OF_POINTS; i++) {
		points[i] = randomPoint();
	}
}

void tickPoints(struct point points[]) {
	for (int i = 0; i < NUMBER_OF_POINTS; i++) {
		for (int j = 0; j < NUMBER_OF_POINTS; j++) {
			if (i == j)
				continue;
		
			applyGravityToForce(&(points[i]), &(points[j]));
		}
	}

	for (int i = 0; i < NUMBER_OF_POINTS; i++) {
		applyForceToVelocity(&(points[i]));
		tickPoint(&(points[i]));
	}
}

struct screenCell {
	struct patternAtom* atoms;
	int number;
};

void displayPoints(struct point points[]) {
	struct screenCell screen[SCREEN_WIDTH][SCREEN_HEIGHT];
	for (int x = 0; x < SCREEN_WIDTH; x++) {
		for (int y = 0; y < SCREEN_HEIGHT; y++) {
			screen[x][y].atoms = NULL;
			screen[x][y].number = 0;
		}
	}

	for (int i = 0; i < NUMBER_OF_POINTS; i++) {
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

	for (int y = 0; y < SCREEN_HEIGHT; y++) {
		for (int x = 0; x < SCREEN_WIDTH; x++) {
			cprintf("%lc", getPattern(screen[x][y].atoms, screen[x][y].number));
		}
		cprintf("\n");
	}
}

int main() {
	setlocale(LC_CTYPE, "");
	enable_compat_mode(PRINT_WCHAR);

	initRandom();

	struct point points[NUMBER_OF_POINTS];
	//initPoints(points);
	
	// hardcoded points to test orbit example
	points[0] = (struct point) {
		position: {
			x: 0,
			y: 0
		},
		velocity: {
			x: 0,
			y: 0
		},
		force: {
			x: 0,
			y: 0
		},
		radius: 0.1,
		mass: 5
	};
	points[1] = (struct point) {
		position: {
			x: 10,
			y: 0
		},
		velocity: {
			x: 0,
			y: 10
		},
		force: {
			x: 0,
			y: 0
		},
		radius: 0.1,
		mass: 0.1
	};
	
	hideCursor();
	
	eraseScreen();

	while(true) {
		moveCursorAbsolute(0, 0);
		displayPoints(points);
		//fflush(stdout);
		usleep(SLEEP_TIME);
		for (int i = 0; i < TICKS_PER_TIME; i++)
			tickPoints(points);
	}
}
