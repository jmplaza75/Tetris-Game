#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "game.h"
#include "collision.h"

void test_scoring(void)
{
    const unsigned int points[] = {0, 100, 300, 500, 800};
    for (unsigned int n = 0; n <= 4; ++n) assert(scoring_lines(n, 3) == points[n] * 3);
    assert(scoring_level(0) == 1 && scoring_level(9) == 1);
    assert(scoring_level(10) == 2 && scoring_level(30) == 4);
    assert(scoring_gravity_interval(1) == GRAVITY_INTERVAL_SECONDS);
    for (unsigned int level = 2; level < 100; ++level) {
        assert(scoring_gravity_interval(level) <= scoring_gravity_interval(level - 1));
        assert(scoring_gravity_interval(level) >= 1.0 / 60.0);
    }
    Game game;
    for (int type = 0; type < PIECE_COUNT; ++type) {
        for (int orientation = 0; orientation < 4; ++orientation) {
            game_init(&game);
            assert(piece_spawn(&game.current_piece, (PieceType)type));
            for (int i = 0; i < orientation; ++i) (void)game_rotate(&game, 1);
            game.board.rows[20] = 1U << 4;
            Game saved = game;
            Piece ghost;
            assert(game_ghost_piece(&game, &ghost));
            assert(memcmp(&game, &saved, sizeof game) == 0);
            assert(!collision_at(&game.board, &ghost, ghost.x, ghost.y));
            assert(collision_at(&game.board, &ghost, ghost.x, ghost.y + 1));
            Board expected = game.board;
            assert(board_lock_piece(&expected, &ghost));
            const PieceType next = game.next[0];
            game.hold_used = true;
            assert(game_hard_drop(&game));
            assert(game.score == (uint64_t)(ghost.y - saved.current_piece.y) * 2);
            assert(memcmp(&game.board, &expected, sizeof expected) == 0);
            assert(game.current_piece.type == next && !game.hold_used);
            assert(game.gravity_elapsed == 0 && game.lock_elapsed == 0);
        }
    }
    /* Clear 1–4 rows with a vertical I, crossing a level boundary. */
    for (int count = 1; count <= 4; ++count) {
        game_init(&game);
        assert(piece_spawn(&game.current_piece, PIECE_I));
        assert(game_rotate(&game, 1));
        game.current_piece.y = BOARD_ROWS - 4;
        for (int y = BOARD_ROWS - count; y < BOARD_ROWS; ++y)
            game.board.rows[y] = (uint16_t)(0x3FF & ~(1U << 5));
        game.lines_cleared = 9;
        assert(game_hard_drop(&game));
        assert(game.score == points[count]); /* old level, zero drop distance */
        assert(game.lines_cleared == 9U + (unsigned int)count && game.level == 2);
    }
    game_init(&game);
    game_update(&game, 1.0);
    assert(game.score == 0);
    game_set_soft_drop(&game, true);
    assert(game.score == 1);
    game_update(&game, SOFT_DROP_INTERVAL_SECONDS);
    assert(game.score == 2);
    game_set_soft_drop(&game, false);
    Piece ghost;
    assert(game_ghost_piece(&game, &ghost));
    game.current_piece = ghost;
    game_set_soft_drop(&game, true);
    game_update(&game, SOFT_DROP_INTERVAL_SECONDS);
    assert(game.score == 2); /* blocked soft drop earns nothing */
    assert(game_hard_drop(&game) && game.score == 2);
    game_init(&game);
    game.level = 2;
    game.lines_cleared = 10;
    const int start_y = game.current_piece.y;
    game_update(&game, 0.8);
    assert(game.current_piece.y == start_y + 1);
    game.state = STATE_GAME_OVER;
    Game saved = game;
    assert(!game_hard_drop(&game) && !game_ghost_piece(&game, &ghost));
    assert(memcmp(&game, &saved, sizeof game) == 0);
    game_init(&game);
    assert(game.score == 0 && game.level == 1 && game.lines_cleared == 0);
    game_request_quit(&game);
    assert(!game_hard_drop(&game));
    puts("Ghost, hard/soft drop scoring, line awards and levels: OK");
}
