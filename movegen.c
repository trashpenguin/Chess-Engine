#include "stdio.h"
#include "defs.h"

/* MVV/LVA table: [victim][attacker] */
int MvvLvaScores[13][13];

void InitMvvLva(void) {
    static const int VictimScore[13] = {
        0, 100, 200, 300, 400, 500, 600,
           100, 200, 300, 400, 500, 600
    };
    int Attacker, Victim;
    for (Attacker = wP; Attacker <= bK; ++Attacker)
        for (Victim = wP; Victim <= bK; ++Victim)
            MvvLvaScores[Victim][Attacker] =
                VictimScore[Victim] + 6 - (VictimScore[Attacker] / 100);
}

/* ---- Move list helpers ---- */
static void AddQuietMove(const S_BOARD *pos, int move, S_MOVELIST *list) {
    list->moves[list->count].move  = move;
    list->moves[list->count].score = 0;
    list->count++;
}

static void AddCaptureMove(const S_BOARD *pos, int move, S_MOVELIST *list) {
    list->moves[list->count].move  = move;
    list->moves[list->count].score =
        MvvLvaScores[CAPTURED(move)][pos->pieces[FROMSQ(move)]];
    list->count++;
}

static void AddEnPassantMove(const S_BOARD *pos, int move, S_MOVELIST *list) {
    list->moves[list->count].move  = move;
    list->moves[list->count].score = 105 + 1000000; /* en passant is a good capture */
    list->count++;
}

/* ---- Pawn promotion helpers ---- */
static void AddWhitePawnCapMove(const S_BOARD *pos, int from, int to, int cap,
                                S_MOVELIST *list) {
    ASSERT(PieceValidEmpty(cap));
    ASSERT(SqOnBoard(from));
    ASSERT(SqOnBoard(to));

    if (RanksBrd[from] == RANK_7) {
        AddCaptureMove(pos, MOVE(from, to, cap, wQ, 0), list);
        AddCaptureMove(pos, MOVE(from, to, cap, wR, 0), list);
        AddCaptureMove(pos, MOVE(from, to, cap, wB, 0), list);
        AddCaptureMove(pos, MOVE(from, to, cap, wN, 0), list);
    } else {
        AddCaptureMove(pos, MOVE(from, to, cap, EMPTY, 0), list);
    }
}

static void AddWhitePawnMove(const S_BOARD *pos, int from, int to,
                             S_MOVELIST *list) {
    ASSERT(SqOnBoard(from));
    ASSERT(SqOnBoard(to));

    if (RanksBrd[from] == RANK_7) {
        AddQuietMove(pos, MOVE(from, to, EMPTY, wQ, 0), list);
        AddQuietMove(pos, MOVE(from, to, EMPTY, wR, 0), list);
        AddQuietMove(pos, MOVE(from, to, EMPTY, wB, 0), list);
        AddQuietMove(pos, MOVE(from, to, EMPTY, wN, 0), list);
    } else {
        AddQuietMove(pos, MOVE(from, to, EMPTY, EMPTY, 0), list);
    }
}

static void AddBlackPawnCapMove(const S_BOARD *pos, int from, int to, int cap,
                                S_MOVELIST *list) {
    ASSERT(PieceValidEmpty(cap));
    ASSERT(SqOnBoard(from));
    ASSERT(SqOnBoard(to));

    if (RanksBrd[from] == RANK_2) {
        AddCaptureMove(pos, MOVE(from, to, cap, bQ, 0), list);
        AddCaptureMove(pos, MOVE(from, to, cap, bR, 0), list);
        AddCaptureMove(pos, MOVE(from, to, cap, bB, 0), list);
        AddCaptureMove(pos, MOVE(from, to, cap, bN, 0), list);
    } else {
        AddCaptureMove(pos, MOVE(from, to, cap, EMPTY, 0), list);
    }
}

static void AddBlackPawnMove(const S_BOARD *pos, int from, int to,
                             S_MOVELIST *list) {
    ASSERT(SqOnBoard(from));
    ASSERT(SqOnBoard(to));

    if (RanksBrd[from] == RANK_2) {
        AddQuietMove(pos, MOVE(from, to, EMPTY, bQ, 0), list);
        AddQuietMove(pos, MOVE(from, to, EMPTY, bR, 0), list);
        AddQuietMove(pos, MOVE(from, to, EMPTY, bB, 0), list);
        AddQuietMove(pos, MOVE(from, to, EMPTY, bN, 0), list);
    } else {
        AddQuietMove(pos, MOVE(from, to, EMPTY, EMPTY, 0), list);
    }
}

/* ---- Direction tables ---- */
static const int KnDir[8] = { 21, 19, 12,  8, -8, -12, -19, -21 };
static const int RkDir[4] = { 10, -10,  1, -1 };
static const int BiDir[4] = { 11,   9, -9, -11 };
static const int KiDir[8] = { 10, -10,  1,  -1,  11,  9, -9, -11 };

/* ---- Generate all pseudo-legal moves ---- */
void GenerateAllMoves(const S_BOARD *pos, S_MOVELIST *list) {
    ASSERT(SideValid(pos->side));

    list->count = 0;

    int side  = pos->side;
    int xside = side ^ 1;
    int pceType, pceNum, sq, t_sq, pce, index, dir;

    if (side == WHITE) {
        /* White pawns */
        pceNum = pos->pceNum[wP];
        for (pceType = 0; pceType < pceNum; ++pceType) {
            sq = pos->pList[wP][pceType];
            ASSERT(SqOnBoard(sq));

            /* Single push */
            if (pos->pieces[sq + 10] == EMPTY) {
                AddWhitePawnMove(pos, sq, sq + 10, list);
                /* Double push from rank 2 */
                if (RanksBrd[sq] == RANK_2 && pos->pieces[sq + 20] == EMPTY)
                    AddQuietMove(pos, MOVE(sq, sq+20, EMPTY, EMPTY, MFLAGPS), list);
            }
            /* Captures */
            if (!SQOFFBOARD(sq + 9) && PieceCol[pos->pieces[sq + 9]] == BLACK)
                AddWhitePawnCapMove(pos, sq, sq + 9, pos->pieces[sq + 9], list);
            if (!SQOFFBOARD(sq + 11) && PieceCol[pos->pieces[sq + 11]] == BLACK)
                AddWhitePawnCapMove(pos, sq, sq + 11, pos->pieces[sq + 11], list);
            /* En passant */
            if (pos->enPas != NO_SQ) {
                if (sq + 9  == pos->enPas)
                    AddEnPassantMove(pos, MOVE(sq, sq+9,  EMPTY, EMPTY, MFLAGEP), list);
                if (sq + 11 == pos->enPas)
                    AddEnPassantMove(pos, MOVE(sq, sq+11, EMPTY, EMPTY, MFLAGEP), list);
            }
        }
        /* Castling */
        if (pos->castlePerm & WKCA) {
            if (pos->pieces[F1] == EMPTY && pos->pieces[G1] == EMPTY) {
                if (!SqAttacked(E1, BLACK, pos) && !SqAttacked(F1, BLACK, pos))
                    AddQuietMove(pos, MOVE(E1, G1, EMPTY, EMPTY, MFLAGCA), list);
            }
        }
        if (pos->castlePerm & WQCA) {
            if (pos->pieces[D1] == EMPTY && pos->pieces[C1] == EMPTY &&
                pos->pieces[B1] == EMPTY) {
                if (!SqAttacked(E1, BLACK, pos) && !SqAttacked(D1, BLACK, pos))
                    AddQuietMove(pos, MOVE(E1, C1, EMPTY, EMPTY, MFLAGCA), list);
            }
        }
    } else {
        /* Black pawns */
        pceNum = pos->pceNum[bP];
        for (pceType = 0; pceType < pceNum; ++pceType) {
            sq = pos->pList[bP][pceType];
            ASSERT(SqOnBoard(sq));

            if (pos->pieces[sq - 10] == EMPTY) {
                AddBlackPawnMove(pos, sq, sq - 10, list);
                if (RanksBrd[sq] == RANK_7 && pos->pieces[sq - 20] == EMPTY)
                    AddQuietMove(pos, MOVE(sq, sq-20, EMPTY, EMPTY, MFLAGPS), list);
            }
            if (!SQOFFBOARD(sq - 9) && PieceCol[pos->pieces[sq - 9]] == WHITE)
                AddBlackPawnCapMove(pos, sq, sq - 9, pos->pieces[sq - 9], list);
            if (!SQOFFBOARD(sq - 11) && PieceCol[pos->pieces[sq - 11]] == WHITE)
                AddBlackPawnCapMove(pos, sq, sq - 11, pos->pieces[sq - 11], list);
            if (pos->enPas != NO_SQ) {
                if (sq - 9  == pos->enPas)
                    AddEnPassantMove(pos, MOVE(sq, sq-9,  EMPTY, EMPTY, MFLAGEP), list);
                if (sq - 11 == pos->enPas)
                    AddEnPassantMove(pos, MOVE(sq, sq-11, EMPTY, EMPTY, MFLAGEP), list);
            }
        }
        /* Castling */
        if (pos->castlePerm & BKCA) {
            if (pos->pieces[F8] == EMPTY && pos->pieces[G8] == EMPTY) {
                if (!SqAttacked(E8, WHITE, pos) && !SqAttacked(F8, WHITE, pos))
                    AddQuietMove(pos, MOVE(E8, G8, EMPTY, EMPTY, MFLAGCA), list);
            }
        }
        if (pos->castlePerm & BQCA) {
            if (pos->pieces[D8] == EMPTY && pos->pieces[C8] == EMPTY &&
                pos->pieces[B8] == EMPTY) {
                if (!SqAttacked(E8, WHITE, pos) && !SqAttacked(D8, WHITE, pos))
                    AddQuietMove(pos, MOVE(E8, C8, EMPTY, EMPTY, MFLAGCA), list);
            }
        }
    }

    /* ---- Sliding pieces ---- */
    /* Bishops + Queens */
    int pceIndex = (side == WHITE) ? wB : bB;
    pceNum = pos->pceNum[pceIndex];
    for (pceType = 0; pceType < pceNum; ++pceType) {
        sq = pos->pList[pceIndex][pceType];
        ASSERT(SqOnBoard(sq));
        for (index = 0; index < 4; ++index) {
            dir  = BiDir[index];
            t_sq = sq + dir;
            while (!SQOFFBOARD(t_sq)) {
                pce = pos->pieces[t_sq];
                if (pce == EMPTY) {
                    AddQuietMove(pos, MOVE(sq, t_sq, EMPTY, EMPTY, 0), list);
                } else {
                    if (PieceCol[pce] == xside)
                        AddCaptureMove(pos, MOVE(sq, t_sq, pce, EMPTY, 0), list);
                    break;
                }
                t_sq += dir;
            }
        }
    }

    /* Rooks + Queens */
    pceIndex = (side == WHITE) ? wR : bR;
    pceNum = pos->pceNum[pceIndex];
    for (pceType = 0; pceType < pceNum; ++pceType) {
        sq = pos->pList[pceIndex][pceType];
        ASSERT(SqOnBoard(sq));
        for (index = 0; index < 4; ++index) {
            dir  = RkDir[index];
            t_sq = sq + dir;
            while (!SQOFFBOARD(t_sq)) {
                pce = pos->pieces[t_sq];
                if (pce == EMPTY) {
                    AddQuietMove(pos, MOVE(sq, t_sq, EMPTY, EMPTY, 0), list);
                } else {
                    if (PieceCol[pce] == xside)
                        AddCaptureMove(pos, MOVE(sq, t_sq, pce, EMPTY, 0), list);
                    break;
                }
                t_sq += dir;
            }
        }
    }

    /* Queens (both diagonals and straights) */
    pceIndex = (side == WHITE) ? wQ : bQ;
    pceNum = pos->pceNum[pceIndex];
    for (pceType = 0; pceType < pceNum; ++pceType) {
        sq = pos->pList[pceIndex][pceType];
        ASSERT(SqOnBoard(sq));
        for (index = 0; index < 4; ++index) {
            dir  = BiDir[index];
            t_sq = sq + dir;
            while (!SQOFFBOARD(t_sq)) {
                pce = pos->pieces[t_sq];
                if (pce == EMPTY) {
                    AddQuietMove(pos, MOVE(sq, t_sq, EMPTY, EMPTY, 0), list);
                } else {
                    if (PieceCol[pce] == xside)
                        AddCaptureMove(pos, MOVE(sq, t_sq, pce, EMPTY, 0), list);
                    break;
                }
                t_sq += dir;
            }
        }
        for (index = 0; index < 4; ++index) {
            dir  = RkDir[index];
            t_sq = sq + dir;
            while (!SQOFFBOARD(t_sq)) {
                pce = pos->pieces[t_sq];
                if (pce == EMPTY) {
                    AddQuietMove(pos, MOVE(sq, t_sq, EMPTY, EMPTY, 0), list);
                } else {
                    if (PieceCol[pce] == xside)
                        AddCaptureMove(pos, MOVE(sq, t_sq, pce, EMPTY, 0), list);
                    break;
                }
                t_sq += dir;
            }
        }
    }

    /* ---- Non-sliding pieces ---- */
    /* Knights */
    pceIndex = (side == WHITE) ? wN : bN;
    pceNum = pos->pceNum[pceIndex];
    for (pceType = 0; pceType < pceNum; ++pceType) {
        sq = pos->pList[pceIndex][pceType];
        ASSERT(SqOnBoard(sq));
        for (index = 0; index < 8; ++index) {
            t_sq = sq + KnDir[index];
            if (SQOFFBOARD(t_sq)) continue;
            pce = pos->pieces[t_sq];
            if (pce == EMPTY)
                AddQuietMove(pos, MOVE(sq, t_sq, EMPTY, EMPTY, 0), list);
            else if (PieceCol[pce] == xside)
                AddCaptureMove(pos, MOVE(sq, t_sq, pce, EMPTY, 0), list);
        }
    }

    /* King */
    pceIndex = (side == WHITE) ? wK : bK;
    pceNum = pos->pceNum[pceIndex];
    for (pceType = 0; pceType < pceNum; ++pceType) {
        sq = pos->pList[pceIndex][pceType];
        ASSERT(SqOnBoard(sq));
        for (index = 0; index < 8; ++index) {
            t_sq = sq + KiDir[index];
            if (SQOFFBOARD(t_sq)) continue;
            pce = pos->pieces[t_sq];
            if (pce == EMPTY)
                AddQuietMove(pos, MOVE(sq, t_sq, EMPTY, EMPTY, 0), list);
            else if (PieceCol[pce] == xside)
                AddCaptureMove(pos, MOVE(sq, t_sq, pce, EMPTY, 0), list);
        }
    }
}

/* ---- Generate capture-only moves (for quiescence search) ---- */
void GenerateAllCaps(const S_BOARD *pos, S_MOVELIST *list) {
    ASSERT(SideValid(pos->side));

    list->count = 0;

    int side  = pos->side;
    int xside = side ^ 1;
    int pceType, pceNum, sq, t_sq, pce, index, dir, pceIndex;

    if (side == WHITE) {
        pceNum = pos->pceNum[wP];
        for (pceType = 0; pceType < pceNum; ++pceType) {
            sq = pos->pList[wP][pceType];
            if (!SQOFFBOARD(sq + 9) && PieceCol[pos->pieces[sq + 9]] == BLACK)
                AddWhitePawnCapMove(pos, sq, sq + 9, pos->pieces[sq + 9], list);
            if (!SQOFFBOARD(sq + 11) && PieceCol[pos->pieces[sq + 11]] == BLACK)
                AddWhitePawnCapMove(pos, sq, sq + 11, pos->pieces[sq + 11], list);
            if (pos->enPas != NO_SQ) {
                if (sq + 9  == pos->enPas)
                    AddEnPassantMove(pos, MOVE(sq, sq+9,  EMPTY, EMPTY, MFLAGEP), list);
                if (sq + 11 == pos->enPas)
                    AddEnPassantMove(pos, MOVE(sq, sq+11, EMPTY, EMPTY, MFLAGEP), list);
            }
        }
    } else {
        pceNum = pos->pceNum[bP];
        for (pceType = 0; pceType < pceNum; ++pceType) {
            sq = pos->pList[bP][pceType];
            if (!SQOFFBOARD(sq - 9) && PieceCol[pos->pieces[sq - 9]] == WHITE)
                AddBlackPawnCapMove(pos, sq, sq - 9, pos->pieces[sq - 9], list);
            if (!SQOFFBOARD(sq - 11) && PieceCol[pos->pieces[sq - 11]] == WHITE)
                AddBlackPawnCapMove(pos, sq, sq - 11, pos->pieces[sq - 11], list);
            if (pos->enPas != NO_SQ) {
                if (sq - 9  == pos->enPas)
                    AddEnPassantMove(pos, MOVE(sq, sq-9,  EMPTY, EMPTY, MFLAGEP), list);
                if (sq - 11 == pos->enPas)
                    AddEnPassantMove(pos, MOVE(sq, sq-11, EMPTY, EMPTY, MFLAGEP), list);
            }
        }
    }

    /* Sliding captures */
    pceIndex = (side == WHITE) ? wB : bB;
    pceNum = pos->pceNum[pceIndex];
    for (pceType = 0; pceType < pceNum; ++pceType) {
        sq = pos->pList[pceIndex][pceType];
        for (index = 0; index < 4; ++index) {
            dir = BiDir[index]; t_sq = sq + dir;
            while (!SQOFFBOARD(t_sq)) {
                pce = pos->pieces[t_sq];
                if (pce != EMPTY) {
                    if (PieceCol[pce] == xside)
                        AddCaptureMove(pos, MOVE(sq, t_sq, pce, EMPTY, 0), list);
                    break;
                }
                t_sq += dir;
            }
        }
    }
    pceIndex = (side == WHITE) ? wR : bR;
    pceNum = pos->pceNum[pceIndex];
    for (pceType = 0; pceType < pceNum; ++pceType) {
        sq = pos->pList[pceIndex][pceType];
        for (index = 0; index < 4; ++index) {
            dir = RkDir[index]; t_sq = sq + dir;
            while (!SQOFFBOARD(t_sq)) {
                pce = pos->pieces[t_sq];
                if (pce != EMPTY) {
                    if (PieceCol[pce] == xside)
                        AddCaptureMove(pos, MOVE(sq, t_sq, pce, EMPTY, 0), list);
                    break;
                }
                t_sq += dir;
            }
        }
    }
    pceIndex = (side == WHITE) ? wQ : bQ;
    pceNum = pos->pceNum[pceIndex];
    for (pceType = 0; pceType < pceNum; ++pceType) {
        sq = pos->pList[pceIndex][pceType];
        for (index = 0; index < 4; ++index) {
            dir = BiDir[index]; t_sq = sq + dir;
            while (!SQOFFBOARD(t_sq)) {
                pce = pos->pieces[t_sq];
                if (pce != EMPTY) {
                    if (PieceCol[pce] == xside)
                        AddCaptureMove(pos, MOVE(sq, t_sq, pce, EMPTY, 0), list);
                    break;
                }
                t_sq += dir;
            }
        }
        for (index = 0; index < 4; ++index) {
            dir = RkDir[index]; t_sq = sq + dir;
            while (!SQOFFBOARD(t_sq)) {
                pce = pos->pieces[t_sq];
                if (pce != EMPTY) {
                    if (PieceCol[pce] == xside)
                        AddCaptureMove(pos, MOVE(sq, t_sq, pce, EMPTY, 0), list);
                    break;
                }
                t_sq += dir;
            }
        }
    }

    /* Non-sliding captures */
    pceIndex = (side == WHITE) ? wN : bN;
    pceNum = pos->pceNum[pceIndex];
    for (pceType = 0; pceType < pceNum; ++pceType) {
        sq = pos->pList[pceIndex][pceType];
        for (index = 0; index < 8; ++index) {
            t_sq = sq + KnDir[index];
            if (!SQOFFBOARD(t_sq)) {
                pce = pos->pieces[t_sq];
                if (PieceCol[pce] == xside)
                    AddCaptureMove(pos, MOVE(sq, t_sq, pce, EMPTY, 0), list);
            }
        }
    }
    pceIndex = (side == WHITE) ? wK : bK;
    pceNum = pos->pceNum[pceIndex];
    for (pceType = 0; pceType < pceNum; ++pceType) {
        sq = pos->pList[pceIndex][pceType];
        for (index = 0; index < 8; ++index) {
            t_sq = sq + KiDir[index];
            if (!SQOFFBOARD(t_sq)) {
                pce = pos->pieces[t_sq];
                if (PieceCol[pce] == xside)
                    AddCaptureMove(pos, MOVE(sq, t_sq, pce, EMPTY, 0), list);
            }
        }
    }
}

/* ---- Utility ---- */
char *PrMove(const int move) {
    static char MvStr[6];
    int ff   = FilesBrd[FROMSQ(move)];
    int rf   = RanksBrd[FROMSQ(move)];
    int ft   = FilesBrd[TOSQ(move)];
    int rt   = RanksBrd[TOSQ(move)];
    int prom = PROMOTED(move);

    if (prom) {
        char pchar = 'q';
        if (PieceKnight[prom])                                  pchar = 'n';
        else if (PieceRookQueen[prom] && !PieceBishopQueen[prom]) pchar = 'r';
        else if (!PieceRookQueen[prom] && PieceBishopQueen[prom]) pchar = 'b';
        sprintf(MvStr, "%c%c%c%c%c", 'a'+ff, '1'+rf, 'a'+ft, '1'+rt, pchar);
    } else {
        sprintf(MvStr, "%c%c%c%c", 'a'+ff, '1'+rf, 'a'+ft, '1'+rt);
    }
    return MvStr;
}

int MoveExists(S_BOARD *pos, const int move) {
    S_MOVELIST list[1];
    GenerateAllMoves(pos, list);
    int index;
    for (index = 0; index < list->count; ++index) {
        if (list->moves[index].move != move) continue;
        if (!MakeMove(pos, move)) return FALSE;
        TakeMove(pos);
        return TRUE;
    }
    return FALSE;
}
