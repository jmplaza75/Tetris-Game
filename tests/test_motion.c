#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include "collision.h"
#include "game.h"

static void test_collisions(void)
{
    Board board;
    board_init(&board);
    for (int type = 0; type < PIECE_COUNT; ++type) {
        Piece piece;
        assert(piece_spawn(&piece, (PieceType)type));
        assert(!collision_at(&board, &piece, piece.x, piece.y));
        int min_x = PIECE_SIZE, max_x = 0, min_y = PIECE_SIZE, max_y = 0;
        for (int y = 0; y < PIECE_SIZE; ++y) {
            for (int x = 0; x < PIECE_SIZE; ++x) {
                if (!piece_has_cell(&piece, x, y)) continue;
                if (x < min_x) min_x = x;
                if (x > max_x) max_x = x;
                if (y < min_y) min_y = y;
                if (y > max_y) max_y = y;
                board.rows[piece.y + y] = (uint16_t)(1U << (piece.x + x));
                assert(collision_at(&board, &piece, piece.x, piece.y));
                board_init(&board);
            }
        }
        assert(!collision_at(&board, &piece, -min_x, piece.y));
        assert(collision_at(&board, &piece, -min_x - 1, piece.y));
        assert(!collision_at(&board, &piece, BOARD_COLUMNS - 1 - max_x, piece.y));
        assert(collision_at(&board, &piece, BOARD_COLUMNS - max_x, piece.y));
        assert(!collision_at(&board, &piece, piece.x, -min_y));
        assert(collision_at(&board, &piece, piece.x, -min_y - 1));
        assert(!collision_at(&board, &piece, piece.x, BOARD_ROWS - 1 - max_y));
        assert(collision_at(&board, &piece, piece.x, BOARD_ROWS - max_y));
        assert(collision_at(&board, &piece, INT_MAX, INT_MAX));
        assert(collision_at(&board, &piece, INT_MIN, INT_MIN));
    }
    Piece piece;
    assert(piece_spawn(&piece, PIECE_T));
    board.rows[4] = 1U << 3; /* Empty corner of the T's bounding box. */
    assert(!collision_at(&board, &piece, 3, 4));
    board_init(&board);
    board.rows[0] = 1U << 4;
    assert(collision_at(&board, &piece, 3, 0));
}

static void test_gravity(void)
{
    Game single, sliced;
    game_init(&single);
    (void)piece_spawn(&single.current_piece, PIECE_T);
    single.next[0] = PIECE_Z;
    game_init(&sliced);
    (void)piece_spawn(&sliced.current_piece, PIECE_T);
    sliced.next[0] = PIECE_Z;
    game_update(&single, 0.5);
    assert(single.current_piece.y == 4);
    game_update(&single, 0.5);
    assert(single.current_piece.y == 5);
    game_update(&single, 1.5);
    for (int i = 0; i < 150; ++i) game_update(&sliced, 1.0 / 60.0);
    assert(single.current_piece.y == 6);
    assert(single.current_piece.y == sliced.current_piece.y);
    assert(fabs(single.gravity_elapsed - sliced.gravity_elapsed) < 1e-8);
    game_update(&single, NAN);
    game_update(&single, INFINITY);
    game_update(&single, -1.0);
    assert(single.current_piece.y == 6);

    game_init(&single);
    (void)piece_spawn(&single.current_piece, PIECE_T);
    single.next[0] = PIECE_Z;
    single.board.rows[8] = 0x03FF;
    game_update(&single, 2.0);
    assert(single.current_piece.y == 6);
    assert(single.board.rows[8] == 0x03FF);
    assert(single.lock_elapsed == 0.0);
    game_init(&single);
    (void)piece_spawn(&single.current_piece, PIECE_T);
    single.next[0] = PIECE_Z;
    game_update(&single, 18.0);
    assert(single.current_piece.y == 22);
    for (int y = 0; y < BOARD_ROWS; ++y) assert(single.board.rows[y] == 0);
    game_set_horizontal(&single, 1, true);
    assert(single.current_piece.x == 4);
    game_request_quit(&single);
    game_update(&single, 1.0);
    assert(single.current_piece.y == 22);
}

static void test_horizontal(void)
{
    Game game;
    game_init(&game);
    (void)piece_spawn(&game.current_piece, PIECE_T);
    game.next[0] = PIECE_Z;
    game_set_horizontal(&game, -1, true);
    assert(game.current_piece.x == 2);
    game_set_horizontal(&game, -1, true);
    assert(game.current_piece.x == 2);
    game_update(&game, 0.149);
    assert(game.current_piece.x == 2);
    game_update(&game, 0.001);
    assert(game.current_piece.x == 1);
    game_update(&game, 0.040);
    assert(game.current_piece.x == 0);
    game_update(&game, 0.040);
    assert(game.current_piece.x == 0);
    game_set_horizontal(&game, 1, true);
    assert(game.current_piece.x == 1);
    game_set_horizontal(&game, 1, false);
    assert(game.current_piece.x == 0);
    game_release_input(&game);
    game_update(&game, 0.5);
    assert(game.current_piece.x == 0);
    game_set_horizontal(&game, 1, true);
    game_update(&game, 0.5);
    assert(game.current_piece.x == 7);
    game_release_input(&game);
    game_init(&game);
    (void)piece_spawn(&game.current_piece, PIECE_T);
    game.next[0] = PIECE_Z;
    game.board.rows[5] = 1U << 2;
    game_set_horizontal(&game, -1, true);
    assert(game.current_piece.x == 3);
    game_set_horizontal(&game, -1, false);
    game.board.rows[5] = 1U << 6;
    game_set_horizontal(&game, 1, true);
    assert(game.current_piece.x == 3);
    game_init(&game);
    (void)piece_spawn(&game.current_piece, PIECE_T);
    game.next[0] = PIECE_Z;
    assert(!game.left_held && !game.right_held);
    assert(game.horizontal_direction == 0 && game.gravity_elapsed == 0.0);
}

void test_motion(void)
{
    test_collisions();
    test_gravity();
    test_horizontal();
    puts("Collisions, gravity and horizontal repeat: OK");
}
