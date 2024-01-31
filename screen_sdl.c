#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdarg.h>

#include "main.h"
#include "timing.h"
#include "vm.h"
#include "screen_sdl.h"

#define SDL_SCREEN_WIDTH 1024
#define SDL_SCREEN_HEIGHT 512
#define SDL_PIXEL_WIDTH 16
#define SDL_PIXEL_HEIGHT 16

SDL_Window *window;
SDL_Surface *surface;
SDL_Renderer *renderer;

// TODO: return an error code and handle errors in main.c, maybe?
// TODO: pass arguments such as screen width, screen height
void sdl_init()
{
    if(SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        fprintf(stderr, "error: could not init SDL2: %s\n", SDL_GetError());
        exit(-1);
    }

    window = SDL_CreateWindow
    (
        "chp8",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        SDL_SCREEN_WIDTH, SDL_SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN
    );
    if(!window)
    {
        fprintf(stderr, "error: could not create window: %s\n", SDL_GetError());
        exit(-1);
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if(!renderer)
    {
        fprintf(stderr, "error: could not create renderer: %s\n", SDL_GetError());
        exit(-1);
    }

    surface = SDL_GetWindowSurface(window);

    SDL_StartTextInput();
}

void sdl_end()
{
    SDL_StopTextInput();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void sdl_draw(vm_t *vm)
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    SDL_Rect rect;

    if(vm->graphicsmode == LORES)
    {
        for(int i = 0; i < SCREEN_SIZE; i++)
        {
            if(!itocoord(i, &rect.x, &rect.y, SCREEN_WIDTH, SCREEN_SIZE))
                continue;

            rect.x *= SDL_PIXEL_WIDTH;
            rect.y *= SDL_PIXEL_HEIGHT;
            rect.w = SDL_PIXEL_WIDTH;
            rect.h = SDL_PIXEL_HEIGHT;

            if(vm->screen[i])
                SDL_RenderFillRect(renderer, &rect);
        }
    }
    else
    {
        for(int i = 0; i < SCREEN_SIZE_HIRES; i++)
        {
            if(!itocoord(i, &rect.x, &rect.y, SCREEN_WIDTH_HIRES, SCREEN_SIZE_HIRES))
                continue;

            rect.x *= SDL_PIXEL_WIDTH / 2;
            rect.y *= SDL_PIXEL_HEIGHT / 2;
            rect.w = SDL_PIXEL_WIDTH / 2;
            rect.h = SDL_PIXEL_HEIGHT / 2;

            if(vm->screenhr[i])
                SDL_RenderFillRect(renderer, &rect);
        }
    }

    SDL_RenderPresent(renderer);
}

// TODO: sdl_drawtext
void sdl_drawtext(int y, int x, const char *format, ...)
{
    //char buf[2048];
    //va_list vargs;
    //va_start(vargs, format);
    //vsnprintf(buf, sizeof(buf), format, vargs);
    //va_end(vargs);
}

int pollevent_bf = NOINP_KEYCODE;
timing_t pollevent_buf_cl = 0;
int sdl_input()
{
    const timing_t pollevent_buf_cl_target = hztotiming(GETCH_HZ);
    timing_t cl = now();

    if(cl - pollevent_buf_cl >= pollevent_buf_cl_target)
    {
        SDL_Event event;

        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case SDL_QUIT:
                    pollevent_bf = 'h';
                    break;
                case SDL_TEXTINPUT:
                    pollevent_bf = event.text.text[0];
                    break;
                default:
                    pollevent_bf = NOINP_KEYCODE;
                    break;
            }
        }
    }

    return pollevent_bf;
}

