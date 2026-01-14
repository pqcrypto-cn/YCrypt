# Flag
CC = gcc
CFLAGS = -O3
LFLAGS = -O3

# Deafult option: release
# For debug option, USAGE: make /f Makefile2 DEBUG=1
ifeq ($(DEBUG), 1)
	# Debug mode, Rewrite the CFLAG and LFLAGS
	CFLAGS = -Wall -DDEBUG -g
	LFLAGS =
endif

CFLAGS += -Wno-strict-aliasing -Wno-missing-braces
# -std=c99

# Portable build: no x64-specific flags
# Old x64 flags: -madx -mbmi2 -march=native
CFLAGS += -fPIC

LFLAGS += -fPIC


INCLUDE = -Iinclude

CFLAGS += $(INCLUDE)

# Files - Pure C implementation (no assembly)
SOURCES = extra.c sm3.c basicOp.c fieldOp.c ecc.c sm2.c utils.c ecc_montg.c ecc_basepoint_mul.c
# Assembly sources are no longer needed - all operations implemented in C
# ASM_SOURCES = basicOpas.s fieldOpas.s ecc_as.s

COBJS=$(SOURCES:.c=.o)
COBJS := $(addprefix build/, $(COBJS))

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

libsm2.so: $(COBJS)
	$(CC) $(LFLAGS) -shared -o $@ $(COBJS)

test/test_sm2: test/test_sm2.c $(COBJS)
	$(CC) $(CFLAGS) $(TESTFLAGS) -o $@ test/test_sm2.c $(COBJS) $(TESTLDFLAGS)

test/test_speed: test/test_speed.c $(COBJS)
	$(CC) $(CFLAGS) $(TESTFLAGS) -o $@ test/test_speed.c $(COBJS) $(TESTLDFLAGS)

$(COBJS):build/%.o:%.c
	$(CC) -c $^ -o $@ $(CFLAGS)

.PHONY:clean
clean:
	rm -f test/test_sm2
	rm -f test/test_speed
	rm -f test/test_basicOp
	rm -f test/test_openssl
	rm -f test/debug_*
	rm -f libsm2.so
	rm -f build/*.o

# clean:
# 	rm $(OBJS) ./build/$(BIN)

# $(OBJS) : $(SRCS)
# 	$(CC) $(CFLAGS) -c yog_utils.c      	-o ./build/yog_utils.o
# 	$(CC) $(CFLAGS) -c extra.c          	-o ./build/extra.o
# 	$(CC) $(CFLAGS) -c sm3.c            	-o ./build/sm3.o
# 	$(CC) $(CFLAGS) -c basicOp.c        	-o ./build/basicOp.o
# 	$(CC) $(CFLAGS) -c fieldOp.c        	-o ./build/fieldOp.o
# 	$(CC) $(CFLAGS) -c ecc.c            	-o ./build/ecc.o
# 	$(CC) $(CFLAGS) -c sm2.c            	-o ./build/sm2.o
# 	$(CC) $(CFLAGS) -c utility.c        	-o ./build/utility.o
# 	$(CC) $(CFLAGS) -c sm2Test.c        	-o ./build/sm2Test.o
# 	$(CC) $(CFLAGS) -c ecc2.c        		-o ./build/ecc2.o
# 	$(CC) $(CFLAGS) -c ecc_montg.c      	-o ./build/ecc_montg.o
# 	$(CC) $(CFLAGS) -c ecc_basepoint_mul.c  -o ./build/ecc_basepoint_mul.o
	
