# emcc -g AudioWorklet.cpp -o AudioWorklet.js -s WASM_WORKERS=1 -s AUDIO_WORKLET=1 -s WEBAUDIO_DEBUG=1

emcmake cmake -B build/wasm
CMAKE_EXPORT_COMPILE_COMMANDS=1 cmake --build build/wasm
cp build/wasm/compile_commands.json .
cp src/index.html build/wasm/index.html
open "http://localhost:8000/build/wasm/index.html"
node server.js
