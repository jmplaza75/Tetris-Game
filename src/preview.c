#include "preview.h"
#include <inttypes.h>
#include <stdio.h>

enum {
    PANEL_WIDTH = 140, PANEL_GAP = 35, PREVIEW_CELL = 24,
    LEFT_X = BOARD_LEFT - PANEL_GAP - PANEL_WIDTH,
    RIGHT_X = BOARD_LEFT + BOARD_WIDTH + PANEL_GAP,
    LABEL_Y = 65, FIRST_Y = 100, SLOT_HEIGHT = 95, LABEL_SCALE = 3
};

/* Tiny 3x5 label glyphs keep previews independent of font assets/SDL_ttf. */
static uint16_t glyph(char c)
{
    switch (c) {
    case '0': return 0x7B6F;
    case '1': return 0x2C97;
    case '2': return 0x73E7;
    case '3': return 0x73CF;
    case '4': return 0x5BC9;
    case '5': return 0x79CF;
    case '6': return 0x79EF;
    case '7': return 0x7249;
    case '8': return 0x7BEF;
    case '9': return 0x7BCF;
    case 'S': return 0x79CF;
    case 'R': return 0x6BED;
    case 'I': return 0x7497;
    case 'V': return 0x5B6A;
    case 'H': return 0x5BED;
    case 'O': return 0x7B6F;
    case 'L': return 0x4927;
    case 'D': return 0x6B6E;
    case 'N': return 0x5FED;
    case 'E': return 0x79E7;
    case 'X': return 0x5AAD;
    case 'T': return 0x7492;
    case 'C': return 0x7927;
    default: return 0;
    }
}

static bool label(Renderer *renderer, int x, int y, const char *text, bool muted, int scale)
{
    const Uint8 value = muted ? 105 : 205;
    if (SDL_SetRenderDrawColor(renderer->handle, value, value, value, 255) != 0) return false;
    for (; *text; ++text, x += 4 * scale) {
        const uint16_t bits = glyph(*text);
        for (int row = 0; row < 5; ++row) {
            for (int col = 0; col < 3; ++col) {
                if (!(bits & (1U << (14 - row * 3 - col)))) continue;
                const SDL_Rect pixel = {x + col * scale, y + row * scale, scale, scale};
                if (SDL_RenderFillRect(renderer->handle, &pixel) != 0) return false;
            }
        }
    }
    return true;
}

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
        if (!label(renderer, LEFT_X, STATS_Y + i * STAT_GAP, names[i], false, LABEL_SCALE) ||
            !label(renderer, LEFT_X, STATS_Y + i * STAT_GAP + VALUE_GAP, value, false, scale)) return false;
    }
    return true;
}

bool preview_draw(Renderer *renderer, const Game *game, const SDL_Color colors[PIECE_COUNT])
{
    if (!stats(renderer, game) ||
        !label(renderer, LEFT_X, LABEL_Y, "HOLD  C", game->hold_used, LABEL_SCALE) ||
        !label(renderer, RIGHT_X, LABEL_Y, "NEXT", false, LABEL_SCALE) ||
        !slot(renderer, game->held_piece, LEFT_X, FIRST_Y, game->hold_used, colors)) return false;
    for (int i = 0; i < NEXT_PIECE_COUNT; ++i) {
        if (!slot(renderer, game->next[i], RIGHT_X, FIRST_Y + i * SLOT_HEIGHT, false, colors)) return false;
    }
    return true;
}
