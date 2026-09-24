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
        } else if (event.type == SDL_WINDOWEVENT &&
                   event.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
            game_release_input(game);
        } else if (event.type == SDL_KEYUP ||
                   (event.type == SDL_KEYDOWN && event.key.repeat == 0)) {
            const int direction = event.key.keysym.sym == SDLK_LEFT ? -1 :
                                  (event.key.keysym.sym == SDLK_RIGHT ? 1 : 0);
            if (direction != 0) {
                game_set_horizontal(game, direction, event.type == SDL_KEYDOWN);
            } else if (event.key.keysym.sym == SDLK_DOWN) {
                game_set_soft_drop(game, event.type == SDL_KEYDOWN);
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_x) {
                    (void)game_rotate(game, 1);
                } else if (event.key.keysym.sym == SDLK_z) {
                    (void)game_rotate(game, -1);
                } else if (event.key.keysym.sym == SDLK_SPACE) {
                    (void)game_hard_drop(game);
                } else if (event.key.keysym.sym == SDLK_c) {
                    (void)game_hold(game);
                }
            }
        }
    }
}
