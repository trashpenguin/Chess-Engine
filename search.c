#include "stdio.h"
#include "defs.h"

/* ===== PV table ===== */

void InitPvTable(S_PVTABLE *table) {
    table->numEntries = PV_SIZE / sizeof(S_PVENTRY);
    if (table->pTable != NULL) free(table->pTable);
    table->pTable = (S_PVENTRY *)malloc(table->numEntries * sizeof(S_PVENTRY));
    ClearPvTable(table);
}

void ClearPvTable(S_PVTABLE *table) {
    S_PVENTRY *entry;
    for (entry = table->pTable; entry < table->pTable + table->numEntries; entry++) {
        entry->posKey = 0ULL;
        entry->move   = NOMOVE;
    }
}

void StorePvMove(const S_BOARD *pos, const int move) {
    int index = pos->posKey % pos->PvTable->numEntries;
    pos->PvTable->pTable[index].posKey = pos->posKey;
    pos->PvTable->pTable[index].move   = move;
}

int ProbePvTable(const S_BOARD *pos) {
    int index = pos->posKey % pos->PvTable->numEntries;
    if (pos->PvTable->pTable[index].posKey == pos->posKey)
        return pos->PvTable->pTable[index].move;
    return NOMOVE;
}

int GetPvLine(const int depth, S_BOARD *pos) {
    int move  = ProbePvTable(pos);
    int count = 0;
    while (move != NOMOVE && count < depth) {
        if (MoveExists(pos, move)) {
            MakeMove(pos, move);
            pos->PvArray[count++] = move;
        } else {
            break;
        }
        move = ProbePvTable(pos);
    }
    while (pos->ply > 0) TakeMove(pos);
    return count;
}

/* ===== Search helpers ===== */

static int IsRepetition(const S_BOARD *pos) {
    int index;
    for (index = pos->hisply - pos->fiftyMove; index < pos->hisply - 1; ++index) {
        ASSERT(index >= 0 && index < MAXGAMEMOVES);
        if (pos->posKey == pos->history[index].posKey) return TRUE;
    }
    return FALSE;
}

static void CheckUp(S_SEARCHINFO *info) {
    if (info->timeSet && GetTimeMs() > info->stopTime)
        info->stopped = TRUE;
    ReadInput(info);
}

static void PickNextMove(int moveNum, S_MOVELIST *list) {
    int index, bestScore = -1, bestNum = moveNum;
    S_MOVE temp;
    for (index = moveNum; index < list->count; ++index) {
        if (list->moves[index].score > bestScore) {
            bestScore = list->moves[index].score;
            bestNum   = index;
        }
    }
    temp                   = list->moves[moveNum];
    list->moves[moveNum]   = list->moves[bestNum];
    list->moves[bestNum]   = temp;
}

static void ScoreMoves(S_MOVELIST *list, S_BOARD *pos, int pvMove) {
    int index;
    for (index = 0; index < list->count; ++index) {
        int move = list->moves[index].move;
        if (move == pvMove) {
            list->moves[index].score = 2000000;
        } else if (CAPTURED(move) != EMPTY) {
            list->moves[index].score =
                1000000 + MvvLvaScores[CAPTURED(move)][pos->pieces[FROMSQ(move)]];
        } else {
            if (pos->searchKillers[0][pos->ply] == move)
                list->moves[index].score = 900000;
            else if (pos->searchKillers[1][pos->ply] == move)
                list->moves[index].score = 800000;
            else
                list->moves[index].score =
                    pos->searchHistory[pos->pieces[FROMSQ(move)]][TOSQ(move)];
        }
    }
}

static void ClearForSearch(S_BOARD *pos, S_SEARCHINFO *info) {
    int i, j;
    for (i = 0; i < 13; ++i)
        for (j = 0; j < BRD_SQ_NUM; ++j)
            pos->searchHistory[i][j] = 0;
    for (i = 0; i < 2; ++i)
        for (j = 0; j < MAXDEPTH; ++j)
            pos->searchKillers[i][j] = 0;
    ClearPvTable(pos->PvTable);
    pos->ply       = 0;
    info->stopped  = FALSE;
    info->nodes    = 0LL;
    info->fh       = 0;
    info->fhf      = 0;
}

/* ===== Quiescence search ===== */

static int Quiescence(int alpha, int beta, S_BOARD *pos, S_SEARCHINFO *info) {
    if ((info->nodes & 2047) == 0) CheckUp(info);
    info->nodes++;

    if (IsRepetition(pos) || pos->fiftyMove >= 100) return 0;
    if (pos->ply >= MAXDEPTH) return EvalPosition(pos);

    int score = EvalPosition(pos);
    if (score >= beta) return beta;
    if (score > alpha) alpha = score;

    S_MOVELIST list[1];
    GenerateAllCaps(pos, list);

    int moveNum, legal = 0;
    for (moveNum = 0; moveNum < list->count; ++moveNum) {
        PickNextMove(moveNum, list);
        if (!MakeMove(pos, list->moves[moveNum].move)) continue;
        legal++;
        score = -Quiescence(-beta, -alpha, pos, info);
        TakeMove(pos);

        if (info->stopped) return 0;
        if (score > alpha) {
            if (score >= beta) return beta;
            alpha = score;
        }
    }
    return alpha;
}

/* ===== Alpha-beta ===== */

static int AlphaBeta(int alpha, int beta, int depth, S_BOARD *pos,
                     S_SEARCHINFO *info) {
    if (depth == 0) return Quiescence(alpha, beta, pos, info);

    if ((info->nodes & 2047) == 0) CheckUp(info);
    info->nodes++;

    if ((IsRepetition(pos) || pos->fiftyMove >= 100) && pos->ply) return 0;
    if (pos->ply >= MAXDEPTH) return EvalPosition(pos);

    /* Check extension */
    int inCheck = SqAttacked(pos->KingSq[pos->side], pos->side ^ 1, pos);
    if (inCheck) depth++;

    S_MOVELIST list[1];
    GenerateAllMoves(pos, list);

    int pvMove    = ProbePvTable(pos);
    int moveNum, legal = 0, oldAlpha = alpha;
    int bestMove  = NOMOVE;
    int score;

    ScoreMoves(list, pos, pvMove);

    for (moveNum = 0; moveNum < list->count; ++moveNum) {
        PickNextMove(moveNum, list);
        if (!MakeMove(pos, list->moves[moveNum].move)) continue;
        legal++;

        score = -AlphaBeta(-beta, -alpha, depth - 1, pos, info);
        TakeMove(pos);

        if (info->stopped) return 0;

        if (score > alpha) {
            if (score >= beta) {
                if (legal == 1) info->fhf++;
                info->fh++;
                /* Beta cutoff: store killer for quiet moves */
                if (!(list->moves[moveNum].move & MFLAGCAP)) {
                    pos->searchKillers[1][pos->ply] = pos->searchKillers[0][pos->ply];
                    pos->searchKillers[0][pos->ply] = list->moves[moveNum].move;
                }
                StorePvMove(pos, list->moves[moveNum].move);
                return beta;
            }
            alpha    = score;
            bestMove = list->moves[moveNum].move;
            /* Update history for quiet moves */
            if (!(bestMove & MFLAGCAP))
                pos->searchHistory[pos->pieces[FROMSQ(bestMove)]][TOSQ(bestMove)] += depth;
        }
    }

    /* Mate or stalemate */
    if (legal == 0) {
        if (inCheck) return -INFINITE + pos->ply;  /* checkmate */
        else         return 0;                       /* stalemate */
    }

    if (alpha != oldAlpha) StorePvMove(pos, bestMove);
    return alpha;
}

/* ===== Iterative deepening root ===== */

void SearchPosition(S_BOARD *pos, S_SEARCHINFO *info) {
    int bestMove  = NOMOVE;
    int bestScore = -INFINITE;
    int pvMoves, pvNum, currentDepth;

    ClearForSearch(pos, info);

    for (currentDepth = 1; currentDepth <= info->depth; ++currentDepth) {
        bestScore = AlphaBeta(-INFINITE, INFINITE, currentDepth, pos, info);
        if (info->stopped) break;

        pvMoves  = GetPvLine(currentDepth, pos);
        bestMove = pos->PvArray[0];

        printf("info score cp %d depth %d nodes %lld time %d pv",
               bestScore, currentDepth, info->nodes,
               GetTimeMs() - info->startTime);
        for (pvNum = 0; pvNum < pvMoves; pvNum++)
            printf(" %s", PrMove(pos->PvArray[pvNum]));
        printf("\n");
        fflush(stdout);
    }

    printf("bestmove %s\n", PrMove(bestMove));
    fflush(stdout);
}
