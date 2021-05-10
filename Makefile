CC       = gcc
CFLAGS   = -std=c99 -Wall -D_POSIX_C_SOURCE=201112L -D_XOPEN_SOURCE=500 -D_GNU_SOURCE -static -g
LD       = gcc
LDFLAGS  = -lm

COMMON   = obj/common/graphics.o\
           obj/common/compat.o\
           obj/common/utils.o
DEPS     = $(OBJS:%.o=%.d)

all: bars gravity font

bars: obj/bars.o $(COMMON)
	$(LD) -o $@ $^ $(LDFLAGS)
	
gravity: obj/gravity.o $(COMMON)
	$(LD) -o $@ $^ $(LDFLAGS)

font: obj/font.o $(COMMON)
	$(LD) -o $@ $^ $(LDFLAGS)

test: obj/test.o $(COMMON)
	$(LD) -o $@ $^ $(LDFLAGS)

-include $(DEPS)

obj/%.o: src/%.c obj
	$(CC) $(CFLAGS) -MMD -c -o $@ $<

obj:
	@mkdir -p obj/common/

clean:
	@echo "Cleaning up..."
	@rm -f obj/*.o
	@rm -f obj/*.d
	@rm -f obj/*/*.o
	@rm -f obj/*/*.d
	@rm -f bars
	@rm -f gravity
	@rm -f font
