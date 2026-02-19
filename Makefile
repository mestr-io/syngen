CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -Iinclude -Isrc -Ivendor/cJSON
LDFLAGS = -lm

SRC_DIR = src
VENDOR_DIR = vendor
OBJ_DIR = obj
BIN_DIR = bin

SRCS = $(SRC_DIR)/main.c $(SRC_DIR)/faker/faker.c $(SRC_DIR)/generator/generator.c $(SRC_DIR)/export/export_manager.c $(VENDOR_DIR)/cJSON/cJSON.c
OBJS = $(patsubst %.c,$(OBJ_DIR)/%.o,$(SRCS))

TARGET = $(BIN_DIR)/syngen

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TARGET)
	./tests/test.sh

lint:
	@if command -v cppcheck >/dev/null 2>&1; then \
		echo "Running cppcheck..."; \
		cppcheck --enable=all --suppress=missingIncludeSystem --suppress=unusedFunction --error-exitcode=1 -Iinclude -Isrc -Ivendor/cJSON $(SRC_DIR); \
	else \
		echo "cppcheck not found. Running strict compiler checks..."; \
		$(CC) $(CFLAGS) -Werror -pedantic -Wconversion -Wshadow -fsyntax-only $(SRCS); \
	fi

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean test lint
