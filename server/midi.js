import { MidiBuffer } from "./midi_buffer.js";

let midi = null; // global MIDIAccess object
let midiBuffer = new MidiBuffer(1024);

function onMIDISuccess(midiAccess) {
  console.log("MIDI ready!");
  midi = midiAccess; // store in the global (in real usage, would probably keep in an object instance)
  startLoggingMIDIInput(midi);
}

function onMIDIFailure(msg) {
  console.error(`Failed to get MIDI access - ${msg}`);
}

export function initializeMIDI() {
  navigator.requestMIDIAccess().then(onMIDISuccess, onMIDIFailure);
}

export function getMidiBuffer() {
  return midiBuffer;
}

function prettyLog(e) {
  const [status, data1, data2] = e.data;
  const cmd = status >> 4;
  const ch = (status & 0x0f) + 1;
  const timeStamp = `${e.timeStamp.toFixed(3)} ms`;

  if (cmd === 0x9 && data2 > 0) {
    console.log(`Note On  ch ${ch}  note ${data1}  vel ${data2}  (${timeStamp})`);
  } else if (cmd === 0x8 || (cmd === 0x9 && data2 === 0)) {
    console.log(`Note Off ch ${ch}  note ${data1}          (${timeStamp})`);
  } else if (cmd === 0xB) {
    console.log(`CC      ch ${ch}  cc ${data1}  val ${data2}    (${timeStamp})`);
  } else if (cmd === 0xE) {
    const value = (data2 << 7) | data1; // pitch bend 0..16383, center 8192
    console.log(`Pitch   ch ${ch}  value ${value}        (${timeStamp})`);
  } else {
    console.log(`Raw     ch ${ch}  [${Array.from(e.data)}]       (${timeStamp})`);
  }
}

function onMIDIMessage(e) {
  midiBuffer.push(e.data[0], e.data[1], e.data[2]);
  prettyLog(e);
}

function startLoggingMIDIInput(midiAccess) {
  midiAccess.inputs.forEach((entry) => {
    entry.onmidimessage = onMIDIMessage;
  });
}