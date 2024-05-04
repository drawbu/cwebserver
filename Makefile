# ↓ Basic variables
CC ?= gcc
CFLAGS := -std=gnu11
CFLAGS += -W -Wall -Wextra -Wpedantic
CFLAGS += -Wundef -Wshadow -Wcast-align -Wunused
CFLAGS += -Wstrict-prototypes -Wmissing-prototypes
CFLAGS += -Waggregate-return -Wcast-qual
CFLAGS += -Wunreachable-code
CFLAGS += -U_FORTIFY_SOURCE
CFLAGS += -iquote ./src

LDLIBS := -pthread
LDFLAGS :=

# ↓ Binaries
NAME ?= server
TEST_NAME := unit_tests
ASAN_NAME := asan
PROF_NAME := prof

# Source files
SRC := $(shell find ./src -name '*.c')

# Tests files
TEST_SRC := $(subst ./src/main.c,,$(SRC))
TEST_SRC += $(shell find ./tests -name '*.c')

# ↓ Objects
BUILD_DIR := .build
OBJ := $(SRC:%.c=$(BUILD_DIR)/source/%.o)
TEST_OBJ := $(TEST_SRC:%.c=$(BUILD_DIR)/tests/%.o)
ASAN_OBJ := $(SRC:%.c=$(BUILD_DIR)/asan/%.o)
PROF_OBJ := $(SRC:%.c=$(BUILD_DIR)/prof/%.o)

# ↓ Dependencies for headers
DEPS_FLAGS := -MMD -MP
DEPS := $(OBJ:.o=.d)
TEST_DEPS := $(TEST_OBJ:.o=.d)
ASAN_DEPS := $(ASAN_OBJ:.o=.d)
PROF_DEPS := $(PROF_OBJ:.o=.d)

# ↓ Colors
ECHO := echo -e
C_RESET := \033[00m
C_BOLD := \e[1m
C_RED := \e[31m
C_GREEN := \e[32m
C_YELLOW := \e[33m
C_BLUE := \e[34m
C_PURPLE := \e[35m
C_CYAN := \e[36m

.DEFAULT_GOAL := all
.PHONY: all
all: $(NAME)

# ↓ Compiling
$(BUILD_DIR)/source/%.o: %.c
	@ mkdir -p $(dir $@)
	@ $(ECHO) "[${C_BOLD}${C_RED}CC${C_RESET}] $^"
	@ $(CC) -o $@ -c $< $(CFLAGS) $(LDLIBS) $(DEPS_FLAGS)

$(NAME): CFLAGS += -flto
$(NAME): $(OBJ)
	@ $(ECHO) "[${C_BOLD}${C_YELLOW}CC${C_RESET}] ${C_GREEN}$@${C_RESET}"
	@ $(CC) -o $@ $^ $(CFLAGS) $(LDLIBS) $(LDFLAGS)

# ↓ Unit tests
$(BUILD_DIR)/tests/%.o: %.c
	@ mkdir -p $(dir $@)
	@ $(ECHO) "[${C_BOLD}${C_RED}CC${C_RESET}] $^"
	@ $(CC) -o $@ -c $< $(CFLAGS) $(LDLIBS) $(DEPS_FLAGS)

ifneq ($(NO_COV), 1)
$(TEST_NAME): CFLAGS += -g3
$(TEST_NAME): LDLIBS += --coverage
endif
$(TEST_NAME): LDLIBS += -lcriterion
$(TEST_NAME): $(TEST_OBJ)
	@ $(ECHO) "[${C_BOLD}${C_YELLOW}CC${C_RESET}] ${C_GREEN}$@${C_RESET}"
	@ $(CC) -o $@ $^ $(CFLAGS) $(LDLIBS) $(LDFLAGS)

.PHONY: tests_run
tests_run: $(TEST_NAME)
	@-./$<

# ↓ Asan
$(BUILD_DIR)/asan/%.o: %.c
	@ mkdir -p $(dir $@)
	@ $(ECHO) "[${C_BOLD}${C_RED}CC${C_RESET}] $^"
	@ $(CC) -o $@ -c $< $(CFLAGS) $(LDLIBS) $(DEPS_FLAGS)

$(ASAN_NAME): CFLAGS += -fsanitize=address,leak,undefined -g3
$(ASAN_NAME): CFLAGS += -fanalyzer
$(ASAN_NAME): CFLAGS += -D DEBUG_MODE
$(ASAN_NAME): $(ASAN_OBJ)
	@ $(ECHO) "[${C_BOLD}${C_YELLOW}CC${C_RESET}] ${C_GREEN}$@${C_RESET}"
	@ $(CC) -o $@ $^ $(CFLAGS) $(LDLIBS) $(LDFLAGS)

# ↓ Profiler
$(BUILD_DIR)/prof/%.o: %.c
	@ mkdir -p $(dir $@)
	@ $(ECHO) "[${C_BOLD}${C_RED}CC${C_RESET}] $^"
	@ $(CC) -o $@ -c $< $(CFLAGS) $(LDLIBS) $(DEPS_FLAGS)

$(PROF_NAME): CFLAGS += -pg
$(PROF_NAME): $(PROF_OBJ)
	@ $(ECHO) "[${C_BOLD}${C_YELLOW}CC${C_RESET}] ${C_GREEN}$@${C_RESET}"
	@ $(CC) -o $@ $^ $(CFLAGS) $(LDLIBS) $(LDFLAGS)

# ↓ Coverage
.PHONY: cov
cov: GCOVR_FLAGS := --exclude bonus/
cov: GCOVR_FLAGS += --exclude tests/
cov:
	@ gcovr $(GCOVR_FLAGS)
	@ gcovr $(GCOVR_FLAGS) --branches

# ↓ Cleaning
.PHONY: clean
clean:
	@ $(RM) -r $(BUILD_DIR)

.PHONY: fclean
fclean: clean
	@ $(RM) $(NAME) $(TEST_NAME) $(ASAN_NAME) $(PROF_NAME)

.PHONY: re
.NOTPARALLEL: re
re: fclean all

-include $(DEPS)
-include $(TEST_DEPS)
-include $(ASAN_DEPS)
-include $(PROF_DEPS)
