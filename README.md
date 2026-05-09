# VICE Chess Engine

![Language](https://img.shields.io/badge/language-C-blue.svg)
![Protocol](https://img.shields.io/badge/protocol-UCI-brightgreen.svg)
![Compiler](https://img.shields.io/badge/compiler-GCC%2015-orange.svg)
![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey.svg)
![License](https://img.shields.io/badge/license-Unlicensed-red.svg)
![Status](https://img.shields.io/badge/status-working-success.svg)

A fully working C chess engine that communicates over the UCI protocol. It supports any UCI-compatible GUI (Arena, Cute Chess, Lucas Chess, etc.) and can search positions to arbitrary depth using iterative-deepening alpha-beta with quiescence search.

## Features

- **UCI protocol** — plug into any UCI-compatible chess GUI
- **120-square board representation** (10×12 layout with padding to simplify boundary checks)
- **Dual-layer representation** — array-based `pieces[120]` combined with pawn bitboards
- **FEN parsing** — load any standard chess position
- **Zobrist hashing** — incremental position fingerprinting for the PV table
- **Full move generation** — all legal moves including castling, en passant, promotions
- **Legal move enforcement** — `MakeMove` verifies the king is not left in check
- **Iterative-deepening alpha-beta** with quiescence search
- **Move ordering** — PV move, MVV/LVA captures, killer heuristic, history heuristic
- **Check extension** — extends search by one ply when in check
- **Principal variation table** (~1 MB transposition table)
- **Piece-square tables** for positional evaluation (pawns, knights, bishops, rooks)
- **Bishop pair bonus**
- **50-move rule and repetition detection**
- **Time management** — supports `movetime`, `wtime`/`btime`, and `depth` limits

## Project Structure

```
Chess-Engine/
├── vice.c        # Main entry point — UCI loop
├── defs.h        # All types, macros, and function prototypes
├── init.c        # Startup: lookup tables, hash keys, MVV/LVA
├── board.c       # Board state, FEN parsing, board printing
├── data.c        # Static piece property tables
├── bitboard.c    # 64-bit bitboard ops (CountBits, PopBit)
├── hushkeys.c    # Zobrist hash key generation
├── attack.c      # SqAttacked — detects attacks from any piece type
├── movegen.c     # GenerateAllMoves, GenerateAllCaps, PrMove, MoveExists
├── makemove.c    # MakeMove / TakeMove with incremental hashing
├── evaluate.c    # EvalPosition — material + piece-square tables
├── search.c      # Alpha-beta, quiescence, PV table, SearchPosition
├── uci.c         # UCI protocol loop (position, go, isready, quit)
├── misc.c        # GetTimeMs, ReadInput (Windows)
├── build.bat     # One-click Windows build script
└── Makefile      # GNU make build
```

## Build

### Requirements

- GCC via [MSYS2](https://www.msys2.org/) (or any MinGW distribution)

### Install MSYS2 + GCC (first time only)

```powershell
# Install MSYS2 via winget
winget install MSYS2.MSYS2

# Then install GCC inside MSYS2
C:\msys64\usr\bin\bash.exe -lc "pacman -S --noconfirm mingw-w64-x86_64-gcc"
```

### Compile

**Option A — batch script:**

```bat
build.bat
```

**Option B — manual:**

```powershell
& "C:\msys64\usr\bin\bash.exe" -lc 'export PATH="/mingw64/bin:$PATH"; cd "/c/Users/<you>/Documents/github/Chess-Engine" && gcc -Wall -O2 vice.c init.c bitboard.c hushkeys.c board.c data.c attack.c movegen.c makemove.c evaluate.c search.c misc.c uci.c -o vice.exe'
```

**Option C — GNU make (from MSYS2 shell):**

```bash
make
```

## Usage

### With a UCI GUI

1. Open your GUI (Arena, Cute Chess, Lucas Chess, etc.)
2. Add a new engine and point it at `vice.exe`
3. The engine will respond to UCI commands automatically

### Command line (manual UCI)

```text
vice.exe
uci
isready
position startpos
go depth 6
```

Example output:

```text
id name VICE 1.0
uciok
readyok
info score cp 30 depth 1 nodes 21 time 0 pv d2d4
info score cp 20 depth 5 nodes 22681 time 15 pv e2e4 e7e5 d2d4 d7d5 c1e3
bestmove e2e4
```

### Supported `go` parameters

| Parameter | Example | Description |
| --- | --- | --- |
| `depth <n>` | `go depth 8` | Search exactly n plies deep |
| `movetime <ms>` | `go movetime 5000` | Search for 5 seconds |
| `wtime / btime` | `go wtime 60000 btime 60000` | Clock-based time management |
| `infinite` | `go infinite` | Search until `stop` is received |

## Architecture

### Board Representation

The engine uses a **120-square (10×12) board**. Valid chess squares (A1–H8) sit at indices 21–98; the surrounding padding returns `OFFBOARD` so move generation never needs explicit boundary checks.

Two lookup arrays bridge the representations:

| Array | Maps |
|---|---|
| `Sq120ToSq64[120]` | 120-sq index → 64-sq index |
| `Sq64ToSq120[64]` | 64-sq index → 120-sq index |

### Move Encoding

Each move is packed into a single 28-bit integer:

| Bits | Field |
|---|---|
| 0–6 | From square |
| 7–13 | To square |
| 14–17 | Captured piece |
| 18 | En passant flag |
| 19 | Pawn-start flag |
| 20–23 | Promoted piece |
| 24 | Castle flag |

### Search

- **Iterative deepening** — searches depth 1, 2, 3, … up to the limit, using each iteration's result to order moves for the next
- **Alpha-beta** — prunes branches that can't improve the best known score
- **Quiescence search** — extends captures-only until a quiet position is reached, avoiding the horizon effect
- **Check extension** — adds one extra ply when the side to move is in check
- **Move ordering**: PV move → MVV/LVA captures → killer moves → history heuristic

### Evaluation

- Material balance (P=100, N=325, B=325, R=550, Q=1000, K=50000)
- Piece-square tables for pawns, knights, bishops, rooks
- Bishop pair bonus (+30 centipawns)
- Score is always returned relative to the side to move

### Piece Values

| Piece | Value (centipawns) |
| --- | --- |
| Pawn | 100 |
| Knight | 325 |
| Bishop | 325 |
| Rook | 550 |
| Queen | 1000 |
| King | 50000 |

## VS Code Setup

Update [.vscode/c_cpp_properties.json](.vscode/c_cpp_properties.json) so IntelliSense finds the new compiler:

```json
"compilerPath": "C:/msys64/mingw64/bin/gcc.exe"
```

## License

This project is unlicensed. All rights reserved.
