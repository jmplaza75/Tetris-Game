#include "randomizer.h"

/* SplitMix64: deterministic state owned by each game, including seed zero. */
static uint32_t next_u32(Randomizer *randomizer)
{
    uint64_t value = (randomizer->state += UINT64_C(0x9e3779b97f4a7c15));
    value = (value ^ (value >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    value = (value ^ (value >> 27)) * UINT64_C(0x94d049bb133111eb);
    return (uint32_t)((value ^ (value >> 31)) >> 32);
}

static uint32_t bounded(Randomizer *randomizer, uint32_t bound)
{
    const uint32_t threshold = (uint32_t)(0U - bound) % bound;
    uint32_t value;
    do { value = next_u32(randomizer); } while (value < threshold);
    return value % bound;
}

void randomizer_init(Randomizer *randomizer, uint64_t seed)
{
    *randomizer = (Randomizer){.state = seed, .cursor = PIECE_COUNT};
}

PieceType randomizer_next(Randomizer *randomizer)
{
    if (randomizer->cursor == PIECE_COUNT) {
        for (int i = 0; i < PIECE_COUNT; ++i) randomizer->bag[i] = (PieceType)i;
        /* Fisher-Yates, with rejection sampling to avoid modulo bias. */
        for (int i = PIECE_COUNT - 1; i > 0; --i) {
            const unsigned int j = bounded(randomizer, (uint32_t)i + 1U);
            const PieceType saved = randomizer->bag[i];
            randomizer->bag[i] = randomizer->bag[j];
            randomizer->bag[j] = saved;
        }
        randomizer->cursor = 0;
    }
    return randomizer->bag[randomizer->cursor++];
}
