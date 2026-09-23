# C++ Chess Engine

Complete educational chess engine implementing:

- 8x8 board representation
- Legal move generation
- Check/checkmate/stalemate detection
- Castling
- En passant
- Promotion
- Make/undo move
- Material + piece-square evaluation
- Minimax-style negamax alpha-beta search
- Quiescence search
- Move ordering
- Iterative deepening
- Zobrist hashing
- Transposition table
- FEN load/save
- Perft testing
- Console interface
- Basic UCI protocol

## Build

```bash
cmake -S . -B build
cmake --build build --config Release
```

Run:

```bash
./build/chess_engine
```

Windows:

```powershell
.\build\Release\chess_engine.exe
```

## Perft starting position

Expected:
- depth 1 = 20
- depth 2 = 400
- depth 3 = 8902
- depth 4 = 197281

## UCI

Select `UCI mode` in the console, or run the executable and send:

```text
uci
isready
position startpos moves e2e4 e7e5
go depth 4
quit
```

For GUI integration, the executable should ideally be launched directly in UCI mode; this implementation also keeps the educational console menu.
