#pragma once

#include <stdio.h>

//constexpr bool DEBUG = false;
#ifndef SOT_DEBUG
#define SOT_DEBUG true
#endif

/*
#define logInfo(fmt, ...) \
            do { if (DEBUG) printf( fmt __VA_ARGS__); } while (0)
#define logWarn(fmt, ...) \
            do { if (DEBUG) printf(fmt, __VA_ARGS__); } while (0)
#define logWarn(fmt) \
            do { if (DEBUG) printf(fmt); } while (0)
            */

template<typename... Args> inline void logWarn(const char *f, Args... args) {
  if constexpr (SOT_DEBUG) {
    printf("[WARN] ");
    printf(f, args...);
    printf("\n");
  }
}

template<typename... Args> inline void logError(const char *f, Args... args) {
  if constexpr (SOT_DEBUG) {
    printf("[WARN] ");
    printf(f, args...);
    printf("\n");
  }
}