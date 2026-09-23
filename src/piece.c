#include "piece.h"
#include "board.h"

static const uint16_t SPAWN_SHAPES[PIECE_COUNT] = {
    [PIECE_I] = 0x00F0,
    [PIECE_J] = 0x0071,
    [PIECE_L] = 0x0074,
    [PIECE_O] = 0x0066,
    [PIECE_S] = 0x0036,
    [PIECE_T] = 0x0072,
    [PIECE_Z] = 0x0063
};

bool piece_spawn(Piece *piece, PieceType type)
{
    if (type < PIECE_I || type >= PIECE_COUNT) {
        return false;
    }
    *piece = (Piece){
        .x = (BOARD_COLUMNS - PIECE_SIZE) / 2,
        /* Spawn fully visible; hidden rows remain available for future spawn rules. */
        .y = BOARD_HIDDEN_ROWS,
        .type = type,
        .orientation = ORIENTATION_0,
        .shape = SPAWN_SHAPES[type]
    };
    return true;
}

bool piece_has_cell(const Piece *piece, int x, int y)
{
    if (x < 0 || x >= PIECE_SIZE || y < 0 || y >= PIECE_SIZE) {
        return false;
    }
    return (piece->shape & (UINT16_C(1) << (y * PIECE_SIZE + x))) != 0;
}
