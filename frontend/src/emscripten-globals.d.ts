// Tell TS this file only adds types
export {};

declare global {
  interface Window {
    // your callback Emscripten/worklet will call
    onAudioProcessorInitialized?: (node: AudioWorkletNode) => void;

    // Emscripten injects this when you load /wasm/AudioWorklet.js
    Module?: EmscriptenModule;

    // Vite sets this at runtime; optional to type it
    __vite_base__?: string;
  }

  // Some builds also expose a global var (not just window.Module)
  var Module: EmscriptenModule | undefined;
}

// Minimal surface you actually use; expand as needed
interface EmscriptenModule {
  locateFile?(p: string): string;
  onRuntimeInitialized?: () => void;
  cwrap?(name: string, ret: string | null, args: string[]): any;

  // Typical heaps if you touch them
  HEAP8?: Int8Array;
  HEAP16?: Int16Array;
  HEAP32?: Int32Array;
  HEAPU8?: Uint8Array;
  HEAPF32?: Float32Array;
  HEAPF64?: Float64Array;
}
