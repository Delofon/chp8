.RECIPEPREFIX = >

CC := gcc
CFLAGS := $(shell pkg-config --cflags libpulse-simple ncurses sdl2)
LIBS := -lm $(shell pkg-config --libs libpulse-simple ncurses sdl2)
CWARNINGS := -Wall -Wextra -Werror=shadow -Wswitch-enum -pedantic
CNOWARNINGS := -Wno-strict-prototypes
SOURCES := main.c sound.c vm.c timing.c screen_ncurses.c screen_sdl.c

chp8:
> $(CC) -O2 $(CWARNINGS) $(CNOWARNINGS) $(CFLAGS) $(LIBS) $(ARGS) $(SOURCES) -o chp8

debug:
> $(CC) -O0 -g3 -DDEBUG $(CWARNINGS) $(CNOWARNINGS) -Wno-format $(CFLAGS) $(LIBS) $(ARGS) $(SOURCES) -o chp8

