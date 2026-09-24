#ifndef TETRIS_GAME_H
#define TETRIS_GAME_H

#include <stdbool.h>
#include "board.h"
#include "piece.h"
#include "randomizer.h"
#include "scoring.h"

enum { NEXT_PIECE_COUNT = 5 };

#define GRAVITY_INTERVAL_SECONDS 1.0
#define SOFT_DROP_INTERVAL_SECONDS (1.0 / 30.0)
#define DAS_DELAY_SECONDS 0.150
#define ARR_INTERVAL_SECONDS 0.040
#define LOCK_DELAY_SECONDS 0.500
#define LOCK_RESET_LIMIT 15

typedef enum { STATE_PLAYING, STATE_GAME_OVER } GameState;

/* The active piece remains separate from the locked cells in the board. */
typedef struct {
    bool running;
    GameState state;
    Board board;
    Piece current_piece;
    double gravity_elapsed;
    double repeat_remaining;
    bool left_held;
    bool right_held;
    bool down_held;
    int horizontal_direction;
    double lock_elapsed;
    unsigned int lock_resets;
    unsigned int lines_cleared;
    uint64_t score;
    unsigned int level;
    Randomizer randomizer;
    PieceType next[NEXT_PIECE_COUNT];
    PieceType held_piece; /* PIECE_COUNT means empty. */
    bool hold_used;
} Game;

void game_init(Game *game);
void game_init_seed(Game *game, uint64_t seed);
bool game_hold(Game *game);
/* Read-only landing query; false when no playable, valid active piece exists. */
bool game_ghost_piece(const Game *game, Piece *ghost);
bool game_hard_drop(Game *game);
void game_update(Game *game, double delta_seconds);
void game_request_quit(Game *game);
/* Direction is -1 (left) or +1 (right). Last pressed direction wins. */
void game_set_horizontal(Game *game, int direction, bool pressed);
void game_release_input(Game *game);
void game_set_soft_drop(Game *game, bool pressed);
bool game_rotate(Game *game, int direction);

#endif
