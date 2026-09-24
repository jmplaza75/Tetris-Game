"""Encode record_demo.c frames as a looping GIF (requires Pillow)."""
import sys
from pathlib import Path
from PIL import Image

if len(sys.argv) != 3:
    raise SystemExit("Usage: python3 tools/encode_demo.py FRAME_DIRECTORY OUTPUT.gif")
paths = sorted(Path(sys.argv[1]).glob("frame-*.bmp"))
if not paths:
    raise SystemExit("No captured frames found")
frames = []
for path in paths:
    with Image.open(path) as source:
        frames.append(source.convert("RGB").quantize(colors=64))
# GIF duration uses 10 ms units; 70/60/70 ms averages to 15 FPS.
durations = [[70, 60, 70][i % 3] for i in range(len(frames))]
frames[0].save(sys.argv[2], save_all=True, append_images=frames[1:],
               duration=durations, loop=0, optimize=True, disposal=1)
print(f"Saved {len(frames)} frames to {sys.argv[2]}")
