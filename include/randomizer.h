#ifndef TETRIS_RANDOMIZER_H
#define TETRIS_RANDOMIZER_H
#include "piece.h"

typedef struct {
    uint64_t state;
    PieceType bag[PIECE_COUNT];
    unsigned int cursor;
} Randomizer;

void randomizer_init(Randomizer *randomizer, uint64_t seed);
PieceType randomizer_next(Randomizer *randomizer);
#endif
