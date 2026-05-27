// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file PlatformDefines.hpp
 * \author jwall64
 * \brief A simple header for defining platform-specific macros and functions, such as debug breakpoints and platform detection.
 */

#pragma once

//
// Operating Systems
//

#if defined(_WIN32) || defined(_WIN64)
#define NBL_PLATFORM_WINDOWS
#endif

#if defined(__linux__)
#define NBL_PLATFORM_LINUX
#endif

#if defined(__APPLE__) && defined(__MACH__)
#define NBL_PLATFORM_MACOS
#endif



//
// Debug Breakpoints
//

#ifdef NDEBUG

#define NBL_DEBUG_BREAK() exit(255)

#else // NDEBUG

// From https://github.com/nemequ/portable-snippets/blob/master/debug-trap/debug-trap.h
#if defined(__has_builtin) && !defined(__ibmxl__)

#if __has_builtin(__builtin_debugtrap)
#define NBL_DEBUG_BREAK() __builtin_debugtrap()
#elif __has_builtin(__debugbreak) // __builtin_debugtrap
#define NBL_DEBUG_BREAK() __debugbreak()
#endif // __debugbreak

#endif // __has_builtin

#if !defined(NBL_DEBUG_BREAK)

#if defined(_MSC_VER) || defined(__INTEL_COMPILER)
#define NBL_DEBUG_BREAK() __debugbreak()
#elif defined(__ARMCC_VERSION)
#define NBL_DEBUG_BREAK() __breakpoint(42)
#elif defined(__ibmxl__) || defined(__xlC__)
#include <builtins.h>
#define NBL_DEBUG_BREAK() __trap(42)
#elif defined(__DMC__) && defined(_M_IX86)
static inline void NBL_DEBUG_BREAK(void) { __asm int 3h; }
#elif defined(__i386__) || defined(__x86_64__)
static inline void NBL_DEBUG_BREAK(void) { __asm__ __volatile__("int3"); }
#elif defined(__thumb__)
static inline void NBL_DEBUG_BREAK(void) { __asm__ __volatile__(".inst 0xde01"); }
#elif defined(__aarch64__)
static inline void NBL_DEBUG_BREAK(void) { __asm__ __volatile__(".inst 0xd4200000"); }
#elif defined(__arm__)
static inline void NBL_DEBUG_BREAK(void) { __asm__ __volatile__(".inst 0xe7f001f0"); }
#elif defined (__alpha__) && !defined(__osf__)
static inline void NBL_DEBUG_BREAK(void) { __asm__ __volatile__("bpt"); }
#elif defined(_54_)
static inline void NBL_DEBUG_BREAK(void) { __asm__ __volatile__("ESTOP"); }
#elif defined(_55_)
static inline void NBL_DEBUG_BREAK(void) { __asm__ __volatile__(";\n .if (.MNEMONIC)\n ESTOP_1\n .else\n ESTOP_1()\n .endif\n NOP"); }
#elif defined(_64P_)
static inline void NBL_DEBUG_BREAK(void) { __asm__ __volatile__("SWBP 0"); }
#elif defined(_6x_)
static inline void NBL_DEBUG_BREAK(void) { __asm__ __volatile__("NOP\n .word 0x10000000"); }
#elif defined(__STDC_HOSTED__) && (__STDC_HOSTED__ == 0) && defined(__GNUC__)
#define NBL_DEBUG_BREAK() __builtin_trap()
#else
#include <signal.h>
#if defined(SIGTRAP)
#define NBL_DEBUG_BREAK() raise(SIGTRAP)
#else
#define NBL_DEBUG_BREAK() raise(SIGABRT)
#endif
#endif

#endif // NBL_DEBUG_BREAK

#endif // _NDEBUG
