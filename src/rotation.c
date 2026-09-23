#include "rotation.h"
#include "collision.h"

typedef struct { int x, y; } Kick;
/* SRS offsets use positive Y upwards, converted at application time.
 * Rows: 0->R, R->0, R->2, 2->R, 2->L, L->2, L->0, 0->L.
 * Reference: https://tetris.wiki/Super_Rotation_System */
static const Kick NORMAL[8][5] = {
    {{0,0},{-1,0},{-1,1},{0,-2},{-1,-2}},
    {{0,0},{1,0},{1,-1},{0,2},{1,2}},
    {{0,0},{1,0},{1,-1},{0,2},{1,2}},
    {{0,0},{-1,0},{-1,1},{0,-2},{-1,-2}},
    {{0,0},{1,0},{1,1},{0,-2},{1,-2}},
    {{0,0},{-1,0},{-1,-1},{0,2},{-1,2}},
    {{0,0},{-1,0},{-1,-1},{0,2},{-1,2}},
    {{0,0},{1,0},{1,1},{0,-2},{1,-2}}
};
static const Kick I_KICKS[8][5] = {
    {{0,0},{-2,0},{1,0},{-2,-1},{1,2}},
    {{0,0},{2,0},{-1,0},{2,1},{-1,-2}},
    {{0,0},{-1,0},{2,0},{-1,2},{2,-1}},
    {{0,0},{1,0},{-2,0},{1,-2},{-2,1}},
    {{0,0},{2,0},{-1,0},{2,1},{-1,-2}},
    {{0,0},{-2,0},{1,0},{-2,-1},{1,2}},
    {{0,0},{1,0},{-2,0},{1,-2},{-2,1}},
    {{0,0},{-1,0},{2,0},{-1,2},{2,-1}}
};

bool rotation_try(const Board *board, Piece *piece, int direction)
{
    if ((direction != 1 && direction != -1) ||
        piece->type < PIECE_I || piece->type >= PIECE_COUNT ||
        piece->orientation < ORIENTATION_0 || piece->orientation > ORIENTATION_L ||
        collision_at(board, piece, piece->x, piece->y)) return false;
    /* O rotates about its own center without changing its occupied cells. */
    if (piece->type == PIECE_O) return false;
    Piece candidate = *piece;
    candidate.orientation = (PieceOrientation)((piece->orientation + (direction == 1 ? 1 : 3)) % 4);
    candidate.shape = 0;
    const int size = piece->type == PIECE_I ? 4 : 3;
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            if (!piece_has_cell(piece, x, y)) continue;
            const int rx = direction == 1 ? size - 1 - y : y;
            const int ry = direction == 1 ? x : size - 1 - x;
            candidate.shape |= (uint16_t)(1U << (ry * PIECE_SIZE + rx));
        }
    }
    static const int cw_index[4] = {0, 2, 4, 6};
    static const int ccw_index[4] = {7, 1, 3, 5};
    const int index = direction == 1 ? cw_index[piece->orientation] : ccw_index[piece->orientation];
    const Kick *kicks = piece->type == PIECE_I ? I_KICKS[index] : NORMAL[index];
    for (int i = 0; i < 5; ++i) {
        candidate.x = piece->x + kicks[i].x;
        candidate.y = piece->y - kicks[i].y;
        if (!collision_at(board, &candidate, candidate.x, candidate.y)) {
            *piece = candidate;
            return true;
        }
    }
    return false;
}
