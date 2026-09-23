#ifndef TETRIS_COLLISION_H
#define TETRIS_COLLISION_H

#include "board.h"
#include "piece.h"

/* Checks occupied cells only, including the hidden rows. Never mutates state. */
bool collision_at(const Board *board, const Piece *piece, int x, int y);

#endif
