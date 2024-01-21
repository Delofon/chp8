#include <stdarg.h>
#define NCURSES_WIDECHAR 1
#include <ncurses.h>

#include "main.h"
#include "vm.h"
#include "screen.h"

uint8_t usecolor = 0;

void screeninit()
{
#ifndef DEBUG
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

void screenend()
{
    resetty();
    endwin();
}

// TODO: make drawing to ncurses screen more configurable
void screendraw(vm_t *vm)
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
                int i = coordtoi(x, y, SCREEN_WIDTH_HIRES, SCREEN_HEIGHT_HIRES);
                if(i == -1)
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

void screendrawtext(int y, int x, const char *format, ...)
{
    va_list args;
    move(y, x);
    va_start(args, format);
    vw_printw(stdscr, format, args);
    va_end(args);
}

int getch_bf = ERR;
timing_t getch_buf_cl = 0;
int getch_buf()
{
    const timing_t getch_buf_cl_target = hztotiming(GETCH_HZ);
    timing_t cl = now();

    // FIXME: why is getch_bf == ERR needed here?
    if(getch_bf == ERR || cl - getch_buf_cl >= getch_buf_cl_target)
    {
        getch_buf_cl = cl;
        getch_bf = getch();
    }
    return getch_bf;
}

int screeninput()
{
    int ch = getch_buf();
    flushinp();

    if(ch == ERR)
        return NOINP_KEYCODE;

    return ch;
}

