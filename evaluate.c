#include "defs.h"

/* Piece-square bonus tables (from white's perspective).
   Black mirrors via Mirror64.                            */
static const int PawnTable[64] = {
     0,  0,  0,  0,  0,  0,  0,  0,
    10, 10,  0,-10,-10,  0, 10, 10,
     5,  0,  0,  5,  5,  0,  0,  5,
     0,  0, 10, 20, 20, 10,  0,  0,
     5,  5,  5, 10, 10,  5,  5,  5,
    10, 10, 10, 20, 20, 10, 10, 10,
    20, 20, 20, 30, 30, 20, 20, 20,
     0,  0,  0,  0,  0,  0,  0,  0
};

static const int KnightTable[64] = {
     0,-10,  0,  0,  0,  0,-10,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  5,  5,  5,  5,  0,  0,
     0,  5, 10, 15, 15, 10,  5,  0,
     5, 10, 15, 20, 20, 15, 10,  5,
     5, 10, 15, 20, 20, 15, 10,  5,
     0,  5, 10, 15, 15, 10,  5,  0,
     0,  0,  0,  0,  0,  0,  0,  0
};

static const int BishopTable[64] = {
     0,  0,-10,  0,  0,-10,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0, 10, 10,  0,  0,  0,
     0,  0, 10, 20, 20, 10,  0,  0,
     0,  0, 10, 20, 20, 10,  0,  0,
     0, 10,  0,  0,  0,  0, 10,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,-10,  0,  0,-10,  0,  0
};

static const int RookTable[64] = {
     0,  0,  5, 10, 10,  5,  0,  0,
     0,  0,  5, 10, 10,  5,  0,  0,
     0,  0,  5, 10, 10,  5,  0,  0,
     0,  0,  5, 10, 10,  5,  0,  0,
     0,  0,  5, 10, 10,  5,  0,  0,
     0,  0,  5, 10, 10,  5,  0,  0,
    25, 25, 25, 25, 25, 25, 25, 25,
     0,  0,  5, 10, 10,  5,  0,  0
};

/* Mirrors a 64-sq index so black uses the same table as white */
static const int Mirror64[64] = {
    56, 57, 58, 59, 60, 61, 62, 63,
    48, 49, 50, 51, 52, 53, 54, 55,
    40, 41, 42, 43, 44, 45, 46, 47,
    32, 33, 34, 35, 36, 37, 38, 39,
    24, 25, 26, 27, 28, 29, 30, 31,
    16, 17, 18, 19, 20, 21, 22, 23,
     8,  9, 10, 11, 12, 13, 14, 15,
     0,  1,  2,  3,  4,  5,  6,  7
};

#define ENDGAME_MAT (1 * PieceVal[wR] + 2 * PieceVal[wN] + \
                     2 * PieceVal[wP] + PieceVal[wK])

int EvalPosition(const S_BOARD *pos) {
    int pceNum, sq, index;
    int score = pos->material[WHITE] - pos->material[BLACK];

    /* Pawn bonuses */
    pceNum = pos->pceNum[wP];
    for (index = 0; index < pceNum; ++index) {
        sq     = pos->pList[wP][index];
        score += PawnTable[SQ64(sq)];
    }
    pceNum = pos->pceNum[bP];
    for (index = 0; index < pceNum; ++index) {
        sq     = pos->pList[bP][index];
        score -= PawnTable[Mirror64[SQ64(sq)]];
    }

    /* Knight bonuses */
    pceNum = pos->pceNum[wN];
    for (index = 0; index < pceNum; ++index) {
        sq     = pos->pList[wN][index];
        score += KnightTable[SQ64(sq)];
    }
    pceNum = pos->pceNum[bN];
    for (index = 0; index < pceNum; ++index) {
        sq     = pos->pList[bN][index];
        score -= KnightTable[Mirror64[SQ64(sq)]];
    }

    /* Bishop bonuses */
    pceNum = pos->pceNum[wB];
    for (index = 0; index < pceNum; ++index) {
        sq     = pos->pList[wB][index];
        score += BishopTable[SQ64(sq)];
    }
    pceNum = pos->pceNum[bB];
    for (index = 0; index < pceNum; ++index) {
        sq     = pos->pList[bB][index];
        score -= BishopTable[Mirror64[SQ64(sq)]];
    }

    /* Rook bonuses */
    pceNum = pos->pceNum[wR];
    for (index = 0; index < pceNum; ++index) {
        sq     = pos->pList[wR][index];
        score += RookTable[SQ64(sq)];
    }
    pceNum = pos->pceNum[bR];
    for (index = 0; index < pceNum; ++index) {
        sq     = pos->pList[bR][index];
        score -= RookTable[Mirror64[SQ64(sq)]];
    }

    /* Bishop pair bonus */
    if (pos->pceNum[wB] >= 2) score += 30;
    if (pos->pceNum[bB] >= 2) score -= 30;

    return (pos->side == WHITE) ? score : -score;
}
