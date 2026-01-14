CC = gcc
CFLAGS = -O3 -march=native -Wall -Wextra -Wpedantic \
  -Wshadow -Wvla -Wpointer-arith -fomit-frame-pointer

PROJECT_HOME = ..
INCLUDE_PATH = -I$(PROJECT_HOME) -I$(PROJECT_HOME)/include
CFLAGS += $(INCLUDE_PATH)

# Source files for reference implementation
SRCS_REF = sm4.c mode/sm4_ctr.c

# Optional: OpenSSL comparison
ifeq ($(TEST_WITH_OPENSSL),1)
CFLAGS += -DTEST_WITH_OPENSSL
LDFLAGS += -lcrypto
endif

# Optional: Sanitizers for debugging
ifeq ($(SANITIZER),1)
CFLAGS += -fsanitize=address -fsanitize=leak -fsanitize=undefined -g
LDFLAGS += -fsanitize=address -fsanitize=leak -fsanitize=undefined
endif

.PHONY: all clean test speed help

all: test speed

test: test/test_sm4
speed: test/test_speed

test/test_sm4: test/test_sm4.c $(SRCS_REF)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test/test_speed: test/test_speed.c $(SRCS_REF)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f test/test_sm4 test/test_speed

help:
	@echo "SM4 Build Targets:"
	@echo "  make test              - Build correctness tests"
	@echo "  make speed             - Build speed benchmark"
	@echo "  make all               - Build both"
	@echo "  make clean             - Remove binaries"
	@echo ""
	@echo "Options:"
	@echo "  TEST_WITH_OPENSSL=1    - Enable OpenSSL comparison"
	@echo "  SANITIZER=1            - Enable address/leak sanitizers"
	@echo ""
	@echo "Examples:"
	@echo "  make test TEST_WITH_OPENSSL=1"
	@echo "  make speed TEST_WITH_OPENSSL=1"
	@echo "  make all SANITIZER=1"
