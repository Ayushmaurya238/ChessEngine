# ♟️ Chess Engine

A **C++ chess engine built from scratch**, combining a custom chess
rules/move-generation core with a search engine and an interactive
**SFML GUI**.

The project is designed as an engineering-focused chess engine rather
than only a graphical chess game. It progressively implements the major
components found in a traditional chess engine: board representation,
legal move generation, evaluation, minimax/alpha-beta search, quiescence
search, transposition tables, Zobrist hashing, FEN support, and
UCI-oriented engine integration.

------------------------------------------------------------------------

## ✨ Features

### Chess Rules & Game State

-   Standard 8×8 chess board representation
-   All six chess piece types
    -   Pawn
    -   Knight
    -   Bishop
    -   Rook
    -   Queen
    -   King
-   Legal move generation
-   Capture handling
-   Check detection
-   Checkmate detection
-   Stalemate detection
-   Castling
-   En passant
-   Pawn promotion
-   Move history
-   Undo moves
-   New-game/reset support
-   FEN representation/parsing support

### Search Engine

-   Minimax-based search
-   Alpha-beta pruning
-   Quiescence search
-   Search-depth control
-   Move ordering
-   Position evaluation
-   Transposition table
-   Zobrist hashing
-   Node counting
-   Search-time measurement
-   Transposition-table hit statistics

### GUI

The project includes an **SFML-based graphical interface** with:

-   Interactive chess board
-   Piece rendering
-   Mouse-based move selection
-   Legal-move highlighting
-   Capture highlighting
-   Check highlighting
-   Game-over detection
-   Undo
-   New game
-   Engine search running independently from the UI
-   Search depth selection
-   Evaluation display
-   Nodes searched
-   Transposition-table statistics
-   Search-time statistics

The GUI search is performed on a **copied board state in a worker
thread**, keeping the interface responsive while the engine calculates a
move.

------------------------------------------------------------------------

## 🏗️ Architecture

The engine is organized into separate layers so that the chess rules,
search logic, and graphical interface remain independent.

``` text
                         ┌─────────────────────┐
                         │      SFML GUI       │
                         │                     │
                         │ Board rendering     │
                         │ Mouse controls      │
                         │ Game interaction    │
                         └──────────┬──────────┘
                                    │
                                    ▼
                         ┌─────────────────────┐
                         │   Chess Engine API  │
                         └──────────┬──────────┘
                                    │
                 ┌──────────────────┼──────────────────┐
                 │                  │                  │
                 ▼                  ▼                  ▼
        ┌────────────────┐ ┌────────────────┐ ┌────────────────┐
        │     Board      │ │ Move Generator │ │   Evaluation   │
        │                │ │                │ │                │
        │ Position       │ │ Pseudo-legal   │ │ Material /     │
        │ Pieces         │ │ Legal moves    │ │ position score │
        │ Castling      │ │ Special moves  │ │                │
        │ En passant    │ │ Check filtering│ │                │
        └───────┬────────┘ └───────┬────────┘ └────────────────┘
                │                  │
                └────────┬─────────┘
                         ▼
                ┌────────────────────┐
                │   Search Engine    │
                │                    │
                │ Minimax            │
                │ Alpha-Beta         │
                │ Quiescence         │
                │ Move ordering      │
                └─────────┬──────────┘
                          │
                ┌─────────┴──────────┐
                ▼                    ▼
       ┌─────────────────┐   ┌─────────────────┐
       │ Transposition   │   │ Zobrist Hashing │
       │ Table            │   │                 │
       └─────────────────┘   └─────────────────┘
```

------------------------------------------------------------------------

## 🧩 Core Components

### `Board`

The `Board` class represents the current chess position.

Responsibilities include:

-   Maintaining the 64 board squares
-   Storing piece type and color
-   Tracking the side to move
-   Maintaining castling rights
-   Handling en-passant state
-   Applying moves
-   Supporting undo/history operations
-   Providing position information to other engine components

The board is the central state object shared by the move-generation and
search layers.

------------------------------------------------------------------------

### `Move`

A `Move` represents a chess action from one square to another.

A move can contain information required for:

-   Source square
-   Destination square
-   Captures
-   Promotion
-   Castling
-   En passant
-   Other special-move state

Keeping moves as explicit objects makes it possible for the search
engine to generate, evaluate, apply, and undo moves efficiently.

------------------------------------------------------------------------

### `MoveGenerator`

The move generator is responsible for producing chess moves.

It is divided conceptually into:

``` text
Pawn moves
Knight moves
Bishop moves
Rook moves
Queen moves
King moves
Special moves
        ↓
Pseudo-legal moves
        ↓
Make move
        ↓
Check king safety
        ↓
Legal moves
```

The generator therefore separates **pseudo-legal move generation** from
**legal move filtering**.

This is important because a geometrically valid move is not necessarily
legal if it leaves the moving side's king in check.

------------------------------------------------------------------------

### `Search`

The search engine explores possible future positions and chooses a move
according to the evaluation function.

The current search stack includes:

``` text
Position
   │
   ▼
Move generation
   │
   ▼
Move ordering
   │
   ▼
Alpha-Beta search
   │
   ├── Transposition Table lookup
   │
   └── Quiescence search at leaf positions
   │
   ▼
Evaluation
   │
   ▼
Best move
```

### Alpha-Beta Pruning

Instead of evaluating every branch of the game tree, alpha-beta pruning
eliminates branches that cannot influence the final decision.

This substantially reduces the number of positions that need to be
searched compared with plain minimax.

### Quiescence Search

A normal fixed-depth search can stop in tactically unstable positions.

Quiescence search extends selected leaf positions, particularly around
tactical moves such as captures, to reduce the **horizon effect**.

------------------------------------------------------------------------

## ⚡ Transposition Tables

Different sequences of moves can lead to the same chess position.

Without caching, the engine may repeatedly analyze identical positions.

The transposition table stores previously searched positions and their
search information:

``` text
Zobrist Key
     │
     ▼
┌─────────────────────────┐
│ Transposition Table     │
│                         │
│ Position hash           │
│ Search depth            │
│ Score                   │
│ Bound type              │
│ Best move               │
└─────────────────────────┘
```

This allows the search engine to reuse previously computed information
and avoid redundant work.

------------------------------------------------------------------------

## 🔐 Zobrist Hashing

The engine uses **Zobrist hashing** to generate compact position keys.

A chess position can be represented using randomly generated values
associated with:

-   Piece type
-   Piece color
-   Board square
-   Side to move
-   Castling rights
-   En-passant state

The resulting hash is used as the key for the transposition table.

Conceptually:

``` text
Position
   │
   ├── Pieces
   ├── Side to move
   ├── Castling rights
   └── En passant
          │
          ▼
   Zobrist Hash
          │
          ▼
   Transposition Table
```

------------------------------------------------------------------------

## 📋 FEN Support

The engine includes support for **Forsyth-Edwards Notation (FEN)**,
allowing chess positions to be represented as compact strings.

A FEN position contains:

1.  Piece placement
2.  Side to move
3.  Castling availability
4.  En-passant target square
5.  Halfmove clock
6.  Fullmove number

This makes it possible to load and test arbitrary chess positions
instead of always starting from the initial board.

------------------------------------------------------------------------

## 🔌 UCI

The project includes work toward **Universal Chess Interface (UCI)**
support.

UCI provides a standard protocol through which chess engines communicate
with chess GUIs and other chess software.

The intended communication model is:

``` text
GUI / Chess GUI
      │
      │ UCI commands
      ▼
Chess Engine
      │
      │ Best move
      ▼
GUI
```

This also provides a path toward testing the engine independently from
the custom SFML interface.

------------------------------------------------------------------------

## 🎨 Graphical Interface

The GUI is implemented using **SFML 3.1.0**.

It provides a visual environment for interacting with the engine rather
than requiring every move to be entered through a command line.

### Current interaction

-   **Left click** --- select/move pieces
-   **N** --- start a new game
-   **U** --- undo
-   **Esc** --- exit
-   **4 / 6 / 8** --- select engine search depth

The interface also exposes engine statistics such as:

``` text
Evaluation
Search depth
Nodes searched
Transposition-table hits
Search time
```

------------------------------------------------------------------------

## 🧵 Multithreaded Engine Search

Engine calculation can take significantly longer at deeper search
depths.

To prevent the GUI from freezing during engine computation, the search
is performed using a **worker thread** operating on a copied board
state.

``` text
Main GUI Thread
      │
      ├── Render board
      ├── Handle mouse input
      └── Update interface
                │
                │
                ▼
        Worker Search Thread
                │
                ├── Generate moves
                ├── Search tree
                ├── Evaluate positions
                └── Return best move
```

This separation keeps rendering and user interaction responsive while
the engine searches.

------------------------------------------------------------------------

## 🧪 Correctness Testing

A key validation method used by the project is **perft** testing.

Perft counts the number of legal positions reachable from a position at
a given depth.

For the standard initial chess position, the engine has been verified
against the known result:

``` text
Perft depth 4 = 197281
```

This is particularly useful for validating:

-   Pawn movement
-   Captures
-   Sliding pieces
-   Knight movement
-   King movement
-   Castling
-   En passant
-   Promotion
-   Check legality

A correct perft implementation is an important foundation before relying
on deeper engine-search results.

------------------------------------------------------------------------

## 🛠️ Tech Stack

  Component         Technology
  ----------------- ---------------------------
  Language          C++
  Build System      CMake
  GUI               SFML 3.1.0
  Search            Minimax + Alpha-Beta
  Tactical Search   Quiescence Search
  Hashing           Zobrist Hashing
  Cache             Transposition Table
  Position Format   FEN
  Engine Protocol   UCI
  Testing           Perft / engine validation
  Platform          Windows / MinGW tested

------------------------------------------------------------------------

## 📁 Project Structure

The exact source layout may evolve as the engine is developed, but the
project is organized around the following logical modules:

``` text
ChessEngine/
│
├── CMakeLists.txt
├── README.md
│
├── include/
│   └── chess/
│       ├── Board.h
│       ├── Move.h
│       ├── MoveGenerator.h
│       ├── Evaluator.h
│       ├── Search.h
│       ├── ZobristHash.h
│       ├── TranspositionTable.h
│       ├── FEN.h
│       └── UCI.h
│
├── src/
│   ├── Board.cpp
│   ├── Move.cpp
│   ├── MoveGenerator.cpp
│   ├── Evaluator.cpp
│   ├── Search.cpp
│   ├── ZobristHash.cpp
│   ├── TranspositionTable.cpp
│   ├── FEN.cpp
│   └── UCI.cpp
│
├── gui/
│   └── SFML interface and assets
│
├── tests/
│   └── engine / move-generation tests
│
└── assets/
    └── chess pieces and GUI resources
```

> The structure above describes the project's architecture; filenames
> can differ slightly between development snapshots.

------------------------------------------------------------------------

## 🚀 Building the Project

### Requirements

Install:

-   C++ compiler with C++17 or newer support
-   CMake
-   MinGW on Windows if using the tested MinGW generator
-   Internet connection for the first CMake configuration if SFML is
    fetched automatically

The project uses CMake's `FetchContent` mechanism to obtain **SFML
3.1.0**.

### Windows + MinGW

From the project root:

``` powershell
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build -j 4
```

After a successful build, the GUI executable is generated at:

``` text
build/bin/chess_gui.exe
```

Run it with:

``` powershell
.\build\bin\chess_gui.exe
```

### Clean rebuild

If you need to regenerate the build directory:

``` powershell
Remove-Item -Recurse -Force build -ErrorAction SilentlyContinue
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build -j 4
```

------------------------------------------------------------------------

## 🎮 Playing Against the Engine

1.  Build the project.
2.  Launch `chess_gui.exe`.
3.  Select a piece with the mouse.
4.  Available legal destinations are highlighted.
5.  Click a destination to make the move.
6.  The engine searches for its response.
7.  Use the depth controls to change search depth.

### Controls

  Key / Action   Function
  -------------- ---------------------
  Left Click     Select / move piece
  `N`            New game
  `U`            Undo
  `Esc`          Exit
  `4`            Search depth 4
  `6`            Search depth 6
  `8`            Search depth 8

------------------------------------------------------------------------

## 🧠 Design Goals

The project is being developed around several principles:

### 1. Correctness before strength

The engine should first generate **correct legal moves** before search
optimization is added.

### 2. Separation of concerns

Chess rules, search, hashing, and GUI logic should remain independent.

### 3. Measurable performance

The engine exposes metrics such as:

-   Nodes searched
-   Search time
-   Transposition-table hits
-   Evaluation

This makes optimization measurable rather than subjective.

### 4. Incremental development

The engine is built in stages:

``` text
Board Representation
        ↓
Move Generation
        ↓
Legal Move Validation
        ↓
Special Chess Rules
        ↓
Evaluation
        ↓
Minimax
        ↓
Alpha-Beta
        ↓
Move Ordering
        ↓
Quiescence Search
        ↓
Zobrist Hashing
        ↓
Transposition Table
        ↓
FEN / UCI
        ↓
GUI Integration
        ↓
Performance Optimization
```

------------------------------------------------------------------------

## 📈 Development Roadmap

### Completed / Implemented

-   [x] Board representation
-   [x] Piece representation
-   [x] Basic move representation
-   [x] Move generation
-   [x] Legal move filtering
-   [x] Check detection
-   [x] Checkmate / stalemate handling
-   [x] Castling
-   [x] En passant
-   [x] Promotion
-   [x] Move history / undo
-   [x] Evaluation
-   [x] Minimax search
-   [x] Alpha-beta pruning
-   [x] Quiescence search
-   [x] Transposition table
-   [x] Zobrist hashing
-   [x] FEN support
-   [x] SFML GUI
-   [x] GUI legal-move highlighting
-   [x] Engine statistics
-   [x] Worker-thread engine search
-   [x] Perft validation
-   [x] UCI implementation/work

### Future Improvements

-   [ ] Stronger positional evaluation
-   [ ] Piece-square tables
-   [ ] Better move ordering
-   [ ] Iterative deepening
-   [ ] Principal variation tracking
-   [ ] Killer-move heuristic
-   [ ] History heuristic
-   [ ] Null-move pruning
-   [ ] Late-move reductions
-   [ ] More extensive opening support
-   [ ] Endgame-specific evaluation
-   [ ] Bitboard-based representation
-   [ ] Larger automated perft test suite
-   [ ] Engine-vs-engine benchmarking
-   [ ] Elo-style strength testing
-   [ ] Full UCI compatibility and external GUI testing

------------------------------------------------------------------------

## ⚠️ Current Limitations

-   Promotion currently defaults automatically to **Queen** rather than
    presenting a promotion-choice UI.
-   Engine playing strength is still under active development and
    depends heavily on the evaluation/search configuration.
-   Deeper searches can become computationally expensive.
-   The current GUI is primarily intended as a development and
    demonstration interface rather than a complete production chess
    client.
-   Search performance can be improved substantially through additional
    pruning, move-ordering, evaluation, and board-representation
    optimizations.

------------------------------------------------------------------------

## 🔬 Performance & Benchmarking

Engine performance should be evaluated using reproducible measurements
rather than only visual gameplay.

Useful metrics include:

``` text
Nodes searched
Nodes / second
Search depth
Search time
Transposition-table hit rate
Perft node counts
```

A future benchmark suite can compare changes to:

-   Move ordering
-   Evaluation
-   Alpha-beta pruning
-   Quiescence search
-   Hash-table size
-   Board representation
-   Bitboards
-   Additional pruning techniques

------------------------------------------------------------------------

## 📚 Concepts Demonstrated

This project brings together several important computer-science and
systems concepts:

### Data Structures

-   Arrays / board representations
-   Trees
-   Hash tables
-   Transposition tables

### Algorithms

-   Minimax
-   Alpha-beta pruning
-   Depth-limited search
-   Quiescence search
-   Move ordering
-   Hash-based memoization

### Systems / Software Engineering

-   C++ class design
-   Modular architecture
-   CMake
-   Multithreading
-   GUI integration
-   Performance measurement
-   Automated correctness testing

### Chess Programming

-   Legal move generation
-   Position evaluation
-   FEN
-   UCI
-   Zobrist hashing
-   Perft
-   Search optimization

------------------------------------------------------------------------

## 🐛 Debugging Philosophy

Chess engines are particularly sensitive to small state-management bugs.

When debugging the engine, useful checks include:

1.  Validate board state after every make/undo operation.
2.  Compare legal move counts against known positions.
3.  Use perft to isolate move-generation errors.
4.  Test special moves independently.
5.  Verify that a move never leaves the moving side's king in check.
6.  Verify Zobrist hashes after make/undo.
7.  Compare search results before and after optimization.
8.  Track node counts and transposition-table hits.

This makes it easier to distinguish **rule-generation bugs** from
**search/evaluation bugs**.

------------------------------------------------------------------------

## 🤝 Contributing

Contributions and suggestions are welcome.

A useful contribution should ideally include:

-   A clear description of the change
-   Reproducible steps for bugs
-   Tests for move-generation changes
-   Perft validation when chess rules are modified
-   Benchmark results for performance changes

------------------------------------------------------------------------

## 📄 License

Add the project's chosen license here before publishing the repository
publicly.

For example:

``` text
MIT License
```

If no license has been selected yet, the repository should not claim a
specific open-source license.

------------------------------------------------------------------------

## 👤 Author

**Ayush Maurya**

B.Tech --- Data Science & Artificial Intelligence\
Indian Institute of Technology, Roorkee

This project is developed as a hands-on exploration of:

-   C++ software engineering
-   Algorithms and data structures
-   Game-tree search
-   Performance optimization
-   Chess programming
-   GUI development

------------------------------------------------------------------------

## ⭐ Project Status

**Active Development**

The engine currently has a functional chess-rule core, search system,
hashing/cache infrastructure, and SFML GUI. The next major stage is
improving **playing strength and search efficiency** through stronger
evaluation, iterative deepening, advanced move ordering/pruning,
benchmarking, and deeper UCI integration.
