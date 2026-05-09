# VICE Chess Engine

A C-based chess engine implementation featuring a 120-square board representation, Zobrist hashing, and bitboard operations. Currently in active development — core infrastructure is complete with move generation and search still to come.

## Features

- **120-square board representation** (10×12 layout with padding to simplify boundary checks)
- **Dual-layer representation**: array-based `pieces[120]` combined with pawn bitboards
- **FEN parsing** to load any standard chess position
- **Zobrist hashing** for unique position fingerprinting
- **Piece lists** for fast piece iteration without full board scans
- **Material tracking** with incremental major/minor piece counts
- **Move history** (`S_UNDO` stack) to support undo/redo during search

## Project Structure

```
Chess-Engine/
├── vice.c          # Main entry point
├── defs.h          # Core definitions, structs, and macros
├── init.c          # Startup initialization (lookup tables, hash keys)
├── board.c         # Board state, FEN parsing, board printing
├── bitboard.c      # 64-bit bitboard operations (CountBits, PopBit)
├── hushkeys.c      # Zobrist hash key generation (GeneratePosKey)
├── data.c          # Static piece property tables (values, colors, types)
└── Makefile        # Build script
```

## Build

Requires GCC (MinGW on Windows).

```bash
make
```

Or manually:

```bash
gcc vice.c init.c bitboard.c hushkeys.c board.c data.c -o vice
```

## Run

```bash
./vice
```

Currently outputs the square-mapping conversion tables as an initialization sanity check.

## Architecture

### Board Representation

The engine uses a **120-square (10×12) board** internally. Only squares 21–98 correspond to valid chess squares (A1–H8); the outer ring of padding squares is used to detect out-of-bounds moves without explicit boundary checks.

Two mapping arrays bridge the representations:

| Array | Purpose |
|---|---|
| `Sq120ToSq64[120]` | Maps 120-sq index → 64-sq index |
| `Sq64ToSq120[64]` | Maps 64-sq index → 120-sq index |

### Key Structs

**`S_BOARD`** — the primary game state:

| Field | Description |
|---|---|
| `pieces[BRD_SQ_NUM]` | Piece on each of the 120 squares |
| `pawns[3]` | Bitboards for white, black, and both pawns |
| `KingSq[2]` | King square indices for each side |
| `posKey` | Zobrist hash of the current position |
| `castlePerm` | Castling rights (4-bit flags) |
| `enPas` | En passant target square |
| `fiftyMove` | Half-move counter for the 50-move rule |
| `pList[13][10]` | Piece lists: up to 10 pieces per type |
| `material[2]` | Total material value for each side |
| `history[MAXGAMEMOVES]` | Undo stack |

**`S_UNDO`** — one entry per move for undoing:

| Field | Description |
|---|---|
| `move` | Encoded move |
| `castlePerm` | Castling rights before the move |
| `enPas` | En passant square before the move |
| `fiftyMove` | 50-move counter before the move |
| `posKey` | Position hash before the move |

### Piece Values

| Piece | Value |
|---|---|
| Pawn | 100 |
| Knight | 325 |
| Bishop | 325 |
| Rook | 550 |
| Queen | 1000 |
| King | 50000 |

### Zobrist Hashing

Position keys are built in `hushkeys.c` (`GeneratePosKey`) by XOR-combining:

- `PieceKeys[13][120]` — one random key per (piece type, square) pair
- `SideKey` — XORed when it is White's turn
- `CastleKeys[16]` — one key per castling-rights combination
- The en passant square key

Random 64-bit keys are seeded during `AllInit()` → `InitHashKeys()`.

### Bitboard Utilities

| Function | Description |
|---|---|
| `CountBits(U64 b)` | Counts set bits (Brian Kernighan's algorithm) |
| `PopBit(U64 *bb)` | Returns and clears the least-significant set bit (De Bruijn) |
| `PrintBitBoard(U64 bb)` | Prints an 8×8 ASCII visualization of a bitboard |

## Implementation Status

| Component | Status |
|---|---|
| Board representation | Complete |
| FEN parsing | Complete |
| Zobrist hashing | Complete |
| Bitboard operations | Complete |
| Material tracking | Complete |
| Board printing | Complete |
| Move generation | Not yet implemented |
| Move validation | Not yet implemented |
| Search (alpha-beta) | Not yet implemented |
| Evaluation function | Not yet implemented |
| UCI protocol | Not yet implemented |

## Development Environment

- **Compiler**: GCC via MinGW32 (`C:/mingw32/bin/gcc.exe`)
- **IDE**: Visual Studio Code with C/C++ extension
- **Debugger**: GDB (configured in `.vscode/launch.json`)
- **OS**: Windows

## License

This project is unlicensed. All rights reserved.
