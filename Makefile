# --- compiler & flags ---
CXX      := clang++
CXXFLAGS := -std=c++20 -O3 -march=native -Wall -Wextra -Wpedantic -Wconversion

# --- include paths (project + Eigen via Homebrew) ---
EIGEN_PREFIX := $(shell brew --prefix eigen 2>/dev/null)
ifeq ($(EIGEN_PREFIX),)
  EIGEN_INC ?= -I/opt/homebrew/include/eigen3 -I/usr/local/include/eigen3
else
  EIGEN_INC := -I$(EIGEN_PREFIX)/include/eigen3
endif

# Project includes (and third-party headers such as nlohmann/json under include/)
INCLUDES := -Iinclude $(EIGEN_INC)

# --- dirs ---
SRC_DIR  := src
TOOL_DIR := tools
BUILD    := build

# --- library objects ---
LIB_OBJ  := \
  $(BUILD)/IS.o \
  $(BUILD)/Impact.o \
  $(BUILD)/IO.o \
  $(BUILD)/Optimize.o

LIB_A    := $(BUILD)/libtca.a

# --- tools / binaries ---
TOOL_IS   := $(BUILD)/tca_is
TOOL_FIT  := $(BUILD)/tca_fit_impact
TOOL_OPT  := $(BUILD)/tca_optimize

# --- default ---
all: $(TOOL_IS) $(TOOL_FIT) $(TOOL_OPT)

# --- library objects ---
$(BUILD)/IS.o: $(SRC_DIR)/IS.cpp include/tca/IS.hpp include/tca/Types.hpp include/tca/Market.hpp
	@mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD)/Impact.o: $(SRC_DIR)/Impact.cpp include/tca/Impact.hpp include/tca/Types.hpp include/tca/Market.hpp
	@mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD)/IO.o: $(SRC_DIR)/IO.cpp include/tca/IO.hpp include/tca/Types.hpp include/tca/Utils.hpp
	@mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD)/Optimize.o: $(SRC_DIR)/Optimize.cpp include/tca/Optimize.hpp include/tca/Types.hpp include/tca/Impact.hpp
	@mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# --- static library ---
$(LIB_A): $(LIB_OBJ)
	@mkdir -p $(BUILD)
	ar rcs $(LIB_A) $(LIB_OBJ)

# --- tools ---
$(TOOL_IS): $(TOOL_DIR)/tca_is.cpp $(LIB_A) include/tca/IO.hpp include/tca/IS.hpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(LIB_A) -o $@

$(TOOL_FIT): $(TOOL_DIR)/tca_fit_impact.cpp $(LIB_A) include/tca/Impact.hpp include/tca/IO.hpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(LIB_A) -o $@

# JSON-based optimizer CLI (uses nlohmann/json single header at include/nlohmann/json.hpp)
$(TOOL_OPT): $(TOOL_DIR)/tca_optimize.cpp $(LIB_A) include/nlohmann/json.hpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(LIB_A) -o $@

# --- convenience run targets ---
run_is:
	$(TOOL_IS) --fills data/fills.csv --mkt data/mkt.csv --arrival 10.00

run_fit:
	$(TOOL_FIT) --fills data/fills.csv --mkt data/mkt.csv

# Requires order.json, impact.json at project root and mkt.csv with N rows in data/
run_opt:
	$(TOOL_OPT) --order order.json --mkt data/mkt.csv --impact impact.json --out schedule.csv

clean:
	rm -rf $(BUILD)

.PHONY: all clean run_is run_fit run_opt
