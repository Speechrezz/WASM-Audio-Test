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

#ifdef XYNTH_WASM
  #include <emscripten.h>
  #include <sstream>
  inline void logToJavascriptConsole(const std::string& s)
  {
    EM_ASM({
      const ptr = $0;
      const len = $1;
      const str = UTF8ToString(ptr, len);
      console.log("[DEBUG] " + str);
    }, s.c_str(), s.size());
  }
#else
  #include <iostream>
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

  #ifdef XYNTH_WASM
    #define DBG(x) \
    { \
      std::stringstream stream; \
      stream << std::boolalpha << x; \
      logToJavascriptConsole(stream.str()); \
    }
  #else
    #define DBG(x) std::cout << std::boolalpha << x << "\n"
  #endif
#else
  #define XASSERT(x)
  #define DBG(x)
#endif