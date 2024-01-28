#include <SDL2/SDL.h>
#include <stdio.h>

#include "vm.h"
#include "screen_sdl.h"

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
        1280, 720,
        SDL_WINDOW_SHOWN
    );

    if(!window)
    {
        fprintf(stderr, "error: could not create window: %s\n", SDL_GetError());
        exit(-1);
    }

    surface = SDL_GetWindowSurface(window);
}

void sdl_end()
{
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void sdl_draw(vm_t *vm)
{
}

void sdl_drawtext(int y, int x, const char *format, ...)
{
}

int sdl_input()
{
}

