#include <SDL.h>
#include <assert.h>
#include <stdio.h>

#include "game.h"
#include "input.h"

static void key(SDL_Keycode code, Uint8 repeat)
{
    SDL_Event event = {0};
    event.type = SDL_KEYDOWN;
    event.key.keysym.sym = code;
    event.key.repeat = repeat;
    assert(SDL_PushEvent(&event) == 1);
}

static void test_state_events(void)
{
    Game game;
    game_init(&game);
    key(SDLK_p, 0);
    input_process(&game);
    assert(game.state == STATE_PAUSED);
    key(SDLK_p, 1);
    key(SDLK_SPACE, 0);
    key(SDLK_c, 0);
    input_process(&game);
    assert(game.state == STATE_PAUSED && game.score == 0 && !game.hold_used);
    key(SDLK_p, 0);
    input_process(&game);
    assert(game.state == STATE_PLAYING);
    game.score = 42;
    key(SDLK_r, 1);
    input_process(&game);
    assert(game.score == 42);
    for (int state = STATE_PLAYING; state <= STATE_GAME_OVER; ++state) {
        game.state = (GameState)state;
        game.score = 42;
        key(SDLK_r, 0);
        input_process(&game);
        assert(game.state == STATE_PLAYING && game.score == 0 && game.level == 1);
    }
    key(SDLK_p, 0);
    key(SDLK_ESCAPE, 0);
    key(SDLK_r, 0);
    input_process(&game);
    assert(!game.running);
}

int main(void)
{
    assert(SDL_Init(SDL_INIT_EVENTS) == 0);
    test_state_events();
    Game game;
    game_init(&game);
    (void)piece_spawn(&game.current_piece, PIECE_T);
    game.next[0] = PIECE_Z;
    assert(game.running);
    game_update(&game, 1.0 / 60.0);
    assert(game.running);

    SDL_Event event = {0};
    event.type = SDL_KEYDOWN;
    event.key.keysym.sym = SDLK_LEFT;
    assert(SDL_PushEvent(&event) == 1);
    input_process(&game);
    assert(game.running);
    assert(game.current_piece.x == 2);

    event.key.repeat = 1;
    assert(SDL_PushEvent(&event) == 1);
    input_process(&game);
    assert(game.current_piece.x == 2);
    event.key.repeat = 0;
    event.type = SDL_KEYUP;
    assert(SDL_PushEvent(&event) == 1);
    input_process(&game);
    assert(!game.left_held);
    event.type = SDL_KEYDOWN;
    event.key.keysym.sym = SDLK_RIGHT;
    assert(SDL_PushEvent(&event) == 1);
    input_process(&game);
    assert(game.current_piece.x == 3);
    event.type = SDL_WINDOWEVENT;
    event.window.event = SDL_WINDOWEVENT_FOCUS_LOST;
    assert(SDL_PushEvent(&event) == 1);
    input_process(&game);
    assert(!game.left_held && !game.right_held);
    assert(game.horizontal_direction == 0);

    event = (SDL_Event){0};
    event.type = SDL_KEYDOWN;
    event.key.keysym.sym = SDLK_UP;
    assert(SDL_PushEvent(&event) == 1);
    input_process(&game);
    assert(game.current_piece.orientation == ORIENTATION_R);
    event.key.repeat = 1;
    assert(SDL_PushEvent(&event) == 1);
    input_process(&game);
    assert(game.current_piece.orientation == ORIENTATION_R);
    event.key.repeat = 0;
    event.key.keysym.sym = SDLK_z;
    assert(SDL_PushEvent(&event) == 1);
    input_process(&game);
    assert(game.current_piece.orientation == ORIENTATION_0);
    event.key.keysym.sym = SDLK_x;
    assert(SDL_PushEvent(&event) == 1);
    input_process(&game);
    assert(game.current_piece.orientation == ORIENTATION_R);
    event.key.keysym.sym = SDLK_DOWN;
    assert(SDL_PushEvent(&event) == 1);
    input_process(&game);
    assert(game.down_held && game.current_piece.y == 5);
    event.type = SDL_KEYUP;
    assert(SDL_PushEvent(&event) == 1);
    input_process(&game);
    assert(!game.down_held);
    event.type = SDL_KEYDOWN;
    assert(SDL_PushEvent(&event) == 1);
    input_process(&game);
    event.type = SDL_WINDOWEVENT;
    event.window.event = SDL_WINDOWEVENT_FOCUS_LOST;
    assert(SDL_PushEvent(&event) == 1);
    input_process(&game);
    assert(!game.down_held);

    const PieceType outgoing = game.current_piece.type;
    const PieceType incoming = game.next[0];
    event = (SDL_Event){0};
    event.type = SDL_KEYDOWN;
    event.key.keysym.sym = SDLK_c;
    assert(SDL_PushEvent(&event) == 1);
    input_process(&game);
    assert(game.held_piece == outgoing && game.current_piece.type == incoming);
    assert(game.hold_used);
    event.key.repeat = 1;
    assert(SDL_PushEvent(&event) == 1);
    input_process(&game);
    assert(game.held_piece == outgoing && game.current_piece.type == incoming);

    event = (SDL_Event){0};
    event.type = SDL_KEYDOWN;
    event.key.keysym.sym = SDLK_SPACE;
    const PieceType after_drop = game.next[0];
    assert(SDL_PushEvent(&event) == 1);
    input_process(&game);
    assert(game.current_piece.type == after_drop && game.score > 0);
    const uint64_t score = game.score;
    event.key.repeat = 1;
    assert(SDL_PushEvent(&event) == 1);
    input_process(&game);
    assert(game.score == score && game.current_piece.type == after_drop);
    event.key.repeat = 0;
    event.type = SDL_KEYUP;
    assert(SDL_PushEvent(&event) == 1);
    input_process(&game);
    assert(game.score == score);

    event = (SDL_Event){0};
    event.type = SDL_KEYDOWN;
    event.key.keysym.sym = SDLK_ESCAPE;
    assert(SDL_PushEvent(&event) == 1);
    input_process(&game);
    assert(!game.running);

    game_init(&game);
    (void)piece_spawn(&game.current_piece, PIECE_T);
    game.next[0] = PIECE_Z;
    assert(game.running);
    event.type = SDL_QUIT;
    assert(SDL_PushEvent(&event) == 1);
    input_process(&game);
    assert(!game.running);
    SDL_Quit();
    puts("Lifecycle and exit events: OK");
    return 0;
}
