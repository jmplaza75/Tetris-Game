#include "collision.h"

bool collision_at(const Board *board, const Piece *piece, int x, int y)
{
    for (int local_y = 0; local_y < PIECE_SIZE; ++local_y) {
        for (int local_x = 0; local_x < PIECE_SIZE; ++local_x) {
            if (!piece_has_cell(piece, local_x, local_y)) {
                continue;
            }
            /* Widen before addition so even invalid candidate coordinates are safe. */
            const int64_t column = (int64_t)x + local_x;
            const int64_t row = (int64_t)y + local_y;
            if (column < 0 || column >= BOARD_COLUMNS || row < 0 || row >= BOARD_ROWS ||
                board_is_occupied(board, (int)column, (int)row)) {
                return true;
            }
        }
    }
    return false;
}
