# --- compiler & flags ---
CXX      := clang++
CXXFLAGS := -std=c++20 -O3 -march=native -Wall -Wextra -Wpedantic -Wconversion

# --- includes (Eigen via Homebrew + project + third-party headers) ---
EIGEN_PREFIX := $(shell brew --prefix eigen 2>/dev/null)
ifeq ($(EIGEN_PREFIX),)
  EIGEN_INC ?= -I/opt/homebrew/include/eigen3 -I/usr/local/include/eigen3
else
  EIGEN_INC := -I$(EIGEN_PREFIX)/include/eigen3
endif
INCLUDES := -Iinclude $(EIGEN_INC)

# --- dirs ---
SRC_DIR  := src
TOOL_DIR := tools
BUILD    := build

# --- library objects ---
LIB_OBJ := \
  $(BUILD)/IS.o \
  $(BUILD)/Impact.o \
  $(BUILD)/IO.o \
  $(BUILD)/Optimize.o \
  $(BUILD)/Report.o

LIB_A := $(BUILD)/libtca.a

# --- default ---
all: $(BUILD)/tca

# --- compile objects ---
$(BUILD)/IS.o: $(SRC_DIR)/IS.cpp include/tca/IS.hpp include/tca/Types.hpp include/tca/Market.hpp
	@mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD)/Impact.o: $(SRC_DIR)/Impact.cpp include/tca/Impact.hpp include/tca/Types.hpp include/tca/Market.hpp
	@mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD)/IO.o: $(SRC_DIR)/IO.cpp include/tca/IO.hpp include/tca/Types.hpp include/tca/Utils.hpp
	@mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD)/Optimize.o: $(SRC_DIR)/Optimize.cpp include/tca/Optimize.hpp include/tca/Impact.hpp include/tca/Types.hpp
	@mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD)/Report.o: $(SRC_DIR)/Report.cpp include/tca/Report.hpp include/tca/IS.hpp include/tca/Impact.hpp include/tca/Optimize.hpp include/nlohmann/json.hpp
	@mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# --- static library ---
$(LIB_A): $(LIB_OBJ)
	@mkdir -p $(BUILD)
	ar rcs $(LIB_A) $(LIB_OBJ)

# --- unified CLI ---
$(BUILD)/tca: $(TOOL_DIR)/tca.cpp $(LIB_A) include/nlohmann/json.hpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(LIB_A) -o $@

# --- convenience run targets (all inputs now in data/) ---
run_is:
	$(BUILD)/tca is --fills data/fills.csv --mkt data/mkt.csv --arrival 10.00

run_fit:
	$(BUILD)/tca fit-impact --fills data/fills.csv --mkt data/mkt.csv

run_opt:
	$(BUILD)/tca optimize --order data/order.json --mkt data/mkt.csv \
	  --impact data/impact.json --out schedule.csv

run_report:
	$(BUILD)/tca report --symbol TEST --fills data/fills.csv --mkt data/mkt.csv \
	  --arrival 10.00 --impact data/impact.json --order data/order.json \
	  --out report.json --sched schedule.csv --is is.csv

clean:
	rm -rf $(BUILD)

.PHONY: all clean run_is run_fit run_opt run_report
