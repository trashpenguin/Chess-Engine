#include "defs.h"
#include "stdlib.h"

#define RAND_64 ( (U64)rand() | \
                  (U64)rand() << 15 | \
                  (U64)rand() << 30 | \
                  (U64)rand() << 45 | \
                  ((U64)rand() & 0xf) << 60 )

int Sq120ToSq64[BRD_SQ_NUM];
int Sq64ToSq120[64];

U64 SetMask[64];
U64 ClearMask[64];

U64 PieceKeys[13][120];
U64 SideKeys;
U64 CastleKeys[16];

int FilesBrd[BRD_SQ_NUM];
int RanksBrd[BRD_SQ_NUM];

static void InitFilesRanksBrd(void) {
    int index, file, rank, sq;

    for (index = 0; index < BRD_SQ_NUM; ++index) {
        FilesBrd[index] = OFFBOARD;
        RanksBrd[index] = OFFBOARD;
    }
    for (rank = RANK_1; rank <= RANK_8; ++rank) {
        for (file = FILE_A; file <= FILE_H; ++file) {
            sq = FR2SQ(file, rank);
            FilesBrd[sq] = file;
            RanksBrd[sq] = rank;
        }
    }
}

static void InitHashKeys(void) {
    int index, index2;
    for (index = 0; index < 13; ++index)
        for (index2 = 0; index2 < 120; ++index2)
            PieceKeys[index][index2] = RAND_64;
    SideKeys = RAND_64;
    for (index = 0; index < 16; ++index)
        CastleKeys[index] = RAND_64;
}

static void InitBitMask(void) {
    int index;
    for (index = 0; index < 64; index++) {
        SetMask[index]  = (1ULL << index);   /* was 0ULL — fixed */
        ClearMask[index] = ~SetMask[index];
    }
}

static void InitSq120To64(void) {
    int index, file, rank, sq, sq64 = 0;

    for (index = 0; index < BRD_SQ_NUM; ++index) Sq120ToSq64[index] = 65;
    for (index = 0; index < 64; ++index)          Sq64ToSq120[index] = 120;

    for (rank = RANK_1; rank <= RANK_8; ++rank) {
        for (file = FILE_A; file <= FILE_H; ++file) {
            sq = FR2SQ(file, rank);
            Sq64ToSq120[sq64] = sq;
            Sq120ToSq64[sq]   = sq64;
            sq64++;
        }
    }
}

void AllInit(void) {
    InitSq120To64();
    InitBitMask();
    InitHashKeys();
    InitFilesRanksBrd();
    InitMvvLva();
}
