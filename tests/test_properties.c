#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "game.h"
#include "collision.h"
#include "rotation.h"

/* A cell matrix and one-row-at-a-time removal provide a reference independent
 * of the engine's bit masks and bottom-up compaction. */
static void test_compaction_reference(void)
{
    uint32_t seed = 0x12345678;
    for (int trial = 0; trial < 1024; ++trial) {
        Board board;
        board_init(&board);
        uint8_t cells[BOARD_ROWS][BOARD_COLUMNS] = {{0}};
        for (int y = 0; y < BOARD_ROWS; ++y) {
            seed = seed * 1664525U + 1013904223U;
            const bool full = (seed >> 28) < 5;
            for (int x = 0; x < BOARD_COLUMNS; ++x) {
                seed = seed * 1664525U + 1013904223U;
                if (full || (seed >> 28) < 8) {
                    cells[y][x] = (uint8_t)(1 + (seed >> 16) % PIECE_COUNT);
                    board.rows[y] |= (uint16_t)(1U << x);
                    board.colors[y][x] = cells[y][x];
                }
            }
        }
        unsigned int count = 0;
        for (int y = 0; y < BOARD_ROWS; ++y) {
            bool full = true;
            for (int x = 0; x < BOARD_COLUMNS; ++x) full &= cells[y][x] != 0;
            if (!full) continue;
            ++count;
            for (int row = y; row > 0; --row) memcpy(cells[row], cells[row - 1], sizeof cells[row]);
            memset(cells[0], 0, sizeof cells[0]);
        }
        assert(board_clear_lines(&board) == count);
        for (int y = 0; y < BOARD_ROWS; ++y) {
            for (int x = 0; x < BOARD_COLUMNS; ++x) {
                assert(board_is_occupied(&board, x, y) == (cells[y][x] != 0));
                assert(board.colors[y][x] == cells[y][x]);
            }
        }
        assert(board_clear_lines(&board) == 0);
    }
}

static void test_placement_reference(void)
{
    Board empty;
    board_init(&empty);
    for (int type = 0; type < PIECE_COUNT; ++type) {
        Piece shape;
        assert(piece_spawn(&shape, (PieceType)type));
        for (int turn = 0; turn < 4; ++turn) {
            for (int y = -4; y <= BOARD_ROWS; ++y) {
                for (int x = -4; x <= BOARD_COLUMNS; ++x) {
                    Board board = empty;
                    /* Include hidden cells and an irregular stack. */
                    board.rows[0] = 0x101;
                    board.rows[18] = 0x155;
                    board.rows[23] = 0x2AA;
                    Board expected = board;
                    Piece piece = shape;
                    piece.x = x;
                    piece.y = y;
                    bool blocked = false;
                    for (int bit = 0; bit < 16; ++bit) {
                        if (!(shape.shape & (1U << bit))) continue;
                        const int column = x + bit % 4, row = y + bit / 4;
                        if (column < 0 || column >= BOARD_COLUMNS || row < 0 || row >= BOARD_ROWS) {
                            blocked = true;
                        } else if ((board.rows[row] >> column) & 1U) {
                            blocked = true;
                        } else {
                            expected.rows[row] |= (uint16_t)(1U << column);
                            expected.colors[row][column] = (uint8_t)(type + 1);
                        }
                    }
                    const Board before = board;
                    assert(collision_at(&board, &piece, x, y) == blocked);
                    assert(board_lock_piece(&board, &piece) == !blocked);
                    if (blocked) expected = before;
                    assert(memcmp(board.rows, expected.rows, sizeof board.rows) == 0);
                    assert(memcmp(board.colors, expected.colors, sizeof board.colors) == 0);
                }
            }
            (void)rotation_try(&empty, &shape, 1);
        }
    }
}

static void test_lock_reset_limit(void)
{
    Game game;
    game_init(&game);
    assert(piece_spawn(&game.current_piece, PIECE_O));
    game.current_piece.y = BOARD_ROWS - 2;
    for (unsigned int i = 0; i < LOCK_RESET_LIMIT; ++i) {
        game_update(&game, 0.1);
        const int direction = i % 2 ? -1 : 1;
        game_set_horizontal(&game, direction, true);
        game_set_horizontal(&game, direction, false);
        assert(game.lock_resets == i + 1 && game.lock_elapsed == 0);
        assert(game.board.rows[BOARD_ROWS - 1] == 0);
    }
    game_update(&game, 0.4);
    game_set_horizontal(&game, -1, true);
    game_set_horizontal(&game, -1, false);
    assert(game.lock_resets == LOCK_RESET_LIMIT && game.lock_elapsed > 0.39);
    game_update(&game, 0.1);
    assert(game.board.rows[BOARD_ROWS - 1] != 0);
}

void test_properties(void)
{
    test_lock_reset_limit();
    test_compaction_reference();
    test_placement_reference();
    puts("1024 reference boards and 12180 collision/lock placements: OK");
}
