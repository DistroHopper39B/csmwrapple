;
; Copyright (C) 2025 Sylas Hollander.
; PURPOSE: ASM entry point for Apple TV.
; SPDX-License-Identifier: MIT
;

extern csmwrapple_init

SECTION .text
global start

start:
    ; Push bootArgs pointer to the stack.
    push eax
    ; Call C entry point
    call csmwrapple_init
    ; Halt the CPU
    hlt