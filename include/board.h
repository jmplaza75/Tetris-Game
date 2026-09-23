#ifndef TETRIS_BOARD_H
#define TETRIS_BOARD_H

#include <stdbool.h>
#include <stdint.h>

enum {
    BOARD_COLUMNS = 10,
    BOARD_VISIBLE_ROWS = 20,
    BOARD_HIDDEN_ROWS = 4,
    BOARD_ROWS = BOARD_VISIBLE_ROWS + BOARD_HIDDEN_ROWS
};

/* Bit x represents column x; row zero is the top hidden row. */
typedef struct {
    uint16_t rows[BOARD_ROWS];
} Board;

void board_init(Board *board);
bool board_is_occupied(const Board *board, int x, int y);

#endif
