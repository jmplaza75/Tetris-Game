#ifndef TETRIS_ROTATION_H
#define TETRIS_ROTATION_H
#include "board.h"
/* direction: +1 clockwise, -1 counterclockwise; failure leaves piece unchanged. */
bool rotation_try(const Board *board, Piece *piece, int direction);
#endif
