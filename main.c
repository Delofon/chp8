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

#include "vm.h"
#include "sound.h"

void cls()
{
    printf("\e[1;1H\e[2J");
}

void usage()
{
    printf("usage: chp8 <program>\n");
}

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

    uint8_t *mem = malloc(4096);
    if(!mem)
    {
        fprintf(stderr, "error: could not allocate memory: %s\n", strerror(errno));
        return 2;
    }

    fread(mem+0x0200, 1, 0x0fff, progfile);
    if(ferror(progfile))
    {
        fprintf(stderr, "error: could not read program file: %s\n", strerror(errno));
        return 3;
    }

    fclose(progfile);

    int paerror = 0;
    pa_simple *s = pabegin(&paerror);
    if(paerror != PA_OK)
    {
        fprintf(stderr, "error: could not init pulseaudio: %s\n", pa_strerror(paerror));
        return 4;
    }

    vm_t vm =
    {
        .V = { 0 },
        .I = 0,
        .PC = 0x0200,
        .SP = 0,
        .halt = false
    };

    WINDOW* stdscr = initscr();
    
    noecho();
    nodelay(stdscr, true);



    endwin(); 

    paend(s);
    free(mem);
    return 0;
}

