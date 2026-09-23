#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "game.h"

void test_lines(void)
{
    Board board;
    board_init(&board);
    assert(board_clear_lines(&board) == 0);
    for (int count = 1; count <= 4; ++count) {
        board_init(&board);
        board.rows[0] = 1;
        board.colors[0][0] = PIECE_I + 1;
        board.rows[19] = 2;
        board.colors[19][1] = PIECE_T + 1;
        for (int y = 24 - count; y < 24; ++y) {
            board.rows[y] = 0x03FF;
            memset(board.colors[y], PIECE_O + 1, BOARD_COLUMNS);
        }
        assert(board_clear_lines(&board) == (unsigned int)count);
        assert(board.rows[count] == 1);
        assert(board.colors[count][0] == PIECE_I + 1);
        assert(board.rows[19 + count] == 2);
        assert(board.colors[19 + count][1] == PIECE_T + 1);
        for (int y = 0; y < count; ++y) {
            assert(board.rows[y] == 0);
            for (int x = 0; x < 10; ++x) assert(board.colors[y][x] == 0);
        }
        assert(board_clear_lines(&board) == 0);
    }
    board_init(&board);
    board.rows[23] = board.rows[21] = 0x03FF;
    board.rows[22] = 4;
    board.rows[20] = 8;
    assert(board_clear_lines(&board) == 2);
    assert(board.rows[23] == 4 && board.rows[22] == 8);
    for (int y = 0; y < BOARD_ROWS; ++y) board.rows[y] = 0x03FF;
    assert(board_clear_lines(&board) == BOARD_ROWS);
    for (int y = 0; y < BOARD_ROWS; ++y) assert(board.rows[y] == 0);

    Piece piece;
    assert(piece_spawn(&piece, PIECE_T));
    piece.y = 22;
    assert(board_lock_piece(&board, &piece));
    assert(board.rows[22] == 0x10 && board.rows[23] == 0x38);
    assert(board.colors[23][3] == PIECE_T + 1);
    Board saved = board;
    assert(!board_lock_piece(&board, &piece));
    assert(memcmp(&saved, &board, sizeof board) == 0);
    piece.x = -1;
    assert(!board_lock_piece(&board, &piece));
    assert(memcmp(&saved, &board, sizeof board) == 0);

    Game game;
    game_init(&game);
    game.current_piece.y = 22;
    game_update(&game, 0.49);
    assert(game.board.rows[23] == 0);
    game_update(&game, 0.01);
    assert(game.board.rows[23] == 0x38);
    assert(game.current_piece.type == PIECE_Z && game.current_piece.y == 4);
    assert(game.lock_elapsed == 0.0 && game.gravity_elapsed == 0.0);

    game_init(&game);
    game.current_piece.y = 22;
    game.board.rows[23] = (uint16_t)(0x03FF & ~0x38);
    game_update(&game, 0.5);
    assert(game.lines_cleared == 1);
    assert(game.board.rows[23] == 0x10);
    assert(game.board.colors[23][4] == PIECE_T + 1);

    game_init(&game);
    game.current_piece.y = 22;
    game_update(&game, 0.4);
    game_set_horizontal(&game, 1, true);
    game_set_horizontal(&game, 1, false);
    assert(game.lock_resets == 1 && game.lock_elapsed == 0.0);
    game_update(&game, 0.2);
    assert(game.current_piece.type == PIECE_T);
    game.lock_resets = LOCK_RESET_LIMIT;
    game_set_horizontal(&game, -1, true);
    game_set_horizontal(&game, -1, false);
    assert(game.lock_elapsed > 0.19);
    game_update(&game, 0.3);
    assert(game.current_piece.type == PIECE_Z);

    game_init(&game);
    game.current_piece.y = 22;
    game.board.rows[4] = 1U << 3; /* Blocks the next Z's spawn. */
    game_update(&game, 0.5);
    assert(game.state == STATE_GAME_OVER && game.running);
    saved = game.board;
    game_update(&game, 5.0);
    game_set_horizontal(&game, 1, true);
    assert(memcmp(&saved, &game.board, sizeof saved) == 0);
    assert(game.horizontal_direction == 0);
    game_init(&game);
    assert(game.state == STATE_PLAYING && game.lines_cleared == 0);
    puts("Lock delay, row compaction, colors and next spawn: OK");
}
