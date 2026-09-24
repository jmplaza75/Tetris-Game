/* Deterministic gameplay capture using the production engine and renderer.
 * Pass an existing directory for the BMP frames; no game state is staged. */
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include "game.h"
#include "renderer.h"

enum { DEMO_FPS = 15, DEMO_FRAMES = 240 };

static void controls(Game *game, int frame)
{
    if (frame == 30) (void)game_hold(game);
    if (frame == 150 || frame == 180) game_toggle_pause(game);
    if (frame >= 150 && frame < 180) return;
    const int tick = frame < 150 ? frame : frame - 30;
    const int turn = tick / 30;
    const int phase = tick % 30;
    const int direction = turn % 2 ? 1 : -1;
    if (phase == 5) (void)game_rotate(game, turn % 2 ? 1 : -1);
    if (phase == 9) game_set_horizontal(game, direction, true);
    if (phase == 13) game_set_horizontal(game, direction, false);
    if (phase == 17) game_set_soft_drop(game, true);
    if (phase == 20) game_set_soft_drop(game, false);
    if (phase == 24) (void)game_hard_drop(game);
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s FRAME_DIRECTORY\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (SDL_Init(SDL_INIT_VIDEO) != 0) return EXIT_FAILURE;
    SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(
        0, WINDOW_WIDTH, WINDOW_HEIGHT, 32, SDL_PIXELFORMAT_RGBA32);
    Renderer renderer = {0};
    int result = EXIT_FAILURE;
    if (surface == NULL) goto cleanup;
    renderer.handle = SDL_CreateSoftwareRenderer(surface);
    if (renderer.handle == NULL) goto cleanup;
    Game game;
    game_init_seed(&game, 7);
    for (int frame = 0; frame < DEMO_FRAMES; ++frame) {
        controls(&game, frame);
        for (int step = 0; step < 60 / DEMO_FPS; ++step) game_update(&game, 1.0 / 60.0);
        if (!renderer_draw(&renderer, &game)) goto cleanup;
        char path[1024];
        const int length = snprintf(path, sizeof path, "%s/frame-%04d.bmp", argv[1], frame);
        if (length < 0 || (size_t)length >= sizeof path || SDL_SaveBMP(surface, path) != 0) goto cleanup;
    }
    result = EXIT_SUCCESS;
cleanup:
    if (result != EXIT_SUCCESS) fprintf(stderr, "Demo capture failed: %s\n", SDL_GetError());
    renderer_destroy(&renderer);
    SDL_FreeSurface(surface);
    SDL_Quit();
    return result;
}
