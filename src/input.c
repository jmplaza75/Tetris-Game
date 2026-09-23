#include "input.h"

#include <SDL.h>

void input_process(Game *game)
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT ||
            (event.type == SDL_KEYDOWN &&
             event.key.keysym.sym == SDLK_ESCAPE)) {
            game_request_quit(game);
        }
    }
}
