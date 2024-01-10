.RECIPEPREFIX = >

CC := gcc
CFLAGS := -O3 -Wall -Wextra -Wshadow -Wswitch-enum -pedantic $(shell pkg-config --cflags libpulse-simple ncurses) -lm $(shell pkg-config --libs libpulse-simple ncurses)

chp8:
> $(CC) $(CFLAGS) main.c sound.c -o $@

