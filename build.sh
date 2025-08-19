# Configures `compile_commands.json` for intellisense
emcmake cmake -S . -B build/wasm -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Builds
emcmake cmake -B build/wasm
cmake --build build/wasm
cp build/wasm/AudioWorklet.** server