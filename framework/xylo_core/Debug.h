#pragma once

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__) 
  #define XYNTH_WINDOWS 1
#elif defined __APPLE__
  #define XYNTH_APPLE 1
  #include "Accelerate/Accelerate.h"
#elif defined __EMSCRIPTEN__
  #define XYNTH_WASM 1
#else
  #error "System NOT supported"
#endif


#if defined(_MSC_VER)
  #define XYNTH_MSVC
#endif

#if defined (_DEBUG) || ! (defined (NDEBUG) || defined (_NDEBUG))
  #define XYNTH_DEBUG 1
#endif

#ifdef XYNTH_DEBUG
  #ifdef XYNTH_MSVC
    #define XASSERT(x) if (!(x)) __debugbreak()
  #else
    #include <cassert>
    #define XASSERT(x) assert(x)
  #endif
#else
  #define XASSERT(x)
#endif