# Battleship

A local 2-player naval combat game built with C++20 and SFML 3.

---

## Requirements

- [MSYS2](https://www.msys2.org/) with MinGW64
- SFML 3 (install via MSYS2):

```
pacman -S mingw-w64-x86_64-sfml
```

---

## Build

Run `build.bat` from the project root:

```
build.bat
```

This compiles `src/main.cpp` with g++ and places the executable in `build/`.

---

## Run

```
build\battleship.exe
```

---

## How to Play

**Setup phase**

Each player places 5 ships on their grid. Ships can be placed horizontally or vertically.

- Press **H** to place horizontally
- Press **V** to place vertically
- Click a cell on the grid to confirm placement
- Green highlight = valid position, red = invalid
- After Player 1 is done, pass the device to Player 2

**Battle phase**

Players take turns firing at the enemy grid.

- Click any cell on the enemy board to fire
- HIT: your turn continues
- MISS: the other player takes over

The first player to sink all 5 enemy ships wins.

---

## Ships

| Ship       | Size |
|------------|------|
| Carrier    | 5    |
| Battleship | 4    |
| Cruiser    | 3    |
| Submarine  | 3    |
| Destroyer  | 2    |

---

## Project Structure

```
src/
    Constants.hpp   grid size, colors, ship definitions
    Board.hpp       10x10 grid logic (header-only)
    main.cpp        game state, rendering, input, main loop
build.bat           build script (calls g++ directly)
```
