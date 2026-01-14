# Flag
CC = gcc
CFLAGS = -O3
LFLAGS = -O3

# Default option: release
# For debug option, USAGE: make DEBUG=1
ifeq ($(DEBUG), 1)
	# Debug mode, Rewrite the CFLAG and LFLAGS
	CFLAGS = -Wall -DDEBUG -g
	LFLAGS =
endif

CFLAGS += -Wno-strict-aliasing -Wno-missing-braces

# Portable build: no x64-specific flags
# Old x64 flags: -madx -mbmi2 -march=native
CFLAGS += -fPIC

LFLAGS += -fPIC

# Include paths
# -Iinclude: for sm2 headers
# -I..: for include/sm_interface.h (needed by sm3)
INCLUDE = -Iinclude -I..

CFLAGS += $(INCLUDE)

# Files - Pure C implementation (no assembly)
# Note: sm3 is now sourced from ../sm3/sm3.c
SM2_SOURCES = extra.c basicOp.c fieldOp.c ecc.c sm2.c utils.c ecc_montg.c ecc_basepoint_mul.c
SM3_SOURCE = ../sm3/sm3.c

COBJS=$(SM2_SOURCES:.c=.o)
COBJS := $(addprefix build/, $(COBJS))
SM3_OBJ = build/sm3.o

ALL_OBJS = $(COBJS) $(SM3_OBJ)

# Make sure build exist
$(shell mkdir -p build)

ifeq ($(SANITIZER), 1)
CFLAGS += -fsanitize=address -fsanitize=leak -fsanitize=undefined -ftrapv -fstack-protector -g
endif

ifeq ($(TEST_WITH_GMSSL), 1)
TESTFLAGS += -DTEST_WITH_GMSSL -I$(GMSSL_ROOT)/include -L$(GMSSL_ROOT)/build/bin
TESTLDFLAGS += -lgmssl
endif

ifeq ($(TEST_WITH_OPENSSL), 1)
TESTFLAGS += -DTEST_WITH_OPENSSL
TESTLDFLAGS += -lssl -lcrypto
endif

all: lib test speed

lib: libsm2.so
test: test/test_sm2
speed: test/test_speed

libsm2.so: $(ALL_OBJS)
	$(CC) $(LFLAGS) -shared -o $@ $(ALL_OBJS)

test/test_sm2: test/test_sm2.c $(ALL_OBJS)
	$(CC) $(CFLAGS) $(TESTFLAGS) -o $@ test/test_sm2.c $(ALL_OBJS) $(TESTLDFLAGS)

test/test_speed: test/test_speed.c $(ALL_OBJS)
	$(CC) $(CFLAGS) $(TESTFLAGS) -o $@ test/test_speed.c $(ALL_OBJS) $(TESTLDFLAGS)

# Compile SM2 sources
$(COBJS):build/%.o:%.c
	$(CC) -c $^ -o $@ $(CFLAGS)

# Compile SM3 source from sm3 directory
$(SM3_OBJ): $(SM3_SOURCE)
	$(CC) -c $^ -o $@ $(CFLAGS)

.PHONY: clean help
clean:
	rm -f test/test_sm2
	rm -f test/test_speed
	rm -f test/test_basicOp
	rm -f test/test_openssl
	rm -f test/debug_*
	rm -f libsm2.so
	rm -f build/*.o

help:
	@echo "SM2 Build Targets:"
	@echo "  make all               - Build library, test, and speed benchmark"
	@echo "  make lib               - Build libsm2.so"
	@echo "  make test              - Build correctness tests"
	@echo "  make speed             - Build speed benchmark"
	@echo "  make clean             - Remove binaries"
	@echo ""
	@echo "Options:"
	@echo "  DEBUG=1                - Enable debug mode"
	@echo "  SANITIZER=1            - Enable address/leak sanitizers"
	@echo "  TEST_WITH_OPENSSL=1    - Enable OpenSSL comparison"
	@echo "  TEST_WITH_GMSSL=1      - Enable GmSSL comparison (requires GMSSL_ROOT)"
	@echo ""
	@echo "Examples:"
	@echo "  make test TEST_WITH_OPENSSL=1"
	@echo "  make speed TEST_WITH_OPENSSL=1"
	@echo "  make all SANITIZER=1"
