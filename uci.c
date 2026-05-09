#include "stdio.h"
#include "string.h"
#include "defs.h"

/* Convert a UCI move string ("e2e4", "e7e8q") to internal move integer */
static int ParseMove(char *ptr, S_BOARD *pos) {
    if (ptr[1] < '1' || ptr[1] > '8') return NOMOVE;
    if (ptr[3] < '1' || ptr[3] > '8') return NOMOVE;
    if (ptr[0] < 'a' || ptr[0] > 'h') return NOMOVE;
    if (ptr[2] < 'a' || ptr[2] > 'h') return NOMOVE;

    int from = FR2SQ(ptr[0] - 'a', ptr[1] - '1');
    int to   = FR2SQ(ptr[2] - 'a', ptr[3] - '1');

    S_MOVELIST list[1];
    GenerateAllMoves(pos, list);

    int index, move, prom;
    for (index = 0; index < list->count; ++index) {
        move = list->moves[index].move;
        if (FROMSQ(move) != from || TOSQ(move) != to) continue;
        prom = PROMOTED(move);
        if (prom != EMPTY) {
            if (PieceKnight[prom]                                       && ptr[4] == 'n') return move;
            if (PieceRookQueen[prom]  && !PieceBishopQueen[prom]       && ptr[4] == 'r') return move;
            if (!PieceRookQueen[prom] &&  PieceBishopQueen[prom]       && ptr[4] == 'b') return move;
            if (PieceRookQueen[prom]  &&  PieceBishopQueen[prom]       && ptr[4] == 'q') return move;
            continue;
        }
        return move;
    }
    return NOMOVE;
}

/* Handle "position startpos [moves ...]" or "position fen <fen> [moves ...]" */
static void ParsePosition(char *lineIn, S_BOARD *pos) {
    lineIn += 9; /* skip "position " */
    char *ptr = lineIn;

    if (strncmp(lineIn, "startpos", 8) == 0) {
        ParseFen(START_FEN, pos);
    } else {
        ptr = strstr(lineIn, "fen");
        if (ptr == NULL) {
            ParseFen(START_FEN, pos);
        } else {
            ptr += 4;
            ParseFen(ptr, pos);
        }
    }

    ptr = strstr(lineIn, "moves");
    if (ptr != NULL) {
        ptr += 6;
        while (*ptr) {
            int move = ParseMove(ptr, pos);
            if (move == NOMOVE) break;
            MakeMove(pos, move);
            pos->ply = 0;
            while (*ptr && *ptr != ' ') ptr++;
            if (*ptr) ptr++;
        }
    }
}

/* Handle "go ..." command and kick off search */
static void ParseGo(char *line, S_SEARCHINFO *info, S_BOARD *pos) {
    int depth     = -1;
    int movestogo = 30;
    int movetime  = -1;
    int time      = -1;
    int inc       = 0;
    char *ptr;

    info->timeSet  = FALSE;
    info->infinite = FALSE;

    if ((ptr = strstr(line, "infinite"))  != NULL) info->infinite = TRUE;
    if ((ptr = strstr(line, "binc"))      != NULL && pos->side == BLACK) inc       = atoi(ptr + 5);
    if ((ptr = strstr(line, "winc"))      != NULL && pos->side == WHITE) inc       = atoi(ptr + 5);
    if ((ptr = strstr(line, "wtime"))     != NULL && pos->side == WHITE) time      = atoi(ptr + 6);
    if ((ptr = strstr(line, "btime"))     != NULL && pos->side == BLACK) time      = atoi(ptr + 6);
    if ((ptr = strstr(line, "movestogo")) != NULL) movestogo = atoi(ptr + 10);
    if ((ptr = strstr(line, "movetime"))  != NULL) movetime  = atoi(ptr + 9);
    if ((ptr = strstr(line, "depth"))     != NULL) depth     = atoi(ptr + 6);

    if (movetime != -1) { time = movetime; movestogo = 1; }

    info->startTime = GetTimeMs();
    info->depth     = (depth == -1) ? MAXDEPTH : depth;

    if (time != -1) {
        info->timeSet = TRUE;
        time /= movestogo;
        time -= 50;                           /* safety buffer */
        if (time < 10) time = 10;
        info->stopTime = info->startTime + time + inc;
    }

    SearchPosition(pos, info);
}

/* Main UCI event loop */
void Uci_Loop(S_BOARD *pos, S_SEARCHINFO *info) {
    setvbuf(stdin,  NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);

    char line[INPUTBUFFER];
    printf("id name %s\n", NAME);
    printf("id author VICE\n");
    printf("uciok\n");
    fflush(stdout);

    while (TRUE) {
        memset(line, 0, sizeof(line));
        fflush(stdout);
        if (!fgets(line, INPUTBUFFER, stdin)) continue;
        if (line[0] == '\n') continue;

        if (!strncmp(line, "isready",    7)) {
            printf("readyok\n");
            fflush(stdout);
        } else if (!strncmp(line, "position",  8)) {
            ParsePosition(line, pos);
        } else if (!strncmp(line, "ucinewgame",10)) {
            ParsePosition("position startpos\n", pos);
        } else if (!strncmp(line, "go",        2)) {
            ParseGo(line, info, pos);
        } else if (!strncmp(line, "quit",      4)) {
            info->quit = TRUE;
            break;
        } else if (!strncmp(line, "uci",       3)) {
            printf("id name %s\n", NAME);
            printf("id author VICE\n");
            printf("uciok\n");
            fflush(stdout);
        }

        if (info->quit) break;
    }
}
