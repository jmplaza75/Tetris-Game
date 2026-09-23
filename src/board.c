#include "board.h"
#include "collision.h"
#include <string.h>

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

bool board_lock_piece(Board *board, const Piece *piece)
{
    if (piece->type < PIECE_I || piece->type >= PIECE_COUNT ||
        collision_at(board, piece, piece->x, piece->y)) {
        return false;
    }
    for (int y = 0; y < PIECE_SIZE; ++y) {
        for (int x = 0; x < PIECE_SIZE; ++x) {
            if (piece_has_cell(piece, x, y)) {
                const int row = piece->y + y;
                const int column = piece->x + x;
                board->rows[row] |= (uint16_t)(1U << column);
                board->colors[row][column] = (uint8_t)(piece->type + 1);
            }
        }
    }
    return true;
}

unsigned int board_clear_lines(Board *board)
{
    const uint16_t full_row = (uint16_t)((1U << BOARD_COLUMNS) - 1U);
    int destination = BOARD_ROWS - 1;
    unsigned int cleared = 0;
    for (int source = BOARD_ROWS - 1; source >= 0; --source) {
        if ((board->rows[source] & full_row) == full_row) {
            ++cleared;
            continue;
        }
        if (destination != source) {
            board->rows[destination] = board->rows[source];
            memcpy(board->colors[destination], board->colors[source],
                   sizeof board->colors[destination]);
        }
        --destination;
    }
    while (destination >= 0) {
        board->rows[destination] = 0;
        memset(board->colors[destination], 0, sizeof board->colors[destination]);
        --destination;
    }
    return cleared;
}
