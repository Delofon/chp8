.RECIPEPREFIX = >

CC := gcc
CFLAGS := -g $(shell pkg-config --cflags libpulse-simple ncurses) -lm $(shell pkg-config --libs libpulse-simple ncurses)
CWARNINGS := -Wall -Wextra -Wshadow -Wswitch-enum -pedantic
CNOWARNINGS := -Wno-strict-prototypes -Wno-gnu-binary-literal

chp8:
> $(CC) $(CWARNINGS) $(CNOWARNINGS) $(CFLAGS) main.c sound.c vm.c -o $@

