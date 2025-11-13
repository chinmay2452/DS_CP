CXX := clang++
CXXFLAGS := -std=c++17 -fPIC -O2
LDFLAGS := -shared

# Prefer src/, fall back to ../src/
SRC_DIR := src
ALT_SRC := ../src

# All cpp files but EXCLUDE main.cpp
SRC_FILES := $(filter-out $(SRC_DIR)/main.cpp, $(wildcard $(SRC_DIR)/*.cpp))

ifeq ($(strip $(SRC_FILES)),)
  SRC_DIR := $(ALT_SRC)
  SRC_FILES := $(filter-out $(SRC_DIR)/main.cpp, $(wildcard $(SRC_DIR)/*.cpp))
endif

# Build folders
BUILD_DIR := build
OBJ_DIR := $(BUILD_DIR)/obj
LIB_NAME := libfriend
LIB_TARGET := $(BUILD_DIR)/$(LIB_NAME).dylib

# Object file list
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES))

.PHONY: all clean dirs print-sources

all: dirs $(LIB_TARGET)
	@echo "Built $(LIB_TARGET)"

dirs:
	@mkdir -p $(BUILD_DIR) $(OBJ_DIR)

# Compile .cpp → .o and include headers (.hpp)
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -I$(SRC_DIR) -c $< -o $@

# Link all .o → .dylib
$(LIB_TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS)

clean:
	rm -rf $(BUILD_DIR)

print-sources:
	@echo "Using SRC_DIR=$(SRC_DIR)"
	@echo "Sources:"
	@echo $(SRC_FILES)