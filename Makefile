CC ?= gcc
CFLAGS ?= -Wall -Wextra -Werror -std=c99 -O2
PYTHON ?= python3
BIN_DIR = bin
SRC_DIR = src
FIRMWARE_SRC = firmware/src
FIRMWARE_HEADERS = $(wildcard $(FIRMWARE_SRC)/*.h)

all: $(BIN_DIR)/libmbgfuzzy.so $(BIN_DIR)/mbg_inspect

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BIN_DIR)/libmbgfuzzy.so: $(FIRMWARE_SRC)/mbg_fuzzy_core.c $(FIRMWARE_SRC)/mbg_profiles.c $(FIRMWARE_HEADERS) | $(BIN_DIR)
	$(CC) -shared -fPIC $(CFLAGS) $(filter %.c,$^) -lm -o $@

$(BIN_DIR)/mbg_inspect: $(SRC_DIR)/mbg_inspect_cli.c $(FIRMWARE_SRC)/mbg_fuzzy_core.c $(FIRMWARE_SRC)/mbg_profiles.c $(FIRMWARE_HEADERS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $(filter %.c,$^) -lm -o $@

$(BIN_DIR)/test_c_fuzzy_core: tests/test_c_fuzzy_core.c $(FIRMWARE_SRC)/mbg_fuzzy_core.c $(FIRMWARE_HEADERS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $(filter %.c,$^) -lm -o $@

$(BIN_DIR)/test_c_profiles_and_calibration: tests/test_c_profiles_and_calibration.c $(FIRMWARE_SRC)/mbg_fuzzy_core.c $(FIRMWARE_SRC)/mbg_profiles.c $(FIRMWARE_SRC)/mbg_calibration.c $(FIRMWARE_HEADERS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $(filter %.c,$^) -lm -o $@

clean:
	rm -rf $(BIN_DIR)

test: all $(BIN_DIR)/test_c_fuzzy_core $(BIN_DIR)/test_c_profiles_and_calibration
	./$(BIN_DIR)/test_c_fuzzy_core
	./$(BIN_DIR)/test_c_profiles_and_calibration
	$(PYTHON) -m pytest tests/ -v

.PHONY: all clean test
