# Chess Engine GUI

This project now contains two executables:

- `chess_engine` — the original console/UCI engine.
- `chess_gui` — an SFML 3 graphical chess interface using the same engine core.

## Build on Windows with MSYS2 MinGW

From the `ChessEngine` directory:

```powershell
Remove-Item -Recurse -Force build -ErrorAction SilentlyContinue
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build -j 4
```

The first configure/build downloads SFML 3.1.0 with CMake FetchContent and builds it with your toolchain.

Run:

```powershell
.\build\bin\chess_gui.exe
```

## GUI controls

- Left click: select and move pieces.
- `N`: new game.
- `U`: undo.
- `Esc`: clear selection.
- Engine depth buttons: 4 / 6 / 8.

The GUI keeps the chess rules inside the engine. It asks the engine for legal moves, applies moves through `ChessEngine`, and runs the search on a copied board in a worker thread so the UI remains responsive.

Promotion currently chooses a queen automatically.
