#include "ui.h"
#include "text.h"

enum {
    TITLE_Y = 20, TITLE_SCALE = 5,
    HELP_X = 30, HELP_Y = BOARD_TOP + BOARD_HEIGHT + 18, HELP_GAP = 19,
    OVERLAY_HEIGHT = 150, OVERLAY_INSET = 12,
    OVERLAY_Y = BOARD_TOP + (BOARD_HEIGHT - OVERLAY_HEIGHT) / 2
};

static bool centered(Renderer *renderer, const char *text, int y, int scale)
{
    const int width = ((int)SDL_strlen(text) * 4 - 1) * scale;
    return text_draw(renderer, (WINDOW_WIDTH - width) / 2, y, text, false, scale);
}

bool ui_draw(Renderer *renderer, const Game *game)
{
    if (!centered(renderer, "TETRIS", TITLE_Y, TITLE_SCALE) ||
        !text_draw(renderer, HELP_X, HELP_Y,
                   "LEFT/RIGHT MOVE   DOWN SOFT DROP   SPACE HARD DROP", false, 2) ||
        !text_draw(renderer, HELP_X, HELP_Y + HELP_GAP,
                   "UP/X/Z ROTATE   C HOLD   P PAUSE   R RESTART   ESC EXIT", false, 2)) return false;
    if (game->state == STATE_PLAYING) return true;
    const SDL_Rect panel = {BOARD_LEFT + OVERLAY_INSET, OVERLAY_Y,
                            BOARD_WIDTH - 2 * OVERLAY_INSET, OVERLAY_HEIGHT};
    if (SDL_SetRenderDrawColor(renderer->handle, 18, 23, 34, 255) != 0 ||
        SDL_RenderFillRect(renderer->handle, &panel) != 0 ||
        SDL_SetRenderDrawColor(renderer->handle, 134, 148, 166, 255) != 0 ||
        SDL_RenderDrawRect(renderer->handle, &panel) != 0) return false;
    const bool paused = game->state == STATE_PAUSED;
    return centered(renderer, paused ? "PAUSED" : "GAME OVER", OVERLAY_Y + 24, 4) &&
           centered(renderer, paused ? "P RESUME" : "R RESTART", OVERLAY_Y + 66, 3) &&
           centered(renderer, paused ? "R RESTART   ESC EXIT" : "ESC EXIT", OVERLAY_Y + 109, 2);
}
