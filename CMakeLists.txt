cmake_minimum_required(VERSION 3.16)
project(test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS framework/*.[ch]pp framework/*.h src/*.[ch]pp src/*.h)

add_executable(AudioWorklet ${MY_SOURCES})

target_include_directories(AudioWorklet PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/framework ${CMAKE_CURRENT_SOURCE_DIR}/src)

if (CMAKE_SYSTEM_NAME STREQUAL "Emscripten") 
  # Emscripten specific compiler settings
  set_target_properties(AudioWorklet PROPERTIES SUFFIX ".js")
  target_compile_options(AudioWorklet PRIVATE "-sWASM_WORKERS=1")
  target_link_options(AudioWorklet PRIVATE "-sWASM_WORKERS=1" "-sAUDIO_WORKLET=1" "-sWEBAUDIO_DEBUG=1" "--bind")
else()
  # TODO: Desktop plugin/standalone target support
endif()
