import { mount } from 'svelte'
import './app.css'
import App from './App.svelte'
import { initializeMIDI } from './lib/scripts/midi'
import AppInterface from './lib/components/AppInterface.svelte';

// const app = mount(App, {
//   target: document.getElementById('app')!,
// });

// export default app;

export let audioWorkletNode: AudioWorkletNode | null = null;

window.onAudioProcessorInitialized = (node: AudioWorkletNode) =>
{
  audioWorkletNode = node;

  console.log("Init!");
  console.log("node", node);
  console.log("Module", window.Module);
  
  initializeMIDI(node);

  mount(AppInterface, {
    target: document.getElementById('synth')!,
  });
}