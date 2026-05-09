#include "defs.h"

/* Knight, bishop, rook, king jump/slide offsets on 120-sq board */
static const int KnDir[8] = { 21, 19, 12,  8, -8, -12, -19, -21 };
static const int RkDir[4] = { 10, -10,  1,  -1 };
static const int BiDir[4] = { 11,   9, -9, -11 };
static const int KiDir[8] = { 10, -10,  1,  -1,  11,   9, -9, -11 };

int SqAttacked(const int sq, const int side, const S_BOARD *pos) {
    int pce, index, t_sq, dir;

    ASSERT(SqOnBoard(sq));
    ASSERT(SideValid(side));

    /* Pawns */
    if (side == WHITE) {
        if (pos->pieces[sq - 11] == wP || pos->pieces[sq - 9] == wP) return TRUE;
    } else {
        if (pos->pieces[sq + 11] == bP || pos->pieces[sq + 9] == bP) return TRUE;
    }

    /* Knights */
    for (index = 0; index < 8; ++index) {
        pce = pos->pieces[sq + KnDir[index]];
        if (pce != OFFBOARD && PieceKnight[pce] && PieceCol[pce] == side) return TRUE;
    }

    /* Rooks / Queens (straight) */
    for (index = 0; index < 4; ++index) {
        dir  = RkDir[index];
        t_sq = sq + dir;
        pce  = pos->pieces[t_sq];
        while (pce != OFFBOARD) {
            if (pce != EMPTY) {
                if (PieceRookQueen[pce] && PieceCol[pce] == side) return TRUE;
                break;
            }
            t_sq += dir;
            pce   = pos->pieces[t_sq];
        }
    }

    /* Bishops / Queens (diagonal) */
    for (index = 0; index < 4; ++index) {
        dir  = BiDir[index];
        t_sq = sq + dir;
        pce  = pos->pieces[t_sq];
        while (pce != OFFBOARD) {
            if (pce != EMPTY) {
                if (PieceBishopQueen[pce] && PieceCol[pce] == side) return TRUE;
                break;
            }
            t_sq += dir;
            pce   = pos->pieces[t_sq];
        }
    }

    /* King */
    for (index = 0; index < 8; ++index) {
        pce = pos->pieces[sq + KiDir[index]];
        if (pce != OFFBOARD && PieceKing[pce] && PieceCol[pce] == side) return TRUE;
    }

    return FALSE;
}

