# Flag
CC = gcc
CFLAGS = -O3 -march=native -Wall -Wextra -Wpedantic \
  -Wshadow -Wvla -Wpointer-arith -Wa,--noexecstack -fomit-frame-pointer

PROJECT_HOME=..
INCLUDE_PATH=-I$(PROJECT_HOME) -I$(PROJECT_HOME)/include
CFLAGS += $(INCLUDE_PATH)

# Files
SRCS = sm3.c

# Deafult option: release
# For debug option, USAGE: make /f Makefile2 DEBUG=1
ifeq ($(DEBUG), 1)
	# Debug mode, Rewrite the CFLAG and LFLAGS
	CFLAGS = -Wall -DDEBUG -g 
endif

ifeq ($(SANITIZER), 1)
CFLAGS += -fsanitize=address -fsanitize=leak -fsanitize=undefined -ftrapv -fstack-protector -g
endif

.PHONY: all clean lib test speed
all: lib test speed
lib: libsm3_x64_linux.so
test: test/test_sm3
speed: test/test_speed

libsm3_x64_linux.so:$(SRCS)
	$(CC) -o $@ $^ -shared $(CFLAGS) -fPIC

test/test_speed: test/test_speed.c $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^
test/test_sm3: test/test_sm3.c utils.c $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f libsm3_x64_linux.so
	rm -f test/test_sm3
	rm -f test/test_speed
