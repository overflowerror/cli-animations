#include <locale.h>
#include <stdbool.h>
#include <unistd.h>
#include <wchar.h>

#include "common/compat.h"
#include "common/graphics.h"
#include "common/utils.h"

#define WIDTH (40)
#define NUMBER_OF_CHARACTERS (8)
#define NUMBER_OF_BLOCKS (20)
#define SLEEP_TIME (50000l)


#define NO_CHARACTER (-1)

wchar_t chars[] = {
    0x258F,
    0x258E,
    0x258D,
    0x258C,
    0x258B,
    0x258A,
    0x2589,
    0x2588
};

enum direction {
    left, right
};

struct block {
    enum direction direction;
    int character;
    float position;
    float speed;
};

struct buffer {
    int characters[WIDTH];
};

struct block randomBlock() {
    return (struct block){
            direction: (randomInt() % 2) ? left : right,
            character: (randomInt() % NUMBER_OF_CHARACTERS),
            position:  (randomInt() % WIDTH),
            speed:     1
    };
}


void printBuffer(struct buffer* buffer) {
    for (int i = 0; i < WIDTH; i++) {
        if (buffer->characters[i] < 0) {
            cprintf(" ");
        } else {
            cprintf("%lc", chars[buffer->characters[i]]);
        }
    }
}

void clearBuffer(struct buffer* buffer) {
    for (int i = 0; i < WIDTH; i++) {
        buffer->characters[i] = -1;
    }
}

void applyBlockToBuffer(struct block* block, struct buffer* buffer) {
    if (buffer->characters[(int) block->position] <= block->character)
        buffer->characters[(int) block->position] = block->character;
}

void tickBlock(struct block* block) {
    if (block->direction == right) {
        block->position += block->speed;
        if (block->position >= WIDTH - 1) {
            block->position = WIDTH - 1;
            block->direction = left;
        }
    } else {
        block->position -= block->speed;
        if (block->position <= 0) {
            block->position = 0;
            block->direction = right;
        }
    }
}

void doTick(struct block blocks[]) {
    struct buffer buffer;
    clearBuffer(&buffer);
    
    moveCursorRelative(- (WIDTH + 2), 0);
    eraseLine();
    
    for (int i = 0; i < NUMBER_OF_BLOCKS; i++) {
        tickBlock(&(blocks[i]));
        
        applyBlockToBuffer(&(blocks[i]), &buffer);
    }    
    
    cprintf("[");
    printBuffer(&buffer);
    cprintf("]");
}

void makeBlocks(struct block blocks[]) {
    for (int i = 0; i < NUMBER_OF_BLOCKS; i++) {
        blocks[i] = randomBlock();
    }
}

int main() {
    setlocale(LC_CTYPE, "");
    
    enable_compat_mode(PRINT_WCHAR);
    
    initRandom();
    
    hideCursor();
    
    struct block blocks[NUMBER_OF_BLOCKS];
    
    makeBlocks(blocks);
    
    while (true) {
        doTick(blocks);
        fflush(stdout);
        usleep(SLEEP_TIME);
    }
    
    cprintf("\n");
    
}
