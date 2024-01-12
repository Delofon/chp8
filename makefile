.RECIPEPREFIX = >

CC := gcc
CFLAGS := $(shell pkg-config --cflags libpulse-simple ncurses) -lm $(shell pkg-config --libs libpulse-simple ncurses)
CWARNINGS := -Wall -Wextra -Wshadow -Wswitch-enum -pedantic
CNOWARNINGS := -Wno-strict-prototypes -Wno-gnu-binary-literal
SOURCES := main.c sound.c vm.c

chp8:
> $(CC) -O3 $(CWARNINGS) $(CNOWARNINGS) $(CFLAGS) $(SOURCES) -o chp8

debug:
> $(CC) -O0 -g3 -DDEBUG $(CWARNINGS) $(CNOWARNINGS) $(CFLAGS) $(SOURCES) -o chp8

