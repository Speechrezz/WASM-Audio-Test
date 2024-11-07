#!/bin/bash

set -e
emcmake cmake -B build/wasm
CMAKE_EXPORT_COMPILE_COMMANDS=1 cmake --build build/wasm
# cp build/wasm/compile_commands.json .
cp src/index.html build/wasm/index.html
# open "http://localhost:8000/build/wasm/index.html"
node server.js
