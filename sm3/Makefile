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

# Optional: OpenSSL comparison
ifeq ($(TEST_WITH_OPENSSL), 1)
CFLAGS += -DTEST_WITH_OPENSSL
LDFLAGS += -lcrypto
endif

.PHONY: all clean lib test speed help
all: lib test speed
lib: libsm3_x64_linux.so
test: test/test_sm3
speed: test/test_speed

libsm3_x64_linux.so:$(SRCS)
	$(CC) -o $@ $^ -shared $(CFLAGS) -fPIC

test/test_speed: test/test_speed.c $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
test/test_sm3: test/test_sm3.c utils.c $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f libsm3_x64_linux.so
	rm -f test/test_sm3
	rm -f test/test_speed

help:
	@echo "SM3 Build Targets:"
	@echo "  make all               - Build library, test, and speed benchmark"
	@echo "  make lib               - Build libsm3_x64_linux.so"
	@echo "  make test              - Build correctness tests"
	@echo "  make speed             - Build speed benchmark"
	@echo "  make clean             - Remove binaries"
	@echo ""
	@echo "Options:"
	@echo "  DEBUG=1                - Enable debug mode"
	@echo "  SANITIZER=1            - Enable address/leak sanitizers"
	@echo "  TEST_WITH_OPENSSL=1    - Enable OpenSSL comparison"
	@echo ""
	@echo "Examples:"
	@echo "  make test TEST_WITH_OPENSSL=1"
	@echo "  make all SANITIZER=1"
