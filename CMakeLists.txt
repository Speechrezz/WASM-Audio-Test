cmake_minimum_required(VERSION 3.16)
project(test LANGUAGES CXX)

# Prefer this over add_definitions(-std=c++17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS src/*.[ch]pp src/*.c src/*.h)

add_executable(AudioWorklet ${MY_SOURCES})

# <-- This is the key line
target_include_directories(AudioWorklet PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Emscripten-specific options (more idiomatic checks & APIs)
if (CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
  set_target_properties(AudioWorklet PROPERTIES SUFFIX ".js")
  target_link_options(AudioWorklet PRIVATE
    "-sWASM_WORKERS=1"
    "-sAUDIO_WORKLET=1"
    "-sWEBAUDIO_DEBUG=1"
  )
endif()
