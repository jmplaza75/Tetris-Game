# Phase 8 Architecture

`main.c` initializes SDL, creates the renderer, and manages the application
lifecycle. On failure, it returns a nonzero exit code and reports the SDL error.
Both normal and error exits destroy the renderer first, then the window, and
finally shut down the SDL subsystems.

```text
main → input_process → game_update (fixed step) → renderer_draw
            ↓                ↓                        ↓
          Game         SDL-independent C           SDL2 window
```

`Game` contains the running flag, board, and active piece. Its update function
applies horizontal key repeat and gravity using timers measured in seconds;
`game_init` resets them all. The active piece remains separate from the locked
cells on the board.

`Board` stores 24 rows as `uint16_t` values: the lower ten bits represent columns
from left to right. Rows 0–3 are hidden; the renderer displays rows 4–23.
`board_is_occupied` returns false outside the board; it is not a collision
function and does not authorize movement.

`Piece` stores its type, position, orientation, and a 16-bit mask. Bit
`y * 4 + x` represents local cell `(x, y)`, with the origin at the top left.
All seven shapes and four orientations are supported. `rotation_try` rotates
within a 3 × 3 box for J/L/S/T/Z and a 4 × 4 box for I, using SRS centers.
O keeps its position and shape. Five candidate positions are tested, with a
separate kick table for I and a shared table for the other rotating pieces;
failure preserves the original state. Reference:
[SRS](https://tetris.wiki/Super_Rotation_System). `piece_spawn` validates the type
and resets the piece without changing the board or checking collisions.
The initial position is `(3, 4)` so the entire shape is visible immediately.
Future spawning rules could use the hidden rows.

`collision_at` checks only the piece's four occupied cells, not its entire
4 × 4 box. Each cell must lie within the 10 × 24 board and must not overlap
an occupied bit. The bounding box may therefore extend outside the board when
its out-of-bounds cells are empty. The query changes neither the board nor the
piece; movement is applied only when the query allows it.

Gravity starts at one second per row and preserves leftover elapsed time.
When it encounters an obstacle, it discards pending gravity time. Nonpositive
or nonfinite update intervals are ignored; the main loop supplies 1/60-second
steps. Larger intervals are subdivided to keep falling, contact, and locking
in order without carrying the previous piece's accumulated gravity into its
replacement.

A grounded piece accumulates a 500 ms lock delay. A successful horizontal move
or rotation resets that delay up to 15 times per piece; a rejected action does
not reset it. Losing contact clears the contact timer. `board_lock_piece`
validates all cells before writing, so failure never partially changes the
board. A byte matrix also stores each cell's piece type plus one to preserve
its color; occupancy continues to use bitmasks.

`board_clear_lines` scans all 24 rows from bottom to top, skips full masks,
and copies surviving rows downward with their colors. It clears the remaining
top rows and returns the number removed, which `Game` adds to `lines_cleared`.
This includes hidden rows and supports nonadjacent full rows.

The first piece in NEXT is then activated. Gravity, lock delay, and DAS reset,
and the spawn position is checked. If it is occupied, `STATE_GAME_OVER`
freezes gameplay, releases held inputs, and hides the piece that could not
spawn. Exit events and rendering remain active; the window title and a
GAME OVER panel indicate the failure and offer R to restart or Esc to exit.

Each horizontal key press moves immediately, then waits 150 ms (DAS) before
repeating every 40 ms (ARR). The last direction pressed takes priority;
releasing it reactivates the other held direction. macOS repeat events are
ignored. Losing focus releases input state without pausing gravity. Timing
constants are configured in `game.h`.

The renderer centers a 300 × 600 grid with 30-pixel cells, omits hidden rows,
and clips cells outside the visible area. All seven piece colors are defined
in one table. Locked cells use the type stored in the board; gray is reserved
for occupied cells without color metadata, such as tests that set bits directly.

`input.c` drains the event queue each frame and requests exit on Esc or
SDL_QUIT. `renderer.c` owns the window and renderer, uses a centralized
background color, and draws in an 800 × 720 logical space. SDL adapts this
space to the window size and high-density displays.

The high-resolution clock feeds an accumulator measured in seconds. Each
update consumes 1/60 second independently of rendering frequency. Elapsed
time added per frame is capped at 0.25 seconds to avoid lengthy catch-up after
process suspension. OS keyboard repeat is not used.

Accelerated rendering with VSync is requested, with a software fallback when
unavailable. A delay at the end of each frame limits resource use when VSync
does not wait or the display exceeds 60 Hz. Gameplay does not depend on that
delay.

The application performs no dynamic allocations of its own inside the game
loop. SDL resources are managed explicitly. `--smoke-test` runs three frames
and exits through the same initialization, rendering, and cleanup path as a
normal run.

Pressing Down immediately moves one row and enables a 1/30-second drop
interval. Pressing or releasing it resets the gravity accumulator to avoid
jumps caused by pending time. Releasing it or losing focus cancels soft drop.
Up/X and Z perform one rotation per press, ignoring OS repeat events. Soft
drop respects collisions and lock delay and awards one point per row traveled.

## Randomizer and queue

Each game owns a `Randomizer` that generates bags containing I/J/L/O/S/T/Z.
Fisher–Yates shuffles them using SplitMix64 and bounded rejection sampling to
avoid modulo bias. Neither `rand()` nor global state is used. The queue always
contains five types: removing one shifts the others and refills the final
slot from the bag, without losing pieces at bag boundaries.
`game_init_seed` enables reproducible tests; `game_init` uses seed 1, while the
application supplies SDL's high-resolution counter.

## Hold and previews

`held_piece == PIECE_COUNT` indicates an empty hold slot. The first C stores
the active piece and consumes NEXT; subsequent swaps do not consume the queue.
`hold_used` prevents another hold until the next lock. The incoming piece
returns to its initial position and orientation; gravity, lock delay, the
lock-reset counter, and DAS reset. An occupied spawn uses the same game-over
path.

`preview.c` draws HOLD and the five upcoming pieces with the shared palette,
centering their occupied cells. HOLD is dimmed when unavailable. Labels use
small built-in glyphs without SDL_ttf or font files. The layout shares board
measurements and scales with SDL's logical space. No dynamic memory is
allocated during gameplay.

## Ghost, hard drop, and scoring

`game_ghost_piece` copies the active piece and finds its last valid position
through collision queries. It does not change the board, timers, queue, or
randomizer. The renderer draws its outline before the active piece, which
covers it when their positions coincide. It is not shown after game over.

Space calls `game_hard_drop` once per press, ignoring key repeat. It uses the
same query as the ghost, awards two points per row descended, and locks
immediately, even when the distance is zero. It reuses the line-clear, NEXT
advance, hold-reset, and spawn-check path.

`scoring.c` centralizes the rules without depending on SDL. `Game.score` is
64-bit. Single/double/triple/Tetris clears award 100/300/500/800 points,
multiplied by the level before the clear. Lines are then accumulated and
`level = 1 + lines_cleared / 10` is calculated. Advanced bonuses are not
implemented in this phase. Gravity uses `max(1/60, 0.8^(level-1))` seconds per
row; calculation stops when the minimum is reached. Soft drop uses the smaller
of that interval and 1/30 second. Only rows traveled while soft drop is active
award one point; normal gravity and blocked attempts do not score.

`preview.c` reuses the glyphs from `text.c` to display SCORE, LINES, and LEVEL,
reducing the numeric scale when needed to fit. Tests verify landing positions
for all pieces and orientations over obstacles, absence of mutation, instant
locking, 1–4-line scoring, level transitions, drops, gravity, and Space events.

## Pause, restart, and UI

The state machine contains PLAYING, PAUSED, and GAME_OVER. P toggles between
PLAYING and PAUSED; it does not change GAME_OVER. Only PLAYING permits
movement, rotation, hold, drops, and gameplay updates. PAUSED preserves gravity,
lock delay, and DAS without consuming time, while SDL continues processing
events and rendering. Pausing releases held keys without resetting those
timers, even if focus is lost; movement keys must be pressed again after
resuming. Losing focus alone still does not pause the game.

`game_restart(game, seed)` delegates to full initialization: it resets the
board and colors, active piece, bag, NEXT, HOLD, score, lines, level, input,
and all timers. R uses a new seed from the SDL clock; tests use explicit seeds.
Restart works from all three states but never reverses an exit request.
The window and renderer remain alive; the process does not restart and no
additional SDL resources are created.

`text.c` contains the glyphs and shared text drawing code. `preview.c`
remains responsible for HOLD, NEXT, and statistics. `ui.c` draws the title,
control hints, and pause/game-over panels at the end of each frame. The active
piece remains visible while paused; an invalid spawn is hidden after game
over. Positions derive from the logical space and board dimensions, allowing
SDL to scale the interface with the window.

Tests cover blocked actions while paused, timer preservation, resuming a
partially elapsed lock delay, occupied spawns, restart from every state, and
P/R events without automatic repeat. Software-rendered images of gameplay,
pause, and game over were also reviewed.
