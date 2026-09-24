# Phase 9 Testing

Run `make test` from the repository root to check the engine, SDL events,
software renderer, and three application frames using dummy video.
Tests use `assert` and fail with a nonzero exit code. The Makefile applies
`-UNDEBUG` to test executables so checks remain active even when `-DNDEBUG`
is supplied. No assets or manual interaction are required.

## Commands

```sh
make test
make test-engine
make test-sdl
make MODE=release test
make sanitize-engine
make sanitize
```

`test-engine` compiles and links only the C engine; it does not query
sdl2-config. `test-sdl` requires SDL2. The renderer and application use dummy
video to run without a visible window. Build outputs are separated by
configuration.

## Functional coverage

- `test_engine.c`: initialization, empty board, all seven piece shapes,
  spawning, and invalid types; runs the engine suites.
- `test_motion.c`: boundaries, obstacles, hidden rows, extreme coordinates,
  timestep-independent gravity, invalid time intervals, and DAS/ARR.
- `test_lines.c`: 1–4-line clears, nonadjacent rows, a full board, color
  preservation, lock delay, blocked spawning, and atomic locking.
- `test_rotation.c`: four turns in both directions, the O piece, T and I kicks,
  walls and floor, rejection without mutation, and soft drop.
- `test_randomizer.c`: 3,200 bags without duplicates, reproducible seeds,
  NEXT continuity, hold, and blocked spawning during a swap.
- `test_scoring.c`: ghost without mutation, hard/soft drop, line scoring,
  level transitions, gravity intervals, and rejection after game over or exit.
- `test_states.c`: pause, frozen timers, resuming lock delay, complete restart
  from every state, and irreversible exit requests.
- `test_properties.c`: 1,024 deterministic boards compared with a matrix model
  that removes rows one at a time; 12,180 positions covering all pieces, four
  turns, boundaries, hidden rows, and obstacles. Checks occupancy, colors, and
  absence of partial writes when locking fails. Also exhausts all 15 lock-delay
  resets through actual movement.
- `test_lifecycle.c`: movement, rotation, hold, hard drop, focus, pause,
  restart, and exit events, including ignored automatic key repeats.
- `test_renderer.c`: ghost and active-piece pixels, pause panel, hiding the
  active piece after game over, absence of game-state mutation, 64-bit scores,
  resizing, and repeated destruction.

The models use a fixed sequence to make failures reproducible. These counts
refer to executed cases, not line-coverage percentages or exhaustive checks
of every possible game or SRS table entry.

## Validation results and limitations

On macOS ARM64, the full debug and release suites pass, as does the engine
with AddressSanitizer/UndefinedBehaviorSanitizer. A fresh build without SDL
was verified using
`make BUILD_DIR=build/no-sdl SDL_CONFIG=/nonexistent test-engine`.

The `make sanitize` attempt in this phase stalled during `test_lifecycle`
startup, with macOS service errors (`_LSModifyNotification` and
`com.apple.hiservices-xpcservice`). The process group was terminated after
25 seconds. SDL integration under sanitizers is not considered validated on
this machine.

Pixel checks and dummy-video tests do not replace manual checks of
readability, physical keyboard input, VSync, HiDPI, or the native window.
Manual steps for playing, pausing, restarting, and resizing are in the README.
