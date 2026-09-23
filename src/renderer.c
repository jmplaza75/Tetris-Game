#include "renderer.h"

static const SDL_Color BACKGROUND_COLOR = {18, 23, 34, 255};
static const SDL_Color GRID_COLOR = {40, 49, 65, 255};
static const SDL_Color BOARD_COLOR = {11, 15, 24, 255};
static const SDL_Color LOCKED_COLOR = {134, 148, 166, 255};
static const SDL_Color PIECE_COLORS[PIECE_COUNT] = {
    [PIECE_I] = {51, 211, 230, 255},
    [PIECE_J] = {72, 113, 237, 255},
    [PIECE_L] = {245, 157, 55, 255},
    [PIECE_O] = {245, 210, 65, 255},
    [PIECE_S] = {87, 204, 125, 255},
    [PIECE_T] = {179, 109, 235, 255},
    [PIECE_Z] = {235, 88, 107, 255}
};

enum {
    CELL_SIZE = 30,
    CELL_INSET = 2,
    BOARD_WIDTH = BOARD_COLUMNS * CELL_SIZE,
    BOARD_HEIGHT = BOARD_VISIBLE_ROWS * CELL_SIZE,
    BOARD_LEFT = (WINDOW_WIDTH - BOARD_WIDTH) / 2,
    BOARD_TOP = (WINDOW_HEIGHT - BOARD_HEIGHT) / 2
};

static bool set_color(Renderer *renderer, SDL_Color color)
{
    return SDL_SetRenderDrawColor(renderer->handle, color.r, color.g,
                                 color.b, color.a) == 0;
}

static bool draw_cell(Renderer *renderer, int x, int y, SDL_Color color)
{
    if (x < 0 || x >= BOARD_COLUMNS || y < BOARD_HIDDEN_ROWS || y >= BOARD_ROWS) {
        return true;
    }
    const SDL_Rect cell = {
        BOARD_LEFT + x * CELL_SIZE + CELL_INSET,
        BOARD_TOP + (y - BOARD_HIDDEN_ROWS) * CELL_SIZE + CELL_INSET,
        CELL_SIZE - 2 * CELL_INSET, CELL_SIZE - 2 * CELL_INSET
    };
    return set_color(renderer, color) && SDL_RenderFillRect(renderer->handle, &cell) == 0;
}

static bool draw_board(Renderer *renderer, const Board *board)
{
    const SDL_Rect area = {BOARD_LEFT, BOARD_TOP, BOARD_WIDTH, BOARD_HEIGHT};
    if (!set_color(renderer, BOARD_COLOR) ||
        SDL_RenderFillRect(renderer->handle, &area) != 0 ||
        !set_color(renderer, GRID_COLOR)) {
        return false;
    }
    for (int x = 0; x <= BOARD_COLUMNS; ++x) {
        const int screen_x = BOARD_LEFT + x * CELL_SIZE;
        if (SDL_RenderDrawLine(renderer->handle, screen_x, BOARD_TOP,
                               screen_x, BOARD_TOP + BOARD_HEIGHT) != 0) {
            return false;
        }
    }
    for (int y = 0; y <= BOARD_VISIBLE_ROWS; ++y) {
        const int screen_y = BOARD_TOP + y * CELL_SIZE;
        if (SDL_RenderDrawLine(renderer->handle, BOARD_LEFT, screen_y,
                               BOARD_LEFT + BOARD_WIDTH, screen_y) != 0) {
            return false;
        }
    }
    for (int y = BOARD_HIDDEN_ROWS; y < BOARD_ROWS; ++y) {
        for (int x = 0; x < BOARD_COLUMNS; ++x) {
            if (board_is_occupied(board, x, y) &&
                !draw_cell(renderer, x, y, LOCKED_COLOR)) {
                return false;
            }
        }
    }
    return true;
}

static bool draw_piece(Renderer *renderer, const Piece *piece)
{
    for (int y = 0; y < PIECE_SIZE; ++y) {
        for (int x = 0; x < PIECE_SIZE; ++x) {
            if (piece_has_cell(piece, x, y) &&
                !draw_cell(renderer, piece->x + x, piece->y + y,
                           PIECE_COLORS[piece->type])) {
                return false;
            }
        }
    }
    return true;
}

bool renderer_init(Renderer *renderer)
{
    *renderer = (Renderer){0};
    renderer->window = SDL_CreateWindow(
        "Tetris — C / Apple Silicon", SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if (renderer->window == NULL) {
        return false;
    }

    renderer->handle = SDL_CreateRenderer(
        renderer->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer->handle == NULL) {
        /* Also permits startup with SDL's headless dummy video driver. */
        renderer->handle = SDL_CreateRenderer(renderer->window, -1,
                                               SDL_RENDERER_SOFTWARE);
    }
    if (renderer->handle == NULL) {
        return false;
    }

    return SDL_RenderSetLogicalSize(renderer->handle, WINDOW_WIDTH,
                                   WINDOW_HEIGHT) == 0;
}

bool renderer_draw(Renderer *renderer, const Game *game)
{
    if (SDL_SetRenderDrawColor(renderer->handle, BACKGROUND_COLOR.r,
                               BACKGROUND_COLOR.g, BACKGROUND_COLOR.b,
                               BACKGROUND_COLOR.a) != 0 ||
        SDL_RenderClear(renderer->handle) != 0) {
        return false;
    }
    if (!draw_board(renderer, &game->board) ||
        !draw_piece(renderer, &game->current_piece)) {
        return false;
    }
    SDL_RenderPresent(renderer->handle);
    return true;
}

void renderer_destroy(Renderer *renderer)
{
    if (renderer->handle != NULL) {
        SDL_DestroyRenderer(renderer->handle);
    }
    if (renderer->window != NULL) {
        SDL_DestroyWindow(renderer->window);
    }
    *renderer = (Renderer){0};
}
