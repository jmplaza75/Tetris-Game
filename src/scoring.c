#include "scoring.h"

unsigned int scoring_level(unsigned int lines)
{
    return 1 + lines / LINES_PER_LEVEL;
}

uint64_t scoring_lines(unsigned int cleared, unsigned int level)
{
    static const unsigned int points[] = {0, 100, 300, 500, 800};
    return cleared <= 4 ? (uint64_t)points[cleared] * level : 0;
}

double scoring_gravity_interval(unsigned int level)
{
    /* 20% shorter interval each level, bounded to 60 rows/s. */
    double interval = 1.0;
    const double minimum = 1.0 / 60.0;
    for (unsigned int i = 1; i < level && interval > minimum; ++i) interval *= 0.8;
    return interval < minimum ? minimum : interval;
}
