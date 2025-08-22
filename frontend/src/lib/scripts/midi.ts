let midi: MIDIAccess | null = null; // global MIDIAccess object
let audioWorkletNode: AudioWorkletNode | null = null;

export function initializeMIDI(workletNode: AudioWorkletNode) {
  audioWorkletNode = workletNode;
  navigator.requestMIDIAccess().then(onMIDISuccess, onMIDIFailure);
}

function onMIDISuccess(midiAccess: MIDIAccess) {
  console.log("MIDI ready!");
  midi = midiAccess; // store in the global (in real usage, would probably keep in an object instance)
  startLoggingMIDIInput(midi);
}

function onMIDIFailure(msg: string) {
  console.error(`Failed to get MIDI access - ${msg}`);
}

function startLoggingMIDIInput(midiAccess: MIDIAccess) {
  midiAccess.inputs.forEach((entry) => {
    entry.onmidimessage = onMIDIMessage;
  });
}

function onMIDIMessage(e: MIDIMessageEvent) {
  if (audioWorkletNode!.context.state === "running") {
    const adjustedTimeStamp = midiTimestampToContextFrame(e.timeStamp);
    Module.pushMidiEvent(packMidiEvent(e.data![0], e.data![1], e.data![2]), adjustedTimeStamp);
  }

  prettyLog(e);
}

function midiTimestampToContextFrame(timeStampMs: number) {
  const audioContext = audioWorkletNode!.context;

  const { performanceTime, contextTime } = audioContext.getOutputTimestamp();
  const eventCtxTimeSec = contextTime + (timeStampMs - performanceTime) * 1e-3;

  return Math.max(0, Math.floor(eventCtxTimeSec * audioContext.sampleRate));
}

function prettyLog(e: MIDIMessageEvent) {
  const [status, data1, data2] = e.data!;
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
    console.log(`Raw     ch ${ch}  [${Array.from(e.data!)}]       (${timeStamp})`);
  }
}

// layout per event: [status, data1, data2, frameOffset]
function packMidiEvent(status: number, d1: number, d2: number, frameOffset: number) {
  const packed =
    ((frameOffset & 0xff) << 24) |
    ((d2 & 0xff) << 16) |
    ((d1 & 0xff) << 8)  |
    (status & 0xff);
  return packed;
}