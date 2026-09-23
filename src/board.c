#include "board.h"

void board_init(Board *board)
{
    *board = (Board){0};
}

bool board_is_occupied(const Board *board, int x, int y)
{
    if (x < 0 || x >= BOARD_COLUMNS || y < 0 || y >= BOARD_ROWS) {
        return false;
    }
    return (board->rows[y] & (UINT16_C(1) << x)) != 0;
}
