CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude

SRC_DIR = src
OBJ_DIR = obj
INC_DIR = include

SOURCES = $(SRC_DIR)/QueueSystem.cpp $(SRC_DIR)/Benchmark.cpp $(SRC_DIR)/main.cpp
OBJECTS = $(OBJ_DIR)/QueueSystem.o $(OBJ_DIR)/Benchmark.o $(OBJ_DIR)/main.o
TARGET = hospital_queue

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR) $(TARGET)

run: all
	./$(TARGET)

.PHONY: all clean run
