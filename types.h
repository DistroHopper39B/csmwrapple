/*
 * Copyright (C) 2025 Sylas Hollander.
 * PURPOSE: General purpose video functions for linear framebuffers
 * SPDX-License-Identifier: MIT
*/

#pragma once

typedef char                int8_t;
typedef unsigned char       uint8_t;
typedef short               int16_t;
typedef unsigned short      uint16_t;
typedef int                 int32_t;
typedef unsigned int        uint32_t;
typedef long long           int64_t;
typedef unsigned long long  uint64_t;

typedef __SIZE_TYPE__       size_t;
typedef __UINTPTR_TYPE__    uintptr_t;

#define true                1
#define false               0
#define NULL                (void *) 0

// Types for compatibility with EDK2.
typedef int8_t              INT8;
typedef uint8_t             UINT8;
typedef int16_t             INT16;
typedef uint16_t            UINT16;
typedef int32_t             INT32;
typedef uint32_t            UINT32;
typedef int64_t             INT64;
typedef uint64_t            UINT64;

typedef _Bool               bool_t;
#define noreturn            _Noreturn