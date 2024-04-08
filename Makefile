# ↓ Basic variables
CC := gcc
CFLAGS := -std=gnu11 -W -Wall -Wextra -Wunused -Wpedantic
CFLAGS += -Wundef -Wshadow -Wcast-align
CFLAGS += -Wstrict-prototypes -Wmissing-prototypes
CFLAGS += -Waggregate-return -Wcast-qual
CFLAGS += -Wunreachable-code
CFLAGS += -U_FORTIFY_SOURCE
CFLAGS += -iquote ./src
LDFLAGS :=

# ↓ Binaries
NAME := server
TEST_NAME := unit_tests
ASAN_NAME := asan
PROF_NAME := prof

# Source files
SRC := $(shell find ./src -name '*.c')

# Tests files
VPATH += tests
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

DIE := exit 1
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

.PHONY: all
all: $(NAME)

# ↓ Compiling
$(BUILD_DIR)/source/%.o: %.c
	@ mkdir -p $(dir $@)
	@ $(ECHO) "[${C_BOLD}${C_RED}CC${C_RESET}] $^"
	@ $(CC) -o $@ -c $< $(LDFLAGS) $(CFLAGS) $(DEPS_FLAGS) || $(DIE)

$(NAME): $(OBJ)
	@ $(ECHO) "[${C_BOLD}${C_YELLOW}CC${C_RESET}] ${C_GREEN}$@${C_RESET}"
	@ $(CC) -o $@ $^ $(LDFLAGS) || $(DIE)

# ↓ Unit tests
$(BUILD_DIR)/tests/%.o: %.c
	@ mkdir -p $(dir $@)
	@ $(ECHO) "[${C_BOLD}${C_RED}CC${C_RESET}] $^"
	@ $(CC) -o $@ -c $< $(LDFLAGS) $(CFLAGS) $(DEPS_FLAGS) || $(DIE)

ifneq ($(NO_COV), 1)
$(TEST_NAME): LDFLAGS += -g3 --coverage
endif
$(TEST_NAME): LDFLAGS += -lcriterion
$(TEST_NAME): $(TEST_OBJ)
	@ $(ECHO) "[${C_BOLD}${C_YELLOW}CC${C_RESET}] ${C_GREEN}$@${C_RESET}"
	@ $(CC) -o $@ $^ $(LDFLAGS) || $(DIE)

.PHONY: tests_run
tests_run: $(TEST_NAME)
	@-./$<

# ↓ Asan
$(BUILD_DIR)/asan/%.o: %.c
	@ mkdir -p $(dir $@)
	@ $(ECHO) "[${C_BOLD}${C_RED}CC${C_RESET}] $^"
	@ $(CC) -o $@ -c $< $(LDFLAGS) $(CFLAGS) $(DEPS_FLAGS) || $(DIE)

$(ASAN_NAME): LDFLAGS += -fsanitize=address,leak,undefined -g3
$(ASAN_NAME): LDFLAGS += -fanalyzer
$(ASAN_NAME): CFLAGS += -D DEBUG_MODE
$(ASAN_NAME): $(ASAN_OBJ)
	@ $(ECHO) "[${C_BOLD}${C_YELLOW}CC${C_RESET}] ${C_GREEN}$@${C_RESET}"
	@ $(CC) -o $@ $^ $(LDFLAGS) || $(DIE)

# ↓ Profiler
$(BUILD_DIR)/prof/%.o: %.c
	@ mkdir -p $(dir $@)
	@ $(ECHO) "[${C_BOLD}${C_RED}CC${C_RESET}] $^"
	@ $(CC) -o $@ -c $< $(LDFLAGS) $(CFLAGS) $(DEPS_FLAGS) || $(DIE)

$(PROF_NAME): LDFLAGS += -pg
$(PROF_NAME): $(PROF_OBJ)
	@ $(ECHO) "[${C_BOLD}${C_YELLOW}CC${C_RESET}] ${C_GREEN}$@${C_RESET}"
	@ $(CC) -o $@ $^ $(LDFLAGS) || $(DIE)

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
