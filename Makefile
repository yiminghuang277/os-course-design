CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Iinclude -pthread
TARGET = os_course_design

SRC = \
	src/main.c \
	src/common.c \
	src/scheduler.c \
	src/memory.c \
	src/sync_demo.c \
	src/filesystem.c \
	src/extension.c

OBJ = $(SRC:.c=.o)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

clean:
	rm -f $(OBJ) $(TARGET)

run: $(TARGET)
	./$(TARGET)
