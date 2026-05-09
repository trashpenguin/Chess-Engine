#include "stdio.h"
#include "defs.h"

/* Incremental hash update helpers */
#define HASH_PCE(pce, sq)  (pos->posKey ^= (PieceKeys[(pce)][(sq)]))
#define HASH_CA            (pos->posKey ^= (CastleKeys[(pos->castlePerm)]))
#define HASH_SIDE          (pos->posKey ^= SideKeys)
#define HASH_EP            (pos->posKey ^= (PieceKeys[EMPTY][(pos->enPas)]))

/* Remove castling rights when king or rook moves off their start square.
   AND pos->castlePerm with this table entry for from-sq and to-sq.       */
static const int CastlePerm[120] = {
    15,15,15,15,15,15,15,15,15,15,
    15,15,15,15,15,15,15,15,15,15,
    15,13,15,15,15,12,15,15,14,15,   /* rank 1: a1=21→13, e1=25→12, h1=28→14 */
    15,15,15,15,15,15,15,15,15,15,
    15,15,15,15,15,15,15,15,15,15,
    15,15,15,15,15,15,15,15,15,15,
    15,15,15,15,15,15,15,15,15,15,
    15,15,15,15,15,15,15,15,15,15,
    15,15,15,15,15,15,15,15,15,15,
    15, 7,15,15,15, 3,15,15,11,15,   /* rank 8: a8=91→7,  e8=95→3,  h8=98→11 */
    15,15,15,15,15,15,15,15,15,15,
    15,15,15,15,15,15,15,15,15,15
};

/* ---- Internal helpers ---- */
static void ClearPiece(const int sq, S_BOARD *pos) {
    ASSERT(SqOnBoard(sq));
    int pce = pos->pieces[sq];
    ASSERT(PieceValid(pce));
    int col     = PieceCol[pce];
    int index;
    int t_pceNum = -1;

    HASH_PCE(pce, sq);
    pos->pieces[sq] = EMPTY;
    pos->material[col] -= PieceVal[pce];

    if (PieceBig[pce]) pos->bigPce[col]--;
    if (PieceMaj[pce]) pos->majPce[col]--;
    if (PieceMin[pce]) pos->minPce[col]--;

    if (PiecePawn[pce]) {
        CLRBIT(pos->pawns[col],  SQ64(sq));
        CLRBIT(pos->pawns[BOTH], SQ64(sq));
    }

    for (index = 0; index < pos->pceNum[pce]; ++index) {
        if (pos->pList[pce][index] == sq) { t_pceNum = index; break; }
    }
    ASSERT(t_pceNum != -1);
    pos->pceNum[pce]--;
    pos->pList[pce][t_pceNum] = pos->pList[pce][pos->pceNum[pce]];
}

static void AddPiece(const int sq, S_BOARD *pos, const int pce) {
    ASSERT(PieceValid(pce));
    ASSERT(SqOnBoard(sq));
    int col = PieceCol[pce];

    HASH_PCE(pce, sq);
    pos->pieces[sq] = pce;
    pos->material[col] += PieceVal[pce];

    if (PieceBig[pce]) pos->bigPce[col]++;
    if (PieceMaj[pce]) pos->majPce[col]++;
    if (PieceMin[pce]) pos->minPce[col]++;

    if (PiecePawn[pce]) {
        SETBIT(pos->pawns[col],  SQ64(sq));
        SETBIT(pos->pawns[BOTH], SQ64(sq));
    }
    pos->pList[pce][pos->pceNum[pce]++] = sq;
}

static void MovePiece(const int from, const int to, S_BOARD *pos) {
    ASSERT(SqOnBoard(from));
    ASSERT(SqOnBoard(to));
    int pce = pos->pieces[from];
    int col = PieceCol[pce];
    int index;

    HASH_PCE(pce, from);
    pos->pieces[from] = EMPTY;

    HASH_PCE(pce, to);
    pos->pieces[to] = pce;

    if (PiecePawn[pce]) {
        CLRBIT(pos->pawns[col],  SQ64(from));
        CLRBIT(pos->pawns[BOTH], SQ64(from));
        SETBIT(pos->pawns[col],  SQ64(to));
        SETBIT(pos->pawns[BOTH], SQ64(to));
    }

    if (PieceKing[pce]) pos->KingSq[col] = to;

    for (index = 0; index < pos->pceNum[pce]; ++index) {
        if (pos->pList[pce][index] == from) { pos->pList[pce][index] = to; break; }
    }
}

/* ---- Public API ---- */
int MakeMove(S_BOARD *pos, int move) {
    ASSERT(SideValid(pos->side));

    int from = FROMSQ(move);
    int to   = TOSQ(move);
    int side = pos->side;

    ASSERT(SqOnBoard(from));
    ASSERT(SqOnBoard(to));
    ASSERT(PieceValid(pos->pieces[from]));

    /* Save state for undo */
    pos->history[pos->hisply].posKey     = pos->posKey;
    pos->history[pos->hisply].move       = move;
    pos->history[pos->hisply].fiftyMove  = pos->fiftyMove;
    pos->history[pos->hisply].enPas      = pos->enPas;
    pos->history[pos->hisply].castlePerm = pos->castlePerm;

    /* Special moves */
    if (move & MFLAGEP) {
        if (side == WHITE) ClearPiece(to - 10, pos);
        else               ClearPiece(to + 10, pos);
    } else if (move & MFLAGCA) {
        switch (to) {
            case G1: MovePiece(H1, F1, pos); break;
            case C1: MovePiece(A1, D1, pos); break;
            case G8: MovePiece(H8, F8, pos); break;
            case C8: MovePiece(A8, D8, pos); break;
            default: ASSERT(FALSE);
        }
    }

    /* Remove old hash contributions */
    if (pos->enPas != NO_SQ) HASH_EP;
    HASH_CA;

    pos->castlePerm &= CastlePerm[from];
    pos->castlePerm &= CastlePerm[to];
    pos->enPas       = NO_SQ;
    pos->fiftyMove++;

    /* Re-hash castle rights */
    HASH_CA;

    /* Capture */
    int captured = CAPTURED(move);
    if (captured != EMPTY) {
        ASSERT(PieceValidEmpty(captured));
        ClearPiece(to, pos);
        pos->fiftyMove = 0;
    }

    pos->hisply++;
    pos->ply++;

    /* Pawn moves */
    if (PiecePawn[pos->pieces[from]]) {
        pos->fiftyMove = 0;
        if (move & MFLAGPS) {
            if (side == WHITE) pos->enPas = from + 10;
            else               pos->enPas = from - 10;
            HASH_EP;
        }
    }

    MovePiece(from, to, pos);

    /* Promotion */
    int prPce = PROMOTED(move);
    if (prPce != EMPTY) {
        ASSERT(PieceValidEmpty(prPce) && !PiecePawn[prPce]);
        ClearPiece(to, pos);
        AddPiece(to, pos, prPce);
    }

    pos->side ^= 1;
    HASH_SIDE;

    /* Legality: is our king in check after the move? */
    if (SqAttacked(pos->KingSq[side], pos->side, pos)) {
        TakeMove(pos);
        return FALSE;
    }
    return TRUE;
}

void TakeMove(S_BOARD *pos) {
    ASSERT(SideValid(pos->side));
    ASSERT(pos->hisply > 0);

    pos->hisply--;
    pos->ply--;

    int move = pos->history[pos->hisply].move;
    int from = FROMSQ(move);
    int to   = TOSQ(move);

    /* Restore hash for ep/castle before overwriting */
    if (pos->enPas != NO_SQ) HASH_EP;
    HASH_CA;

    pos->castlePerm = pos->history[pos->hisply].castlePerm;
    pos->fiftyMove  = pos->history[pos->hisply].fiftyMove;
    pos->enPas      = pos->history[pos->hisply].enPas;

    if (pos->enPas != NO_SQ) HASH_EP;
    HASH_CA;

    pos->side ^= 1;
    HASH_SIDE;

    /* Undo special moves */
    if (move & MFLAGEP) {
        if (pos->side == WHITE) AddPiece(to - 10, pos, bP);
        else                    AddPiece(to + 10, pos, wP);
    } else if (move & MFLAGCA) {
        switch (to) {
            case G1: MovePiece(F1, H1, pos); break;
            case C1: MovePiece(D1, A1, pos); break;
            case G8: MovePiece(F8, H8, pos); break;
            case C8: MovePiece(D8, A8, pos); break;
            default: ASSERT(FALSE);
        }
    }

    MovePiece(to, from, pos);

    /* Restore captured piece */
    int captured = CAPTURED(move);
    if (captured != EMPTY) AddPiece(to, pos, captured);

    /* Undo promotion */
    int prPce = PROMOTED(move);
    if (prPce != EMPTY) {
        ClearPiece(from, pos);
        AddPiece(from, pos, (PieceCol[prPce] == WHITE) ? wP : bP);
    }
}
