#ifndef TETRIS_PIECE_H
#define TETRIS_PIECE_H

#include <stdbool.h>
#include <stdint.h>

enum { PIECE_SIZE = 4 };
typedef enum {
    PIECE_I, PIECE_J, PIECE_L, PIECE_O, PIECE_S, PIECE_T, PIECE_Z,
    PIECE_COUNT
} PieceType;

typedef enum {
    ORIENTATION_0, ORIENTATION_R, ORIENTATION_2, ORIENTATION_L
} PieceOrientation;

/* Each nibble is a row; bit (y * 4 + x) is local cell (x, y).
 * Only spawn orientation is implemented in Phase 2. */
typedef struct {
    int x;
    int y;
    PieceType type;
    PieceOrientation orientation;
    uint16_t shape;
} Piece;

bool piece_spawn(Piece *piece, PieceType type);
bool piece_has_cell(const Piece *piece, int x, int y);

#endif
