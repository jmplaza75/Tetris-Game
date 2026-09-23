#ifndef TETRIS_GAME_H
#define TETRIS_GAME_H

#include <stdbool.h>
#include "board.h"
#include "piece.h"

/* The active piece remains separate from the locked cells in the board. */
typedef struct {
    bool running;
    Board board;
    Piece current_piece;
} Game;

void game_init(Game *game);
void game_update(Game *game, double delta_seconds);
void game_request_quit(Game *game);

#endif
