# mini scratch

we built this for our advanced programming course at Sharif. it's a block-based visual programming environment — think Scratch, but written from scratch in C++ and SDL2.

you can drag blocks onto a canvas, snap them together, hit the green flag, and watch your sprite move around the stage. there's pen drawing, speech bubbles, custom backdrops, variable monitors, and a step-through debugger for when things inevitably break.

---

## group 62 — Sharif University of Technology

| name | student ID | what they built |
|------|-----------|-----------------|
| Davoud Samie Darian | 40410206 4 | everything you see — rendering, sprites, drag-and-drop, UI |
| Hamed Arabpour Dahouei | 40417051 2 | everything that runs — block interpreter, loops, conditions |
| Ali Dehghan Nayeri | 40410186 2 | everything that persists — save/load, undo/redo, dialogs |

---

## build

```bash
# dependencies: SDL2, SDL2_image, SDL2_ttf, SDL2_mixer, SDL2_gfx

g++ src/main.cpp tinyfiledialogs.c -o scratch_editor \
    -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -lSDL2_gfx \
    -lcomdlg32 -lole32 -lm -std=c++11
```
