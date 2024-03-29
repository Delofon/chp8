#include <stdarg.h>
#define NCURSES_WIDECHAR 1
#include <ncurses.h>

#include "main.h"
#include "vm.h"
#include "screen.h"

#define UPPERHALF L"\u2580"
#define LOWERHALF L"\u2584"
#define FULLBLOCK L"\u2588"

uint8_t usecolor = 0;

void nc_init()
{
#ifndef FLOOD_WITH_OPS
    initscr();
#endif

    savetty();
    curs_set(0);
    cbreak();
    noecho();
    nodelay(stdscr, true);

    if(has_colors())
    {
        start_color();
        use_default_colors();

        init_pair(1, COLOR_BLACK, COLOR_WHITE);
        init_pair(2, COLOR_WHITE, -1);

        usecolor = 1;
    }
}

void nc_end()
{
    resetty();
    endwin();
}

// TODO: make drawing to ncurses screen more configurable
void nc_draw(vm_t *vm)
{
    if(vm->graphicsmode == LORES)
    {
        for(int i = 0; i < SCREEN_SIZE; i++)
        {
            int x, y;
            if(!itocoord(i, &x, &y, SCREEN_WIDTH, SCREEN_SIZE))
                continue;
            move(y, x*2);

            if(!usecolor)
            {
                if(!vm->screen[i]) printw("  ");
                else printw("00");
            }
            else
            {
                if(vm->screen[i]) attron(COLOR_PAIR(1));
                addstr("  ");
                attroff(COLOR_PAIR(1));
            }
        }
    }
    else
    {
        for(int y = 0; y < SCREEN_HEIGHT_HIRES; y+=2)
        {
            for(int x = 0; x < SCREEN_WIDTH_HIRES; x++)
            {
                int i;
                if(!coordtoi(&i, x, y, SCREEN_WIDTH_HIRES, SCREEN_HEIGHT_HIRES))
                    continue;

                move(y/2, x);


                uint8_t up = vm->screenhr[i];
                uint8_t dw = vm->screenhr[i+SCREEN_WIDTH_HIRES];

                attron(COLOR_PAIR(2));
                if(up && !dw)
                {
                    addwstr(UPPERHALF);
                    //printw("0");
                }
                else if(!up && dw)
                {
                    addwstr(LOWERHALF);
                    //printw("1");
                }

                else if (up && dw)
                {
                    addwstr(FULLBLOCK);
                    //printw("2");
                }
                else
                {
                    printw(" ");
                }
                attroff(COLOR_PAIR(2));
            }
        }
    }
}

void nc_drawtext(int y, int x, const char *format, ...)
{
    va_list args;
    move(y, x);
    va_start(args, format);
    vw_printw(stdscr, format, args);
    va_end(args);
}

int nc_input()
{
    return -1;
}

