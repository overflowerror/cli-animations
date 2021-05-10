#include "common/compat.h"
#include "common/graphics.h"
#include "common/utils.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>

int main() {
	if (setvbuf(stdin, NULL, _IONBF, 0) != 0) {
		panic("can't set buffer for stdin: %s", strerror(errno));
	}

	struct dimensions d = getScreenSize();

	cprintf("%d, %d\n", d.width, d.height);
}
