#ifndef TETRIS_GAME_H
#define TETRIS_GAME_H

#include <stdbool.h>
#include "board.h"
#include "piece.h"

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
} Game;

void game_init(Game *game);
void game_update(Game *game, double delta_seconds);
void game_request_quit(Game *game);
/* Direction is -1 (left) or +1 (right). Last pressed direction wins. */
void game_set_horizontal(Game *game, int direction, bool pressed);
void game_release_input(Game *game);
void game_set_soft_drop(Game *game, bool pressed);
bool game_rotate(Game *game, int direction);

#endif
