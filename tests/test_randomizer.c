#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "game.h"
#include "collision.h"

static void lock_current(Game *game)
{
    while (!collision_at(&game->board, &game->current_piece,
                          game->current_piece.x, game->current_piece.y + 1)) ++game->current_piece.y;
    game_update(game, LOCK_DELAY_SECONDS);
}

void test_randomizer(void)
{
    for (uint64_t seed = 0; seed < 32; ++seed) {
        Randomizer randomizer, twin;
        randomizer_init(&randomizer, seed);
        randomizer_init(&twin, seed);
        for (int bag = 0; bag < 100; ++bag) {
            unsigned int seen = 0;
            for (int i = 0; i < PIECE_COUNT; ++i) {
                const PieceType type = randomizer_next(&randomizer);
                assert(type >= PIECE_I && type < PIECE_COUNT);
                assert(type == randomizer_next(&twin));
                assert(!(seen & (1U << type)));
                seen |= 1U << type;
            }
            assert(seen == 0x7F);
        }
    }
    Game game;
    game_init_seed(&game, 123);
    Randomizer reference;
    randomizer_init(&reference, 123);
    assert(game.current_piece.type == randomizer_next(&reference));
    for (int i = 0; i < NEXT_PIECE_COUNT; ++i) assert(game.next[i] == randomizer_next(&reference));
    for (int turn = 0; turn < 40; ++turn) {
        const PieceType next = game.next[0];
        PieceType queue[NEXT_PIECE_COUNT];
        memcpy(queue, game.next, sizeof queue);
        lock_current(&game);
        assert(game.current_piece.type == next);
        for (int i = 0; i < NEXT_PIECE_COUNT - 1; ++i) assert(game.next[i] == queue[i + 1]);
        assert(game.next[NEXT_PIECE_COUNT - 1] == randomizer_next(&reference));
        board_init(&game.board);
    }
    game_init_seed(&game, 321);
    assert(game.held_piece == PIECE_COUNT && !game.hold_used);
    const PieceType original = game.current_piece.type, next = game.next[0];
    assert(game_hold(&game));
    assert(game.held_piece == original && game.current_piece.type == next && game.hold_used);
    const Game saved = game;
    assert(!game_hold(&game));
    assert(memcmp(&saved, &game, sizeof game) == 0);
    lock_current(&game);
    assert(!game.hold_used);
    PieceType queue[NEXT_PIECE_COUNT];
    memcpy(queue, game.next, sizeof queue);
    Randomizer rng = game.randomizer;
    const PieceType outgoing = game.current_piece.type;
    (void)game_rotate(&game, 1);
    game.current_piece.y += 3;
    game.gravity_elapsed = 0.4;
    game.lock_elapsed = 0.2;
    game.lock_resets = 7;
    assert(game_hold(&game));
    assert(game.current_piece.type == original && game.held_piece == outgoing);
    assert(game.current_piece.orientation == ORIENTATION_0 && game.current_piece.y == 4 && game.current_piece.x == 3);
    assert(game.gravity_elapsed == 0.0 && game.lock_elapsed == 0.0 && game.lock_resets == 0);
    assert(memcmp(queue, game.next, sizeof queue) == 0);
    assert(memcmp(&rng, &game.randomizer, sizeof rng) == 0);
    game_init_seed(&game, 123);
    game.current_piece.y = 15;
    game.board.rows[4] = game.board.rows[5] = 0x03FF;
    const Board board = game.board;
    assert(game_hold(&game));
    assert(game.state == STATE_GAME_OVER);
    assert(memcmp(&board, &game.board, sizeof board) == 0);
    assert(!game_hold(&game));
    game_init_seed(&game, 123);
    assert(!game.hold_used && game.held_piece == PIECE_COUNT && game.state == STATE_PLAYING);
    puts("7-bag, queue continuity, hold and blocked spawn: OK");
}
