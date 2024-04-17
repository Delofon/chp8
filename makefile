.RECIPEPREFIX = >

CC := gcc
CC_MINGW := x86_64-w64-mingw32-gcc

CFLAGS := $(shell pkg-config --cflags libpulse-simple ncurses sdl2)
LIBS := -lm $(shell pkg-config --libs libpulse-simple ncurses sdl2)

CFLAGS_WIN := -Iexternal/SDL2/include -Dmain=SDL_main -DSDL_MAIN_HANDLED -DNO_PULSE -DNO_NCURSES
LIBS_WIN := -Lexternal/SDL2/lib -lmingw32 -lm -lSDL2main -lSDL2

CWARNINGS := -Wall -Wextra -Werror=shadow -Wswitch-enum -pedantic
CNOWARNINGS := -Wno-strict-prototypes

SOURCES := main.c sound.c vm.c timing.c screen_ncurses.c screen_sdl.c
SOURCES_WIN := main.c vm.c timing.c screen_sdl.c

.PHONY: chp8
chp8: build/chp8
.PHONY: chp8-win
chp8-win: build/chp8-win.exe

build/chp8: $(SOURCES) mkdir
> $(CC) -O2 $(CFLAGS) $(CWARNINGS) $(CNOWARNINGS) $(ARGS) -o $@ $(SOURCES) $(LIBS)

build/chp8-win.exe: $(SOURCES_WIN) external/SDL2 mkdir
> $(CC_MINGW) -O2 $(CFLAGS_WIN) $(CWARNINGS) $(CNOWARNINGS) $(ARGS) -o $@ $(SOURCES_WIN) $(LIBS_WIN)

.PHONY: debug
debug: $(SOURCES) mkdir
> $(CC) -O0 -g3 -DDEBUG $(CFLAGS) $(CWARNINGS) $(CNOWARNINGS) -Wno-format $(ARGS) -o chp8 $^ $(LIBS)

.PHONY: mkdir
mkdir:
> mkdir -p build

.PHONY: clean
clean:
> rm -rf build external

external/SDL2:
> ./download_sdl_win.sh

