#
# Copyright (C) 2025 Sylas Hollander.
# PURPOSE: General purpose video functions for linear framebuffers
# SPDX-License-Identifier: MIT
#

# Check what OS we're running. Should work on Linux and macOS.
OSTYPE = $(shell uname)

# Target defs for Linux cross compiler.
TARGET = i386-apple-darwin8

# Definitions for compiler
CC := clang
NASM := nasm

# Definitions for linker
ifeq ($(OSTYPE),Linux)
	LD := /opt/cross/bin/$(TARGET)-ld
else
	LD := ld
endif

# Flags for mach-o linker
LDFLAGS := -static \
           -segalign 0x1000 \
           -segaddr __TEXT 0x00400000 \
           -sectalign __TEXT __text 0x1000 \
           -sectalign __DATA __common 0x1000 \
           -sectalign __DATA __bss 0x1000 \

DEFINES := -DDEBUG

CFLAGS := -Wall -nostdlib -fno-stack-protector -fno-builtin -O0 --target=$(TARGET) -Iinclude $(DEFINES)

OBJS := start.o baselibc_string.o csmwrapple.o tinyprintf.o video_cons.o

%.o: %.nasm
	$(NASM) -fmacho32 $< -o $@
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
mach_kernel: $(OBJS)
	$(LD) $(LDFLAGS) $^ -o $@
all: mach_kernel

clean:
	rm -f *.o mach_kernel