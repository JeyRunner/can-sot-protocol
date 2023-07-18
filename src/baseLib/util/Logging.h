#pragma once

#include <stdio.h>

constexpr bool DEBUG = true;

#define logInfo(fmt, ...) \
            do { if (DEBUG) printf( fmt __VA_ARGS__); } while (0)
#define logWarn(fmt, ...) \
            do { if (DEBUG) printf(fmt __VA_ARGS__); } while (0)