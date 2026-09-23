#include "renderer.h"

static const SDL_Color BACKGROUND_COLOR = {18, 23, 34, 255};

bool renderer_init(Renderer *renderer)
{
    *renderer = (Renderer){0};
    renderer->window = SDL_CreateWindow(
        "Tetris — C / Apple Silicon", SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if (renderer->window == NULL) {
        return false;
    }

    renderer->handle = SDL_CreateRenderer(
        renderer->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer->handle == NULL) {
        /* Also permits startup with SDL's headless dummy video driver. */
        renderer->handle = SDL_CreateRenderer(renderer->window, -1,
                                               SDL_RENDERER_SOFTWARE);
    }
    if (renderer->handle == NULL) {
        return false;
    }

    return SDL_RenderSetLogicalSize(renderer->handle, WINDOW_WIDTH,
                                   WINDOW_HEIGHT) == 0;
}

bool renderer_draw(Renderer *renderer)
{
    if (SDL_SetRenderDrawColor(renderer->handle, BACKGROUND_COLOR.r,
                               BACKGROUND_COLOR.g, BACKGROUND_COLOR.b,
                               BACKGROUND_COLOR.a) != 0 ||
        SDL_RenderClear(renderer->handle) != 0) {
        return false;
    }
    SDL_RenderPresent(renderer->handle);
    return true;
}

void renderer_destroy(Renderer *renderer)
{
    if (renderer->handle != NULL) {
        SDL_DestroyRenderer(renderer->handle);
    }
    if (renderer->window != NULL) {
        SDL_DestroyWindow(renderer->window);
    }
    *renderer = (Renderer){0};
}
