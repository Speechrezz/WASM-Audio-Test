# Configures `compile_commands.json` for intellisense
emcmake cmake -S . -B build/wasm -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Build WASM
emcmake cmake -B build/wasm
cmake --build build/wasm
mkdir frontend/public/wasm
cp build/wasm/AudioWorklet.** frontend/public/wasm

# Build Svelte
cd frontend
npm run build
cd -

# Copy files to final directory
cp frontend/dist/** server-svelte -r