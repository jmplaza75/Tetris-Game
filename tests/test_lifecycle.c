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
