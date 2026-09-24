#include "preview.h"
#include "text.h"
#include <inttypes.h>
#include <stdio.h>

enum {
    PANEL_WIDTH = 140, PANEL_GAP = 35, PREVIEW_CELL = 24,
    LEFT_X = BOARD_LEFT - PANEL_GAP - PANEL_WIDTH,
    RIGHT_X = BOARD_LEFT + BOARD_WIDTH + PANEL_GAP,
    LABEL_Y = 65, FIRST_Y = 100, SLOT_HEIGHT = 95, LABEL_SCALE = 3
};

static bool slot(Renderer *renderer, PieceType type, int x, int y, bool muted,
                 const SDL_Color colors[PIECE_COUNT])
{
    const SDL_Rect panel = {x, y, PANEL_WIDTH, SLOT_HEIGHT - 8};
    if (SDL_SetRenderDrawColor(renderer->handle, 40, 49, 65, 255) != 0 ||
        SDL_RenderDrawRect(renderer->handle, &panel) != 0) return false;
    if (type == PIECE_COUNT) return true;
    Piece piece;
    if (!piece_spawn(&piece, type)) return false;
    int min_x = 4, min_y = 4, max_x = 0, max_y = 0;
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            if (!piece_has_cell(&piece, col, row)) continue;
            if (col < min_x) min_x = col;
            if (col > max_x) max_x = col;
            if (row < min_y) min_y = row;
            if (row > max_y) max_y = row;
        }
    }
    SDL_Color color = colors[type];
    if (muted) { color.r /= 2; color.g /= 2; color.b /= 2; }
    if (SDL_SetRenderDrawColor(renderer->handle, color.r, color.g, color.b, 255) != 0) return false;
    const int origin_x = x + (PANEL_WIDTH - (max_x - min_x + 1) * PREVIEW_CELL) / 2;
    const int origin_y = y + (panel.h - (max_y - min_y + 1) * PREVIEW_CELL) / 2;
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            if (!piece_has_cell(&piece, col, row)) continue;
            const SDL_Rect cell = {origin_x + (col - min_x) * PREVIEW_CELL + 2,
                                   origin_y + (row - min_y) * PREVIEW_CELL + 2,
                                   PREVIEW_CELL - 4, PREVIEW_CELL - 4};
            if (SDL_RenderFillRect(renderer->handle, &cell) != 0) return false;
        }
    }
    return true;
}

static bool stats(Renderer *renderer, const Game *game)
{
    enum { STATS_Y = FIRST_Y + SLOT_HEIGHT + 40, STAT_GAP = 85, VALUE_GAP = 25 };
    const char *names[] = {"SCORE", "LINES", "LEVEL"};
    const uint64_t values[] = {game->score, game->lines_cleared, game->level};
    for (int i = 0; i < 3; ++i) {
        char value[21];
        (void)snprintf(value, sizeof value, "%" PRIu64, values[i]);
        const int scale = SDL_strlen(value) > 11 ? 1 : LABEL_SCALE;
        if (!text_draw(renderer, LEFT_X, STATS_Y + i * STAT_GAP, names[i], false, LABEL_SCALE) ||
            !text_draw(renderer, LEFT_X, STATS_Y + i * STAT_GAP + VALUE_GAP, value, false, scale)) return false;
    }
    return true;
}

bool preview_draw(Renderer *renderer, const Game *game, const SDL_Color colors[PIECE_COUNT])
{
    if (!stats(renderer, game) ||
        !text_draw(renderer, LEFT_X, LABEL_Y, "HOLD  C", game->hold_used, LABEL_SCALE) ||
        !text_draw(renderer, RIGHT_X, LABEL_Y, "NEXT", false, LABEL_SCALE) ||
        !slot(renderer, game->held_piece, LEFT_X, FIRST_Y, game->hold_used, colors)) return false;
    for (int i = 0; i < NEXT_PIECE_COUNT; ++i) {
        if (!slot(renderer, game->next[i], RIGHT_X, FIRST_Y + i * SLOT_HEIGHT, false, colors)) return false;
    }
    return true;
}
