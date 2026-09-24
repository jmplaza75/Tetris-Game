#include <assert.h>
#include <stdio.h>
#include "game.h"

void test_properties(void);
void test_states(void);
void test_scoring(void);
void test_motion(void);
void test_lines(void);
void test_rotation(void);
void test_randomizer(void);

static void test_board(void)
{
    Board board;
    for (int y = 0; y < BOARD_ROWS; ++y) {
        board.rows[y] = 0x03FF;
    }
    board_init(&board);
    for (int y = 0; y < BOARD_ROWS; ++y) {
        assert(board.rows[y] == 0);
    }
    board.rows[0] = 1;
    board.rows[BOARD_ROWS - 1] = 1 << (BOARD_COLUMNS - 1);
    assert(board_is_occupied(&board, 0, 0));
    assert(board_is_occupied(&board, 9, 23));
    assert(!board_is_occupied(&board, 8, 23));
    assert(!board_is_occupied(&board, -1, 0));
    assert(!board_is_occupied(&board, 10, 0));
    assert(!board_is_occupied(&board, 0, -1));
    assert(!board_is_occupied(&board, 0, 24));
}

static void test_pieces(void)
{
    const char *expected[PIECE_COUNT] = {
        "...." "####" "...." "....",
        "#..." "###." "...." "....",
        "..#." "###." "...." "....",
        ".##." ".##." "...." "....",
        ".##." "##.." "...." "....",
        ".#.." "###." "...." "....",
        "##.." ".##." "...." "...."
    };
    for (int type = PIECE_I; type < PIECE_COUNT; ++type) {
        Piece piece;
        assert(piece_spawn(&piece, (PieceType)type));
        assert(piece.type == (PieceType)type);
        assert(piece.orientation == ORIENTATION_0);
        int occupied = 0;
        for (int y = 0; y < PIECE_SIZE; ++y) {
            for (int x = 0; x < PIECE_SIZE; ++x) {
                const bool cell = piece_has_cell(&piece, x, y);
                assert(cell == (expected[type][y * PIECE_SIZE + x] == '#'));
                if (cell) {
                    ++occupied;
                    assert(piece.x + x >= 0 && piece.x + x < BOARD_COLUMNS);
                    assert(piece.y + y >= BOARD_HIDDEN_ROWS && piece.y + y < BOARD_ROWS);
                }
            }
        }
        assert(occupied == 4);
        assert(!piece_has_cell(&piece, -1, 0));
        assert(!piece_has_cell(&piece, 4, 0));
        assert(!piece_has_cell(&piece, 0, -1));
        assert(!piece_has_cell(&piece, 0, 4));
    }
    Piece piece;
    assert(piece_spawn(&piece, PIECE_T));
    const uint16_t previous_shape = piece.shape;
    assert(!piece_spawn(&piece, PIECE_COUNT));
    assert(!piece_spawn(&piece, (PieceType)-1));
    assert(piece.shape == previous_shape);
}

static void test_initialization(void)
{
    Game game;
    game_init(&game);
    game.board.rows[10] = 0x03FF;
    game.current_piece.x = -20;
    game_request_quit(&game);
    game_init(&game);
    assert(game.running);
    assert(game.current_piece.type >= PIECE_I && game.current_piece.type < PIECE_COUNT);
    assert(game.current_piece.x == 3);
    assert(game.current_piece.y == BOARD_HIDDEN_ROWS);
    game_update(&game, 1.0);
    assert(game.current_piece.y == BOARD_HIDDEN_ROWS + 1);
    for (int y = 0; y < BOARD_ROWS; ++y) {
        assert(game.board.rows[y] == 0);
    }
}

int main(void)
{
    test_board();
    test_pieces();
    test_initialization();
    test_properties();
    test_states();
    test_scoring();
    test_motion();
    test_lines();
    test_rotation();
    test_randomizer();
    puts("Board, seven tetrominoes and spawn: OK");
    return 0;
}
