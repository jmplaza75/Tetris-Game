#include <SDL.h>
#include <assert.h>
#include <stdio.h>

#include "game.h"
#include "input.h"

int main(void)
{
    assert(SDL_Init(SDL_INIT_EVENTS) == 0);
    Game game;
    game_init(&game);
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

    event = (SDL_Event){0};
    event.type = SDL_KEYDOWN;
    event.key.keysym.sym = SDLK_ESCAPE;
    assert(SDL_PushEvent(&event) == 1);
    input_process(&game);
    assert(!game.running);

    game_init(&game);
    assert(game.running);
    event.type = SDL_QUIT;
    assert(SDL_PushEvent(&event) == 1);
    input_process(&game);
    assert(!game.running);
    SDL_Quit();
    puts("Lifecycle and exit events: OK");
    return 0;
}
