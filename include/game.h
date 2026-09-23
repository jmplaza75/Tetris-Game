#ifndef TETRIS_GAME_H
#define TETRIS_GAME_H

#include <stdbool.h>

/* The engine has no SDL dependency. Gameplay will be added in later phases. */
typedef struct {
    bool running;
} Game;

void game_init(Game *game);
void game_update(Game *game, double delta_seconds);
void game_request_quit(Game *game);

#endif
