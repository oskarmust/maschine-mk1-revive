# Pad hits flashing unrelated controller LEDs

The emitter first applied a legacy remap that sent logical pad slots 0..15 to
controller LEDs in Packet B. A later pass applied the current 57-byte LED layout
using logical slots 32..56, but skipped zero values. Thus an explicit controller
LED OFF did not replace the obsolete pad-derived value, and pad activity leaked
into Groups H/G/F/E/D, Auto Write, Snap, Sampling, Browse, and other slots.

Fix: the current-layout pass always writes the software value, including zero.
No LED flashes are clamped, suppressed, or replaced with a fixed brightness.
Maschine's explicit selected-group activity feedback remains intact.

Evidence: `diagnostics/led-crosstalk/led-before.log` retains the incoming payloads
and generated USB packets. The actual emitter regression test failed before the
change and passes afterward: all 16 pad LED changes leave Packet B unchanged;
controller OFF states and all logical 32..56 flash states pass through.

Run `sh tests/run-led-mapping-test.sh`.

Installed in `/Applications/MK1 Revive.app`; USB and instance connection verified.
The user tested the physical pads and confirmed unrelated LED flashes disappeared.

Reference behavior:

- Native Instruments, MASCHINE 2.0 MK1 Manual, pp. 151 and 159-160: group focus
  brightness and required group blinking in Duplicate mode.
  https://www.native-instruments.com/fileadmin/ni_media/downloads/manuals/maschine/MASCHINE_2.0_MK1_Manual_English.pdf
- NI community discussion mentions using group-button blinking for visual
  rhythmic activity, but concerns Maschine+ rather than proving MK1 behavior:
  https://community.native-instruments.com/discussion/15907/maschine-plus-visual-metronome
- Local Maschine 2.18.4 IPC independently changes logical[39] (Group A) from
  0x32 to 0x3f during pad hits. Preserve this software-requested feedback.
