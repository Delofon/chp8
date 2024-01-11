#include <sys/stat.h>
#include <pulse/simple.h>
#include <pulse/error.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <ncurses.h>

#include "main.h"
#include "vm.h"
#include "sound.h"

void usage()
{
    printf("usage: chp8 <program>\n");
}

pa_simple *s;
uint8_t screen[SCREEN_WIDTH * SCREEN_HEIGHT];

int main(int argc, char **argv)
{
    if(argc == 1)
    {
        usage();
        return 0;
    }

    FILE* progfile = fopen(argv[1], "rb");
    if(!progfile)
    {
        fprintf(stderr, "error: could not open file \"%s\": %s\n", argv[1], strerror(errno));
        return 1;
    }

    vm_t vm =
    {
        .stack = { 0 },
        .V = { 0 },
        .I = 0,
        .PC = 0x0200,
        .SP = 0,
        .halt = false,

        .delay = 0,
        .sound = 0
    };

    vm.mem = malloc(4096);
    if(!vm.mem)
    {
        fprintf(stderr, "error: could not allocate memory: %s\n", strerror(errno));
        return 2;
    }

    fread(vm.mem, 1, 0x0fff, progfile);
    if(ferror(progfile))
    {
        fprintf(stderr, "error: could not read program file: %s\n", strerror(errno));
        return 3;
    }

    fclose(progfile);

    int paerror = 0;
    s = pabegin(&paerror);
    if(paerror != PA_OK)
    {
        fprintf(stderr, "error: could not init pulseaudio: %s\n", pa_strerror(paerror));
        return 4;
    }

    WINDOW* stdscr = initscr();
    
    noecho();
    nodelay(stdscr, true);

    clock_t target = CLOCKS_PER_SEC / TARGET_HZ;

    while(!vm.halt)
    {
        clock_t start = clock();
        step(&vm);

        while(clock() - start < target);

        refresh();
    }

    endwin(); 

    paend(s);
    free(vm.mem);
    return 0;
}

void pabeep()
{
    paplay(s, 440, 1000);
}

int input()
{
    const int mapping[16] =
    { 
        'X',
        '1',
        '2',
        '3',
        'Q',
        'W',
        'E',
        'A',
        'S',
        'D',
        'Z',
        'C',
        '4',
        'R',
        'F',
        'V'
    };
    int ch = getch();

    for(int i = 0; i < 16; i++)
    {
        if(ch == mapping[i]) 
            return i;
    }

    return -1;
}

void draw(vm_t *vm, int x, int y, int n)
{
    uint8_t *sprite = vm->mem + (15*n);

    int index = x + y * SCREEN_WIDTH;
    for(int i = 0; i < 15; i++)
    {
    }
}

