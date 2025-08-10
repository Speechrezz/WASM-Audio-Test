cmake_minimum_required(VERSION 3.8)

project(test)

add_definitions(-std=c++17)
set (CMAKE_CXX_STANDARD 17)

if (DEFINED EMSCRIPTEN)
	add_executable(AudioWorklet src/AudioWorklet.cpp)

	set(CMAKE_EXECUTABLE_SUFFIX ".js")

	set_target_properties(AudioWorklet PROPERTIES LINK_FLAGS "-s WASM_WORKERS=1 -s AUDIO_WORKLET=1 -s WEBAUDIO_DEBUG=1")
else()
	#add_executable(AudioWorklet main.cpp)
	#add_library(main SHARED main.cpp)
endif()