#ifndef TETRIS_RENDERER_H
#define TETRIS_RENDERER_H

#include <stdbool.h>
#include <SDL.h>
#include "game.h"

enum { WINDOW_WIDTH = 800, WINDOW_HEIGHT = 720 };

/* Owns both SDL resources; destroy before SDL_Quit, including on failure. */
typedef struct {
    SDL_Window *window;
    SDL_Renderer *handle;
} Renderer;

bool renderer_init(Renderer *renderer);
bool renderer_draw(Renderer *renderer, const Game *game);
void renderer_destroy(Renderer *renderer);

#endif
