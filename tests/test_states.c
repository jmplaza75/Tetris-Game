#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "game.h"

void test_states(void)
{
    Game game;
    game_init(&game);
    game_set_horizontal(&game, 1, true);
    game_set_soft_drop(&game, true);
    game_update(&game, 0.01);
    const double gravity = game.gravity_elapsed;
    const double repeat = game.repeat_remaining;
    game_toggle_pause(&game);
    assert(game.state == STATE_PAUSED);
    assert(!game.left_held && !game.right_held && !game.down_held);
    assert(game.horizontal_direction == 0);
    assert(game.gravity_elapsed == gravity && game.repeat_remaining == repeat);
    Game saved = game;
    game_update(&game, 100.0);
    game_set_horizontal(&game, -1, true);
    game_set_soft_drop(&game, true);
    game_set_soft_drop(&game, false);
    game_release_input(&game);
    assert(!game_hold(&game) && !game_rotate(&game, 1) && !game_hard_drop(&game));
    assert(memcmp(&saved, &game, sizeof game) == 0);
    game_toggle_pause(&game);
    const int y = game.current_piece.y;
    game_update(&game, 1.0 - gravity);
    assert(game.current_piece.y == y + 1);

    /* A paused grounded piece keeps precisely its remaining lock delay. */
    game_init(&game);
    Piece ghost;
    assert(game_ghost_piece(&game, &ghost));
    game.current_piece = ghost;
    game_update(&game, 0.3);
    game_toggle_pause(&game);
    const double lock = game.lock_elapsed;
    game_update(&game, 30);
    assert(game.lock_elapsed == lock);
    game_toggle_pause(&game);
    game_update(&game, 0.19);
    assert(game.board.rows[BOARD_ROWS - 1] == 0);
    game_update(&game, 0.01);
    assert(game.board.rows[BOARD_ROWS - 1] != 0);

    /* Actual blocked spawn, frozen inputs, then full reset from every state. */
    game_init(&game);
    assert(piece_spawn(&game.current_piece, PIECE_T));
    game.current_piece.y = BOARD_ROWS - 2;
    game.next[0] = PIECE_Z;
    game.board.rows[4] = 1U << 3;
    assert(game_hard_drop(&game));
    assert(game.state == STATE_GAME_OVER);
    saved = game;
    game_toggle_pause(&game);
    game_update(&game, 5);
    assert(!game_hold(&game) && !game_hard_drop(&game) && !game_rotate(&game, 1));
    assert(memcmp(&saved, &game, sizeof game) == 0);
    for (int state = STATE_PLAYING; state <= STATE_GAME_OVER; ++state) {
        game.state = (GameState)state;
        game.score = 9999;
        game.lines_cleared = 45;
        game.level = 5;
        game.hold_used = true;
        game.held_piece = PIECE_I;
        game.left_held = game.down_held = true;
        game.lock_resets = 12;
        game.gravity_elapsed = game.lock_elapsed = game.repeat_remaining = 0.2;
        game_restart(&game, 1234);
        Game fresh;
        game_init_seed(&fresh, 1234);
        assert(memcmp(&game, &fresh, sizeof game) == 0);
    }
    game_request_quit(&game);
    saved = game;
    game_restart(&game, 999);
    game_toggle_pause(&game);
    assert(memcmp(&saved, &game, sizeof game) == 0);
    puts("Pause timers, blocked spawn and complete restart: OK");
}
