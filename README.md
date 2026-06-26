# Battleship — C++ / SFML

A 2-player naval combat game built with **C++20** and **SFML 3.0.2**.

## Build

Requires [MSYS2](https://www.msys2.org/) with MinGW64 and SFML 3:

```bash
pacman -S mingw-w64-x86_64-sfml mingw-w64-x86_64-cmake mingw-w64-x86_64-ninja
```

Then build:

```bat
cmake -S . -B build -G Ninja ^
  -DCMAKE_CXX_COMPILER=C:/msys64/mingw64/bin/g++.exe ^
  -DCMAKE_PREFIX_PATH=C:/msys64/mingw64
cmake --build build
```

Or just run `build.bat` for a direct g++ compile.

## Run

```bat
build\battleship.exe
```

## How to Play

| Phase | Controls |
|---|---|
| Home | Click **START GAME** |
| Setup | Click grid to place ship · **H** / **V** keys or buttons to rotate |
| Handoff | Click **CONTINUE** to pass device to Player 2 |
| Battle | Click enemy grid to fire · HIT = keep turn · MISS = switch |
| Win | Click **PLAY AGAIN** |

## Project Structure

```
src/
├── Constants.hpp   — colors, grid size, ship definitions
├── Board.hpp       — header-only 10×10 grid logic
└── main.cpp        — game state, rendering, events, main loop
CMakeLists.txt
build.bat           — direct g++ fallback build script
```
