CC = clang
CFLAGS = -O3 -Wall -Wextra

TARGET = big_challenge
SRCS = src/main.c src/heap.c src/utils.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
.PHONY: all clean
