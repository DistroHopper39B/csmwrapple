/*
 * Copyright (C) 2025 Sylas Hollander.
 * Copyright (C) 2020-2025 Jiaxun Yang.
 * PURPOSE: CSMWrapple main file.
 * SPDX-License-Identifier: LGPL-2.1-only
*/

#include "csmwrapple.h"

mach_boot_args_t *gBA;

noreturn void csmwrapple_init(mach_boot_args_t *ba)
{
    gBA = ba;

    cons_init(&ba->video, 0xFFFFFFFF, 0x00000000);

    bool_t verbose = (ba->video.display_mode == DISPLAY_MODE_TEXT);
    if (verbose)
        cons_clear_screen(0x00000000);

    printf("Hello World!\n");


    while (1);
}