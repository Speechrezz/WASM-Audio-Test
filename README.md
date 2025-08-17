# Audio WASM Test

Attempt at a framework for creating audio software that runs in the browser via WASM. C++ for the audio processing, standard web technologies for the user interface.

Working name: Xylo.

## Goals

- Use [Emscripten](https://github.com/emscripten-core/emscripten) to compile C++ to WASM.
- C++ abstractions which wrap around the Web Audio API to create a [JUCE](https://github.com/juce-framework/JUCE)-like API.
- Simple way to connect the HTML/CSS/JS user interface with the audio processing C++.
- Maybe use [Svelte](https://svelte.dev/) + TypeScript to further simplify the creation of user interfaces and state management.
- Maybe add a [CLAP](https://github.com/free-audio/clap) backend with webviews for the interface to allow the same codebase to target the web and desktop plugin binaries (CLAP, VST3, etc.) and even standalone desktop binaries.

## Building

First be sure to have [Emscripten](https://emscripten.org/docs/getting_started/downloads.html) installed.

Run the `build.sh` script found in the root directory. This will automatically generate the `compile_commands.json` file for intellisense in VSCode. This will also copy the necessary WASM/JS files to the `server/` directory.

To configure intellisense, create a `.vscode/setting.json` with the following text:

```json
{
    "C_Cpp.default.compileCommands": "${workspaceFolder}/build/wasm/compile_commands.json",
    "C_Cpp.default.compilerPath": "path/to/emsdk/upstream/emscripten/em++",
    "C_Cpp.default.intelliSenseMode": "clang-x64"
}
```

> Be sure to enter the correct `emsdk` path.

## Running

To properly run the WASM component, a web server must be running to; simply opening `index.html` will **NOT** work. I have included a very basic web server in `server/server.js`. Run it with node:

```sh
cd server
node server.js
```