# Gameplay demo

`gameplay.gif` is a 16-second, 800 × 720 animation captured at 15 FPS from the
production C engine and SDL software renderer. The simulation advances at
60 Hz. A fixed seed and scripted controls make the capture reproducible;
no board cells, scores, or piece queues are staged. This is a gameplay demo,
not a recording of a human play session or a performance benchmark.

The sequence shows movement, both rotation directions, HOLD, soft/hard drops,
the ghost preview, and a two-second pause before play resumes.

## Recreate the GIF

Requires the usual C compiler and SDL2 development files, plus Python 3 with
Pillow for GIF encoding. Run from the repository root:

```sh
mkdir -p build/demo/frames
cc -std=c17 -Wall -Wextra -Wpedantic -Werror -Iinclude \
  $(/opt/homebrew/bin/sdl2-config --cflags) \
  tools/record_demo.c src/game.c src/board.c src/piece.c src/collision.c \
  src/rotation.c src/randomizer.c src/scoring.c src/renderer.c \
  src/preview.c src/text.c src/ui.c \
  $(/opt/homebrew/bin/sdl2-config --libs) -lm -o build/demo/record_demo
SDL_VIDEODRIVER=dummy build/demo/record_demo build/demo/frames
python3 tools/encode_demo.py build/demo/frames assets/demo/gameplay.gif
```

Replace the absolute `sdl2-config` path if SDL2 is installed elsewhere.
Captured BMP frames and the capture executable stay under the ignored `build/`
directory. Only the final GIF is stored in the repository.
