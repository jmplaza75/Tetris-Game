#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "game.h"
#include "rotation.h"
#include "collision.h"

void test_rotation(void)
{
    Board board;
    board_init(&board);
    for (int type = 0; type < PIECE_COUNT; ++type) {
        for (int direction = -1; direction <= 1; direction += 2) {
            Piece piece;
            assert(piece_spawn(&piece, (PieceType)type));
            const Piece original = piece;
            for (int turn = 0; turn < 4; ++turn) {
                const bool rotated = rotation_try(&board, &piece, direction);
                assert(rotated == (type != PIECE_O));
                int cells = 0;
                for (int y = 0; y < 4; ++y)
                    for (int x = 0; x < 4; ++x) cells += piece_has_cell(&piece, x, y);
                assert(cells == 4);
                assert(piece.x == original.x && piece.y == original.y);
            }
            assert(piece.shape == original.shape && piece.orientation == ORIENTATION_0);
        }
    }
    Piece piece;
    assert(piece_spawn(&piece, PIECE_T));
    assert(rotation_try(&board, &piece, 1));
    assert(piece.shape == 0x0262);
    piece.x = -1;
    assert(rotation_try(&board, &piece, -1));
    assert(piece.x == 0 && piece.orientation == ORIENTATION_0);
    assert(rotation_try(&board, &piece, -1));
    piece.x = 8;
    assert(rotation_try(&board, &piece, 1));
    assert(piece.x == 7);
    assert(piece_spawn(&piece, PIECE_T));
    piece.y = 22;
    assert(rotation_try(&board, &piece, 1));
    assert(piece.x == 2 && piece.y == 21);
    assert(!collision_at(&board, &piece, piece.x, piece.y));

    assert(piece_spawn(&piece, PIECE_I));
    assert(rotation_try(&board, &piece, 1));
    assert(piece.shape == 0x4444);
    piece.x = -2;
    assert(rotation_try(&board, &piece, -1));
    assert(piece.x == 0 && piece.shape == 0x00F0);
    piece.y = 22;
    assert(rotation_try(&board, &piece, 1));
    assert(piece.y == 20 && piece.x == 1);

    assert(piece_spawn(&piece, PIECE_T));
    for (int y = 0; y < BOARD_ROWS; ++y) board.rows[y] = 0x03FF;
    for (int y = 0; y < 4; ++y)
        for (int x = 0; x < 4; ++x)
            if (piece_has_cell(&piece, x, y)) board.rows[piece.y + y] &= (uint16_t)~(1U << (piece.x + x));
    const Piece saved = piece;
    assert(!rotation_try(&board, &piece, 1));
    assert(!rotation_try(&board, &piece, -1));
    assert(memcmp(&saved, &piece, sizeof piece) == 0);

    Game game;
    game_init(&game);
    game_set_soft_drop(&game, true);
    assert(game.current_piece.y == 5);
    game_set_soft_drop(&game, true);
    assert(game.current_piece.y == 5);
    game_update(&game, 0.1);
    assert(game.current_piece.y == 8);
    game_set_soft_drop(&game, false);
    game_update(&game, 0.9);
    assert(game.current_piece.y == 8);
    game_update(&game, 0.1);
    assert(game.current_piece.y == 9);
    game_set_soft_drop(&game, true);
    game_release_input(&game);
    assert(!game.down_held);

    game_init(&game);
    game_set_soft_drop(&game, true);
    game_update(&game, 0.6);
    assert(game.current_piece.y == 22 && game.current_piece.type == PIECE_T);
    assert(game.board.rows[23] == 0);
    game_update(&game, 0.5);
    assert(game.current_piece.type == PIECE_Z);
    assert(game.board.rows[23] == 0x38);

    game_init(&game);
    game.current_piece.y = 22;
    game_update(&game, 0.4);
    assert(game_rotate(&game, 1));
    assert(game.lock_elapsed == 0.0 && game.lock_resets == 1);
    game.state = STATE_GAME_OVER;
    assert(!game_rotate(&game, -1));
    game_set_soft_drop(&game, true);
    assert(!game.down_held);
    puts("SRS rotation, kicks and fast soft drop: OK");

    game_init(&game);
    assert(piece_spawn(&game.current_piece, PIECE_I));
    assert(game_rotate(&game, 1));
    for (int y = 20; y < 24; ++y) game.board.rows[y] = (uint16_t)(0x03FF & ~(1U << 5));
    game_set_soft_drop(&game, true);
    game_update(&game, 1.1);
    assert(game.lines_cleared == 4);
    for (int y = 0; y < BOARD_ROWS; ++y) assert(game.board.rows[y] == 0);
}
