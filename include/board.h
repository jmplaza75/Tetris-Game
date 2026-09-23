#ifndef TETRIS_BOARD_H
#define TETRIS_BOARD_H

#include <stdbool.h>
#include <stdint.h>
#include "piece.h"

enum {
    BOARD_COLUMNS = 10,
    BOARD_VISIBLE_ROWS = 20,
    BOARD_HIDDEN_ROWS = 4,
    BOARD_ROWS = BOARD_VISIBLE_ROWS + BOARD_HIDDEN_ROWS
};

/* Bit x represents column x; row zero is the top hidden row. */
typedef struct {
    uint16_t rows[BOARD_ROWS];
    /* Zero means no stored color; otherwise PieceType + 1. */
    uint8_t colors[BOARD_ROWS][BOARD_COLUMNS];
} Board;

void board_init(Board *board);
bool board_is_occupied(const Board *board, int x, int y);
bool board_lock_piece(Board *board, const Piece *piece);
unsigned int board_clear_lines(Board *board);

#endif
