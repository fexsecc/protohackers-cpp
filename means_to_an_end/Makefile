CXX := g++
CXXFLAGS := -std=c++23 -g -Wall -Wextra -MMD -MP
CHALL := means_to_an_end

SRCS := main.cpp
BUILD_DIR := ../build/$(CHALL)
OBJ_DIR := $(BUILD_DIR)/obj

TARGET := $(BUILD_DIR)/main
OBJS := $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

.PHONY: all clean

all: $(TARGET)

# Link
$(TARGET): $(OBJS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile (ensure object dir exists)
$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

-include $(DEPS)

clean:
	rm -rf $(BUILD_DIR)

