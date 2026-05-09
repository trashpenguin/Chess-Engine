#include "stdio.h"
#include "defs.h"

int main(void) {
    AllInit();

    S_BOARD pos[1];
    S_SEARCHINFO info[1];

    pos->PvTable->pTable = NULL;
    InitPvTable(pos->PvTable);

    info->quit = FALSE;
    Uci_Loop(pos, info);

    free(pos->PvTable->pTable);
    return 0;
}
