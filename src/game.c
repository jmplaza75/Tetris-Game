#include "game.h"

void game_init(Game *game)
{
    game->running = true;
    board_init(&game->board);
    (void)piece_spawn(&game->current_piece, PIECE_T);
}

void game_update(Game *game, double delta_seconds)
{
    /* Movement and gravity will be introduced in Phase 3. */
    (void)game;
    (void)delta_seconds;
}

void game_request_quit(Game *game)
{
    game->running = false;
}
