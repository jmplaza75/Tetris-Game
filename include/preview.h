#ifndef TETRIS_PREVIEW_H
#define TETRIS_PREVIEW_H
#include "renderer.h"
bool preview_draw(Renderer *renderer, const Game *game, const SDL_Color colors[PIECE_COUNT]);
#endif
