#include "game.h"
#include "collision.h"
#include "rotation.h"

#include <math.h>

static bool try_move(Game *game, int dx, int dy)
{
    const int x = game->current_piece.x + dx;
    const int y = game->current_piece.y + dy;
    if (collision_at(&game->board, &game->current_piece, x, y)) {
        return false;
    }
    if (dx != 0 && game->lock_resets < LOCK_RESET_LIMIT &&
        collision_at(&game->board, &game->current_piece,
                     game->current_piece.x, game->current_piece.y + 1)) {
        game->lock_elapsed = 0.0;
        ++game->lock_resets;
    }
    game->current_piece.x = x;
    game->current_piece.y = y;
    return true;
}

void game_init(Game *game)
{
    *game = (Game){.running = true};
    board_init(&game->board);
    (void)piece_spawn(&game->current_piece, PIECE_T);
}

static void lock_and_spawn(Game *game)
{
    if (!board_lock_piece(&game->board, &game->current_piece)) {
        game->state = STATE_GAME_OVER;
        game_release_input(game);
        return;
    }
    game->lines_cleared += board_clear_lines(&game->board);
    /* Temporary deterministic sequence until the Phase 6 seven-bag. */
    const PieceType next = (PieceType)((game->current_piece.type + 1) % PIECE_COUNT);
    (void)piece_spawn(&game->current_piece, next);
    game->gravity_elapsed = 0.0;
    game->lock_elapsed = 0.0;
    game->lock_resets = 0;
    game->repeat_remaining = DAS_DELAY_SECONDS;
    if (collision_at(&game->board, &game->current_piece,
                     game->current_piece.x, game->current_piece.y)) {
        game->state = STATE_GAME_OVER;
        game_release_input(game);
    }
}

static void update_step(Game *game, double delta_seconds)
{
    const bool was_grounded = collision_at(&game->board, &game->current_piece,
                                           game->current_piece.x, game->current_piece.y + 1);
    if (game->horizontal_direction != 0) {
        game->repeat_remaining -= delta_seconds;
        while (game->repeat_remaining <= 1e-9) {
            (void)try_move(game, game->horizontal_direction, 0);
            game->repeat_remaining += ARR_INTERVAL_SECONDS;
        }
    }
    game->gravity_elapsed += delta_seconds;
    const double fall_interval = game->down_held ? SOFT_DROP_INTERVAL_SECONDS : GRAVITY_INTERVAL_SECONDS;
    while (game->gravity_elapsed + 1e-9 >= fall_interval) {
        game->gravity_elapsed -= fall_interval;
        if (game->gravity_elapsed < 0.0) {
            game->gravity_elapsed = 0.0;
        }
        if (!try_move(game, 0, 1)) {
            /* Do not accumulate gravity while supported. */
            game->gravity_elapsed = 0.0;
            break;
        }
    }
    if (collision_at(&game->board, &game->current_piece,
                     game->current_piece.x, game->current_piece.y + 1)) {
        if (was_grounded) game->lock_elapsed += delta_seconds;
        if (game->lock_elapsed + 1e-9 >= LOCK_DELAY_SECONDS) {
            lock_and_spawn(game);
        }
    } else {
        game->lock_elapsed = 0.0;
    }
}

void game_update(Game *game, double delta_seconds)
{
    if (!game->running || !isfinite(delta_seconds) || delta_seconds <= 0.0) return;
    /* Keep landing, repeat and lock events ordered even for larger caller steps. */
    while (delta_seconds > 1e-9 && game->state == STATE_PLAYING) {
        const double step = fmin(delta_seconds, 1.0 / 60.0);
        update_step(game, step);
        delta_seconds -= step;
    }
}

void game_set_horizontal(Game *game, int direction, bool pressed)
{
    if (!game->running || game->state != STATE_PLAYING ||
        (direction != -1 && direction != 1)) {
        return;
    }
    bool *held = direction == -1 ? &game->left_held : &game->right_held;
    if (*held == pressed) {
        return;
    }
    *held = pressed;
    if (pressed) {
        game->horizontal_direction = direction;
    } else if (game->horizontal_direction == direction) {
        game->horizontal_direction = game->left_held ? -1 : (game->right_held ? 1 : 0);
    } else {
        return;
    }
    game->repeat_remaining = DAS_DELAY_SECONDS;
    if (game->horizontal_direction != 0) {
        (void)try_move(game, game->horizontal_direction, 0);
    }
}

void game_release_input(Game *game)
{
    game_set_soft_drop(game, false);
    game->left_held = false;
    game->right_held = false;
    game->horizontal_direction = 0;
    game->repeat_remaining = 0.0;
}

void game_set_soft_drop(Game *game, bool pressed)
{
    if (!game->running || (pressed && game->state != STATE_PLAYING) ||
        game->down_held == pressed) return;
    game->down_held = pressed;
    game->gravity_elapsed = 0.0;
    if (pressed) (void)try_move(game, 0, 1);
}

bool game_rotate(Game *game, int direction)
{
    if (!game->running || game->state != STATE_PLAYING) return false;
    const bool grounded = collision_at(&game->board, &game->current_piece,
                                       game->current_piece.x, game->current_piece.y + 1);
    if (!rotation_try(&game->board, &game->current_piece, direction)) return false;
    if (grounded && game->lock_resets < LOCK_RESET_LIMIT) {
        game->lock_elapsed = 0.0;
        ++game->lock_resets;
    }
    return true;
}

void game_request_quit(Game *game)
{
    game->running = false;
}
