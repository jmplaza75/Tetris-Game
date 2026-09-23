#include "game.h"

void game_init(Game *game)
{
    game->running = true;
}

void game_update(Game *game, double delta_seconds)
{
    /* Phase 1 establishes the update boundary without adding gameplay. */
    (void)game;
    (void)delta_seconds;
}

void game_request_quit(Game *game)
{
    game->running = false;
}
