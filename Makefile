# Makefile for CPU Scheduler Simulator

CC = gcc
CFLAGS = -Wall -g -Iinclude
LDFLAGS = -lncurses

SRC = src/scheduler.c src/algorithms.c src/metrics.c src/report.c src/gui_ncurses.c
BUILD_DIR = build

TESTS = test_fifo test_sjf test_stcf test_rr test_mlfq

all: $(BUILD_DIR)/scheduler

# Build the main scheduler
$(BUILD_DIR)/scheduler: $(SRC)
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SRC) -o $@ $(LDFLAGS)

# Build individual tests
$(BUILD_DIR)/%: tests/%.c src/algorithms.c src/metrics.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $^ -o $@

# Build all tests
tests: $(addprefix $(BUILD_DIR)/, $(TESTS))

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR) *.o

.PHONY: all tests clean

