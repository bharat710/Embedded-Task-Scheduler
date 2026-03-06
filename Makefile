# ─────────────────────────────────────────────────────────────
#  Embedded Task Scheduler — Makefile
# ─────────────────────────────────────────────────────────────

CC        = gcc
CFLAGS    = -Wall -Wextra -Wpedantic -std=c11
LDFLAGS   =

# Directories
SRC_DIR   = src
BUILD_DIR = build

# Source files (recursive wildcard on macOS-compatible approach)
SRCS      = $(SRC_DIR)/main.c \
            $(SRC_DIR)/scheduler/scheduler.c \
            $(SRC_DIR)/scheduler/sync.c \
            $(SRC_DIR)/timer/timer.c \
            $(SRC_DIR)/tasks/tasks.c

# Object files
OBJS      = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))

# Binary
TARGET    = $(BUILD_DIR)/embedded_scheduler

# ─── Targets ────────────────────────────────────────────────

.PHONY: all clean run run-rr run-priority help

all: $(TARGET)
	@echo "\n  ✅  Build successful → $(TARGET)\n"

$(TARGET): $(OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -c -o $@ $<

clean:
	rm -rf $(BUILD_DIR)
	@echo "  🧹  Build directory cleaned."

# ─── Run shortcuts ──────────────────────────────────────────

run: all
	@echo ""
	./$(TARGET) rr 40

run-rr: all
	@echo ""
	./$(TARGET) rr 40

run-priority: all
	@echo ""
	./$(TARGET) priority 40

# ─── Help ───────────────────────────────────────────────────

help:
	@echo ""
	@echo "  Embedded Task Scheduler — Build Targets"
	@echo "  ────────────────────────────────────────"
	@echo "  make            Build the project"
	@echo "  make run        Build & run (Round Robin, 40 ticks)"
	@echo "  make run-rr     Build & run (Round Robin, 40 ticks)"
	@echo "  make run-priority  Build & run (Priority, 40 ticks)"
	@echo "  make clean      Remove build artifacts"
	@echo "  make help       Show this help"
	@echo ""
