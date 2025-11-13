# SPDX-License-Identifier: MIT
# Author:  Giovanni Santini
# Mail:    giovanni.santini@proton.me
# License: MIT

#
# Compiler flags
#
CFLAGS      = -Wall -Werror -Wextra -Wpedantic -std=c99
DEBUG_FLAGS = -ggdb
LDFLAGS     =
CC?         = gcc

#
# Project files
#
OUT_NAME  = example
OBJ       = example.o
TEST_NAME = test
TEST_OBJ  = test.o

#
# Commands
#
all: $(OUT_NAME)

debug: CFLAGS += $(DEBUG_FLAGS)
debug: $(OUT_NAME)

run: $(OUT_NAME)
	chmod +x $(OUT_NAME)
	./$(OUT_NAME)

check: $(TEST_NAME)
	chmod +x $(TEST_NAME)
	./$(TEST_NAME)

clean:
	rm -f $(OBJ)

distclean:
	rm -f $(OUT_NAME)

$(OUT_NAME): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) $(CFLAGS) -o $(OUT_NAME)

$(TEST_NAME): $(TEST_OBJ)
	$(CC) $(TEST_OBJ) $(LDFLAGS) $(CFLAGS) -o $(TEST_NAME)

%.o: %pp.c
	$(CC) $(CFLAGS) -c $< -o $@
