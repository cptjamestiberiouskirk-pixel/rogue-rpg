TARGET:=splash
CFLAGS?=-Wall -Wextra -std=c11 -pedantic -Og -g
LDFLAGS?=
LDLIBS:=

# --silence-errors only needed for 'make clean' from parent dir
CFLAGS+=$(shell pkg-config --silence-errors sdl2 --cflags)
LDLIBS+=$(shell pkg-config --silence-errors sdl2 --libs)

OBJS=splash.o load_sdl.o

all: default

default: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@

clean:
	rm -f $(TARGET) *.o

.PHONY: all default clean
