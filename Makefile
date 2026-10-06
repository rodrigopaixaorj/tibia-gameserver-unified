CXX ?= g++
CC ?= gcc
CXXFLAGS ?= -O2 -std=c++17 -Wall -Wextra -Isrc -Isrc/compat -Isrc/crypto -Isrc/database -Isrc/login -Isrc/game
CFLAGS ?= -O2 -Isrc/database
LDFLAGS ?= -lpthread -ldl

SRCS_CXX := $(wildcard src/compat/*.cc src/crypto/*.cc src/database/*.cc src/login/*.cc src/game/*.cc src/*.cc)
SRCS_C := $(wildcard src/database/*.c)

OBJS := $(SRCS_CXX:.cc=.o) $(SRCS_C:.c=.o)
TARGET := tibia-server

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
