#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "renderer.h"

static Uint32 pixel(SDL_Surface *surface, int x, int y)
{
    Uint32 value;
    memcpy(&value, (Uint8 *)surface->pixels + y * surface->pitch + x * 4, sizeof value);
    return value;
}

int main(void)
{
    assert(SDL_Init(SDL_INIT_VIDEO) == 0);
    SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0, WINDOW_WIDTH, WINDOW_HEIGHT, 32, SDL_PIXELFORMAT_RGBA32);
    assert(surface != NULL);
    Renderer renderer = {.handle = SDL_CreateSoftwareRenderer(surface)};
    assert(renderer.handle != NULL);
    Game game;
    game_init(&game);
    assert(piece_spawn(&game.current_piece, PIECE_O));
    game.score = UINT64_MAX; /* Exercise the compact numeric layout. */
    game.board.rows[0] = 0x3FF; /* Hidden rows must not draw over the title. */
    const Game saved = game;
    assert(renderer_draw(&renderer, &game));
    assert(memcmp(&game, &saved, sizeof game) == 0);
    const int column = game.current_piece.x + 1;
    const int active_x = BOARD_LEFT + column * CELL_SIZE + CELL_SIZE / 2;
    const int active_y = BOARD_TOP + CELL_SIZE / 2;
    const int ghost_y = BOARD_TOP + (BOARD_VISIBLE_ROWS - 2) * CELL_SIZE;
    const Uint32 active = pixel(surface, active_x, active_y);
    const Uint32 background = pixel(surface, active_x, ghost_y + CELL_SIZE / 2);
    assert(active != background);
    assert(pixel(surface, active_x, ghost_y + CELL_INSET) == active);
    const Uint32 playing = pixel(surface, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
    game_toggle_pause(&game);
    assert(renderer_draw(&renderer, &game));
    assert(pixel(surface, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2) != playing);
    assert(pixel(surface, active_x, active_y) == active);
    game.state = STATE_GAME_OVER;
    assert(renderer_draw(&renderer, &game));
    assert(pixel(surface, active_x, active_y) == background);
    renderer_destroy(&renderer);
    renderer_destroy(&renderer); /* Cleanup is safe after partial/previous cleanup. */
    SDL_FreeSurface(surface);

    assert(renderer_init(&renderer));
    SDL_SetWindowSize(renderer.window, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
    for (int state = STATE_PLAYING; state <= STATE_GAME_OVER; ++state) {
        game.state = (GameState)state;
        assert(renderer_draw(&renderer, &game));
        assert(SDL_GetWindowTitle(renderer.window)[0] != '\0');
    }
    int width, height;
    SDL_RenderGetLogicalSize(renderer.handle, &width, &height);
    assert(width == WINDOW_WIDTH && height == WINDOW_HEIGHT);
    renderer_destroy(&renderer);
    SDL_Quit();
    puts("Renderer: ghost, active piece, state overlays, resize and cleanup: OK");
    return 0;
}
