let midi = null; // global MIDIAccess object

function onMIDISuccess(midiAccess) {
  console.log("MIDI ready!");
  midi = midiAccess; // store in the global (in real usage, would probably keep in an object instance)
  startLoggingMIDIInput(midi);
}

function onMIDIFailure(msg) {
  console.error(`Failed to get MIDI access - ${msg}`);
}

function getMIDIAccess() {
  navigator.requestMIDIAccess().then(onMIDISuccess, onMIDIFailure);
}

function prettyLog(e, device = null) {
  console.log("e.data:", e.data)
  const [status, data1, data2] = e.data;
  const cmd = status >> 4;
  const ch = (status & 0x0f) + 1;

  if (cmd === 0x9 && data2 > 0) {
    console.log(`Note On  ch ${ch}  note ${data1}  vel ${data2}  (${device})`);
  } else if (cmd === 0x8 || (cmd === 0x9 && data2 === 0)) {
    console.log(`Note Off ch ${ch}  note ${data1}            (${device})`);
  } else if (cmd === 0xB) {
    console.log(`CC      ch ${ch}  cc ${data1}  val ${data2}  (${device})`);
  } else if (cmd === 0xE) {
    const value = (data2 << 7) | data1; // pitch bend 0..16383, center 8192
    console.log(`Pitch   ch ${ch}  value ${value}           (${device})`);
  } else {
    console.log(`Raw     ch ${ch}  [${Array.from(e.data)}]   (${device})`);
  }
}
// usage in the handler:
// input.onmidimessage = (e) => prettyLog(e, input.name);


function onMIDIMessage(event) {
  let str = `MIDI message received at timestamp ${event.timeStamp}[${event.data.length} bytes]: `;
  for (const character of event.data) {
    str += `0x${character.toString(16)} `;
  }
  console.log(str);
}

function startLoggingMIDIInput(midiAccess) {
  midiAccess.inputs.forEach((entry) => {
    //entry.onmidimessage = onMIDIMessage;
    entry.onmidimessage = prettyLog;
  });
}
