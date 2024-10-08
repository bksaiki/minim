BUILD_DIR = build
GC_DIR = bdwgc
SRC_DIR = src

CFLAGS ?= -Wall -std=c11 -O3 -g
DEPFLAGS ?= -MMD -MP
INCFLAGS ?= -I$(GC_DIR)/include
LDFLAGS ?= -L$(BUILD_DIR) -lgc

ENTRY = $(SRC_DIR)/minim.c
CONFIG = $(BUILD_DIR)/config.h
EXENAME = minim

SRCS = $(shell find $(SRC_DIR) -name "*.c" ! -wholename $(ENTRY))
OBJS = $(SRCS:%.c=$(BUILD_DIR)/%.o)
DEPS = $(OBJS:.o=.d)

MKDIR_P	= mkdir -p
RM = rm -rf

.PRECIOUS: $(BUILD_DIR)/. $(BUILD_DIR)%/.
.SECONDEXPANSION: $(BUILD_DIR)/%.o

all: $(EXENAME)

gc: build/libgc.a

clean:
	$(RM) $(BUILD_DIR)

clean-all: clean
	$(MAKE) -C $(GC_DIR) clean

$(EXENAME): $(BUILD_DIR) gc $(CONFIG) $(OBJS)
	$(CC) $(CFLAGS) $(INCFLAGS) $(OBJS) $(ENTRY) $(LDFLAGS) -o $(EXENAME)

$(GC_DIR)/Makefile:
	cd $(GC_DIR) && ./autogen.sh && ./configure --enable-static=yes --enable-shared=no

$(BUILD_DIR)/libgc.a: $(GC_DIR)/Makefile
	$(MAKE) -C $(GC_DIR)
	cp $(GC_DIR)/.libs/libgc.a $(BUILD_DIR)/libgc.a

$(CONFIG):
	echo "#define MINIM_X86_64 1" >> $@
	echo "#define PRELUDE_PATH \"$(shell pwd)/s/prelude.min\"" >> $@

$(BUILD_DIR):
	$(MKDIR_P) $@

$(BUILD_DIR)/.:
	$(MKDIR_P) $@

$(BUILD_DIR)%/.:
	$(MKDIR_P) $@

$(BUILD_DIR)/$(SRC_DIR)/%.o: $(SRC_DIR)/%.c | $$(@D)/.
	$(CC) $(CFLAGS) $(INCFLAGS) $(DEPFLAGS) -c -o $@ $<

.PHONY: all gc clean
