#include <SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "input.h"
#include "renderer.h"

static const double FIXED_STEP_SECONDS = 1.0 / 60.0;
static const double MAX_FRAME_SECONDS = 0.25;
static const double TARGET_FRAME_MS = 1000.0 / 60.0;
enum { SMOKE_TEST_FRAMES = 3 };

static int run_loop(Renderer *renderer, bool smoke_test)
{
    Game game;
    game_init(&game);
    const double frequency = (double)SDL_GetPerformanceFrequency();
    Uint64 previous = SDL_GetPerformanceCounter();
    double accumulator = 0.0;
    unsigned int frames = 0;

    while (game.running) {
        const Uint64 frame_start = SDL_GetPerformanceCounter();
        double elapsed = (double)(frame_start - previous) / frequency;
        previous = frame_start;
        /* Bound catch-up work after a debugger stop or suspended window. */
        if (elapsed > MAX_FRAME_SECONDS) {
            elapsed = MAX_FRAME_SECONDS;
        }
        accumulator += elapsed;

        input_process(&game);
        if (!game.running) {
            break;
        }
        while (accumulator >= FIXED_STEP_SECONDS) {
            game_update(&game, FIXED_STEP_SECONDS);
            accumulator -= FIXED_STEP_SECONDS;
        }

        if (!renderer_draw(renderer)) {
            fprintf(stderr, "Render failed: %s\n", SDL_GetError());
            return EXIT_FAILURE;
        }
        if (smoke_test && ++frames >= SMOKE_TEST_FRAMES) {
            game_request_quit(&game);
        }

        /* Avoid busy spinning when VSync is unavailable or faster than 60 Hz. */
        const double frame_ms =
            (double)(SDL_GetPerformanceCounter() - frame_start) * 1000.0 / frequency;
        if (frame_ms < TARGET_FRAME_MS) {
            SDL_Delay((Uint32)(TARGET_FRAME_MS - frame_ms));
        }
    }
    return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
    const bool smoke_test = argc == 2 && strcmp(argv[1], "--smoke-test") == 0;
    if (argc > 1 && !smoke_test) {
        fprintf(stderr, "Usage: %s [--smoke-test]\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        fprintf(stderr, "SDL initialization failed: %s\n", SDL_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }

    Renderer renderer = {0};
    int result = EXIT_FAILURE;
    if (renderer_init(&renderer)) {
        result = run_loop(&renderer, smoke_test);
    } else {
        fprintf(stderr, "Window/renderer initialization failed: %s\n", SDL_GetError());
    }
    renderer_destroy(&renderer);
    SDL_Quit();
    return result;
}
