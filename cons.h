/*
 * Copyright (C) 2025 Sylas Hollander.
 * PURPOSE: Header for general purpose video functions for linear framebuffers
 * SPDX-License-Identifier: MIT
*/

#pragma once

typedef struct _linear_framebuffer_t
{
    bool_t      enabled;

    uint64_t    base;
    uint64_t    size;

    uint32_t    pitch;
    uint32_t    width;
    uint32_t    height;
    uint32_t    depth;
    uint32_t    pixels_per_row;

    uint8_t     red_size;
    uint8_t     red_shift;
    uint8_t     green_size;
    uint8_t     green_shift;
    uint8_t     blue_size;
    uint8_t     blue_shift;
    uint8_t     reserved_size;
    uint8_t     reserved_shift;
} linear_framebuffer_t;

typedef struct _console_priv_t
{

    uint32_t    cursor_x;
    uint32_t    cursor_y;
    uint32_t    width;
    uint32_t    height;
    uint32_t    size;
    uint32_t    fg_color;
    uint32_t    bg_color;
} console_priv_t;

#define PRINT_BUFFER_SIZE 1024

#define RGBA_TO_NATIVE(fb, color) \
    (((color >> 24) & 0xFF) << fb.red_shift) | \
    (((color >> 16) & 0xFF) << fb.green_shift) | \
    (((color >> 8) & 0xFF) << fb.blue_shift) | \
    (((color) & 0xFF) << fb.reserved_shift)

/* Functions */
extern bool_t cons_init(void *video_params, uint32_t fg_color, uint32_t bg_color);
extern void cons_clear_screen(uint32_t color);