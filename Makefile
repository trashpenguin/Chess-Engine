CC     = gcc
CFLAGS = -Wall -Wextra -O2
SRCS   = vice.c init.c bitboard.c hushkeys.c board.c data.c \
         attack.c movegen.c makemove.c evaluate.c search.c misc.c uci.c
TARGET = vice

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

clean:
	del /f $(TARGET).exe
