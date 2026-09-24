#ifndef TETRIS_SCORING_H
#define TETRIS_SCORING_H

#include <stdint.h>

enum { LINES_PER_LEVEL = 10, SOFT_DROP_POINTS = 1, HARD_DROP_POINTS = 2 };
unsigned int scoring_level(unsigned int lines);
uint64_t scoring_lines(unsigned int cleared, unsigned int level);
double scoring_gravity_interval(unsigned int level);

#endif
