// layout per event: [status, data1, data2, frameOffset]

const READ_INDEX = 0;
const WRITE_INDEX = 1;

const HEADER_INTS = 2; // still Int32Array for head/tail indices

export function packMidiEvent(status, d1, d2, frameOffset) {
  const packed =
    ((frameOffset & 0xff) << 24) |
    ((d2 & 0xff) << 16) |
    ((d1 & 0xff) << 8)  |
    (status & 0xff);
  return packed;
}

export function unpackMidiEvent(ev32) {
  const status =  ev32 & 0xff;
  const data1  = (ev32 >>> 8) & 0xff;
  const data2  = (ev32 >>> 16) & 0xff;
  const offs   = (ev32 >>> 24) & 0xff;
  return { status, data1, data2, frameOffset: offs };
}

export class MidiBuffer {
  constructor(maxNumberOfEvents = 1024) {
    this.maxEvents = maxNumberOfEvents
    this.sab = new SharedArrayBuffer(
      (HEADER_INTS + this.maxEvents) * Int32Array.BYTES_PER_ELEMENT
    );
    this.header = new Int32Array(this.sab);
    this.midiData = new Int32Array(this.sab, HEADER_INTS * Int32Array.BYTES_PER_ELEMENT);
  }

  push(status, d1, d2, frameOffset = 0) {
    const writeIndex = Atomics.load(this.header, WRITE_INDEX);
    this.midiData[writeIndex] = packMidiEvent(status, d1, d2, frameOffset);

    const newWriteIndex = (writeIndex + 1) % this.maxEvents;
    Atomics.store(this.header, WRITE_INDEX, newWriteIndex);
  }

  read(outputArray) {
    let readIndex = Atomics.load(this.header, READ_INDEX);
    const writeIndex = Atomics.load(this.header, WRITE_INDEX);
    let count = 0;
    
    while (readIndex !== writeIndex) {
      outputArray[count++] = this.midiData[readIndex];
      readIndex = (readIndex + 1) % this.maxEvents;
    }

    Atomics.store(this.header, READ_INDEX, readIndex);
    return count;
  }

  calculateUnreadCount(readIndex, writeIndex) {
    return (writeIndex - readIndex + this.maxEvents) % this.maxEvents;
  }
}