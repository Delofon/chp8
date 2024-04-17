.RECIPEPREFIX = >

CC := gcc
CC_MINGW := x86_64-w64-mingw32-gcc

CFLAGS := $(shell pkg-config --cflags libpulse-simple ncurses sdl2)
LIBS := -lm $(shell pkg-config --libs libpulse-simple ncurses sdl2)

CFLAGS_WIN := $(shell pkg-config --cflags sdl2)
LIBS_WIN := -lm $(shell pkg-config --libs sdl2)

CWARNINGS := -Wall -Wextra -Werror=shadow -Wswitch-enum -pedantic
CNOWARNINGS := -Wno-strict-prototypes

SOURCES := main.c sound.c vm.c timing.c screen_ncurses.c screen_sdl.c
SOURCES_WIN := main.c vm.c timing.c screen_sdl.c

chp8: $(SOURCES)
> $(CC) -O2 $(CFLAGS) $(CWARNINGS) $(CNOWARNINGS) $(ARGS) -o chp8 $^ $(LIBS)

chp8-win: $(SOURCES_WIN)
> $(CC_MINGW) -O2 $(CFLAGS_WIN) $(CWARNINGS) $(CNOWARNINGS) $(ARGS) -o chp8-win $^ $(LIBS_WIN)

.PHONY: debug
debug: $(SOURCES)
> $(CC) -O0 -g3 -DDEBUG $(CFLAGS) $(CWARNINGS) $(CNOWARNINGS) -Wno-format $(ARGS) -o chp8 $^ $(LIBS)

