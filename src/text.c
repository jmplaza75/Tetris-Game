#include "text.h"

/* Tiny 3x5 label glyphs keep previews independent of font assets/SDL_ttf. */
static uint16_t glyph(char c)
{
    switch (c) {
    case 'A': return 0x2BED;
    case 'B': return 0x6BAE;
    case 'F': return 0x79E4;
    case 'G': return 0x796F;
    case 'M': return 0x5FED;
    case 'P': return 0x7BE4;
    case 'U': return 0x5B6F;
    case 'W': return 0x5B7D;
    case '/': return 0x1248;
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

bool text_draw(Renderer *renderer, int x, int y, const char *text, bool muted, int scale)
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

