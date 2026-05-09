#include "stdio.h"
#include "string.h"
#include "defs.h"
#include "windows.h"

int GetTimeMs(void) {
    return (int)GetTickCount();
}

/* Poll stdin for a "stop" or "quit" command sent by a GUI during search */
void ReadInput(S_SEARCHINFO *info) {
    DWORD avail = 0;
    HANDLE h    = GetStdHandle(STD_INPUT_HANDLE);
    char buf[256];
    DWORD read;

    if (!PeekNamedPipe(h, NULL, 0, NULL, &avail, NULL)) {
        /* Running from a terminal — just ignore, rely on time management */
        return;
    }
    if (avail > 0) {
        ReadFile(h, buf, (avail < 255 ? avail : 255), &read, NULL);
        buf[read] = '\0';
        if (strstr(buf, "quit") || strstr(buf, "stop")) {
            info->stopped = TRUE;
            info->quit    = TRUE;
        }
    }
}
