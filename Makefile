CC      = gcc
CFLAGS  = -std=c11 -Wall -Wextra -g -pthread -Iinclude
LDFLAGS = -pthread
SRC     = $(wildcard src/*.c)
OBJ     = $(SRC:src/%.c=build/%.o)
TARGET  = proxy

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build:
	mkdir -p build

# Build with ThreadSanitizer to find data races
tsan: CFLAGS += -fsanitize=thread
tsan: LDFLAGS += -fsanitize=thread
tsan: clean $(TARGET)

clean:
	rm -rf build $(TARGET)

.PHONY: all clean tsan
