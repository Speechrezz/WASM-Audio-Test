const BYTES_PER_EVENT = 3; // we'll store 4 uint8s per MIDI event
// layout per event: [status, data1, data2]

const READ_INDEX = 0;
const WRITE_INDEX = 1;

const HEADER_INTS = 2; // still Int32Array for head/tail indices
const HEADER_BYTES = HEADER_INTS * Int32Array.BYTES_PER_ELEMENT;

export class MidiBuffer {
  constructor(maxNumberOfEvents) {
    this.maxEvents = maxNumberOfEvents
    this.sab = new SharedArrayBuffer(
      HEADER_BYTES + this.maxEvents * BYTES_PER_EVENT
    );
    this.header = new Int32Array(this.sab);
    this.midiData = new Uint8Array(this.sab, HEADER_BYTES);
  }

  pushMidi(midiMessage) {
    const readIndex = Atomics.load(this.header, READ_INDEX);
    const writeIndex = Atomics.load(this.header, WRITE_INDEX);
  }
}