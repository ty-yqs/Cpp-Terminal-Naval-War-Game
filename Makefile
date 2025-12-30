# Compiler settings
CXX = g++
CXXFLAGS = -std=c++17 -Wall -g
LDFLAGS = -lncurses

# Detect Operating System
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	# macOS usually uses clang++
	CXX = clang++
endif

# Directories
OBJ_DIR = obj

# Files
SRCS = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp, $(OBJ_DIR)/%.o, $(SRCS))
TARGET = game

# Rules
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(TARGET)

.PHONY: all clean
