CC = gcc
CFLAGS = -Wall -Wextra -pthread -Iinclude

TARGET = cachet

SRC = src/cachet.c \
      src/http.c \
      src/remote.c \
      src/cache.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

rebuild: clean all

.PHONY: all run clean rebuild