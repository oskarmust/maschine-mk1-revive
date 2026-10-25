> Fork maintained by [oskarmust](https://github.com/oskarmust), based on
> [andy83-projects/maschine-mk1-revive](https://github.com/andy83-projects/maschine-mk1-revive).
> Apple Silicon beta: macOS 14 or later. Physical MK1 hardware is required.

## Why this fork exists

This started as a repair job for a real MASCHINE MK1. Pads sometimes failed to register or dropped hits, and highlights could appear on the wrong controls. Display updates also left stale content behind. We wanted the controller to behave reliably and the helper to show what the hardware was actually doing.

The fixes decode pad reports by their embedded pad IDs, preserve releases, correct physical control/LED mappings, and retain all pending display updates, including black clears. Controller Studio now follows the MK1 layout and makes input, sensitivity and mapping adjustments visible.

Somehow, we also couldn't find Doom running on the controller. Clearly an oversight. Doom support has therefore been “restored” — using the Doom engine and Freedoom data, with gameplay on the left display and the original status bar on the right. For historical accuracy: upstream never claimed to include Doom; the restoration is entirely in our imagination.

## Controller Studio and idle games

Open **Controller Studio** from the menu bar to see live pads, encoders and
buttons in a matching MK1 layout. Select a pad or encoder to edit its enable
state, gain and MASCHINE destination; pads also support threshold overrides,
and encoders support direction inversion. Release held pads before applying.
Profiles live in `~/Library/Application Support/MK1 Revive/controls.plist`.
The existing global settings remain available in the same window.

The Games tab launches Pong, Freedoom or Breakout when MASCHINE is not using
the controller. The hardware arrows below Browse/Sampling select a game;
Play starts it and Browse returns to the menu. Screen encoder 3 also selects.
Pong switches each paddle to AI after 10 seconds without input; turning its
encoder restores manual control. Breakout uses encoder 1 or pads 1/4, with
Play to serve. Freedoom uses pads 14/10 to move, 9/11 to turn, 13/15 to
strafe, 12 to fire and 16 to use. Gameplay is on the left LCD, its original
status bar on the right. Freedoom audio is disabled.

## Screenshots

**Controller Studio** — MK1 hardware layout with live pad, encoder and button
feedback. Select a pad to inspect its pressure, sensitivity, thresholds and
MASCHINE destination.

![Controller Studio showing the MK1 layout and Pad 14 settings](docs/screenshots/controller-studio.png)

**Encoder settings** — select a rotary encoder to adjust gain, invert its
direction or override its MASCHINE destination.

![Controller Studio showing screen encoder sensitivity, inversion and destination settings](docs/screenshots/encoder-settings.png)

**Game launcher** — start Pong, Freedoom or Breakout from the helper when
MASCHINE is not using the controller. The hardware arrows also select games.

![Game launcher with Pong, Freedoom and Breakout](docs/screenshots/game-launcher.png)

## Building this fork

Install Apple's Command Line Tools (`xcode-select --install`), Python 3 and Git.
Full Xcode is optional. Clone recursively, then run:

```sh
git submodule update --init --recursive
sh scripts/build-release-assets.sh
sh scripts/test.sh
```

The build downloads the pinned Freedoom 0.13.0 WAD and validates its SHA-256.
The app ZIP is in `dist/`; drag the app to Applications and launch it.
These beta builds use ad hoc signing and are not notarized. The engine's
GPL source and Freedoom license/credits are retained in this repository and
app bundle. See [NOTICE.md](NOTICE.md) for component attribution.

---

# maschine-mk1-revive

Userspace driver for the Native Instruments Maschine MK1 on Apple Silicon macOS — no kext, no DriverKit, no special entitlements.

[![Buy Me a Coffee](https://img.shields.io/badge/Buy%20me%20a%20coffee-andy83-FFDD00?style=flat&logo=buy-me-a-coffee&logoColor=black)](https://buymeacoffee.com/andy83)

## What works

- Maschine software detects the MK1 in its controller list
- Both displays render correctly (170×64 grayscale, ST7529)
- All LEDs: pad rubber, group A–H, transport, scene-row, screen area (SA1–SA8)
- Pad input with velocity and pressure; sustain gate eliminates stuck notes
- All buttons and transport controls
- All 11 encoders (Volume, Tempo, Swing + 8 screen area), with jitter and crosstalk filtering
- Physical DIN MIDI Out (hardware-verified); DIN MIDI In (untested)
- USB hot-plug: survives unplug/replug; starts before device is connected
- **MK1 Revive** menu bar app: start/stop/restart the bridge, adjust settings

## Install

Download `maschine-mk1-revive-vX.X.X-macos.zip` from [Releases](https://github.com/andy83-projects/maschine-mk1-revive/releases), unzip, and drag **MK1 Revive** to `/Applications`.

Launch **MK1 Revive** — on first launch it automatically installs the LaunchAgent and starts the bridge.

### If macOS blocks the app

Right-click → Open, or go to `System Settings → Privacy & Security` and allow it. If the app is quarantined after unzipping:

```bash
xattr -dr com.apple.quarantine "/Applications/MK1 Revive.app"
```

Then launch the app normally.

### Uninstall

```bash
launchctl bootout "gui/$(id -u)" ~/Library/LaunchAgents/com.dragco.mk1-bridge.plist 2>/dev/null || true
rm -f ~/Library/LaunchAgents/com.dragco.mk1-bridge.plist
sudo rm -rf "/Applications/MK1 Revive.app"
```

## Configuration

Use the **Settings…** panel in the MK1 Revive menu bar app, or set environment variables directly in the launchd plist.

### Pad sensitivity

| Env var | Default | Description |
|---------|---------|-------------|
| `MK1_PAD_HIT_ON` | `300` | ADC pressure (0–4095) to trigger a pad strike |
| `MK1_PAD_HIT_OFF` | `150` | ADC pressure must fall below this to register release |
| `MK1_PAD_PRESSURE` | `200` | Minimum change to forward a pressure-update event |
| `MK1_PAD_DEBOUNCE_MS` | `10` | Milliseconds between hit-off and next hit-on |
| `MK1_PAD_SUSTAIN` | `3` | Consecutive above-threshold reports required before hit_on fires |

### Encoder

| Env var | Default | Description |
|---------|---------|-------------|
| `MK1_ENCODER_SENSITIVITY` | `10` | Step multiplier in tenths (10 = ×1.0, 5 = ×0.5, 20 = ×2.0) |
| `MK1_ENCODER_MIN_DELTA` | `2` | Minimum byte-delta per report to register a step (filters jitter) |

### Display & integration

| Env var | Default | Description |
|---------|---------|-------------|
| `MK1_DISPLAY_TICK_MS` | `16` | Display flush interval in ms (16 ≈ 60 fps) |
| `MK1_AUTO_OPEN_MASCHINE` | off | Auto-launch Maschine when bridge starts |
| `MK1_AUTO_CLOSE_MASCHINE` | off | Auto-quit Maschine when bridge stops |

### Diagnostics

| Env var | Default | Description |
|---------|---------|-------------|
| `MK1_VERBOSE_IO` | off | Log every USB bulk transfer |
| `MK1_TIMING_TRACE` | off | Log IPC and USB timing measurements |
| `MK1_PAD_TRACE` | off | Log pad event send results to `timing.log`, without tracing every USB transfer |
| `MK1_PAD_CAPTURE` | unset | Path for a circular raw EP4 capture before filtering; 72-byte records (LE uptime ns + 64 raw bytes), limited to 16 MiB; sort by timestamp after wrap |

## Build

Requires Xcode 15+ on Apple Silicon macOS 14+.

```bash
xcodebuild -project maschine-mk1-revive.xcodeproj -scheme mk1-bridge -configuration Debug build
bash mk1-menubar/build.sh   # builds MK1 Revive.app → build/MK1 Revive.app
```

Run directly for development:

```bash
./build/Debug/mk1-bridge [--display_tick_ms=16] [--no-partial-display] [--help]
```

## How it works

The MK1 originally required `NIUSBMaschineController.kext` — a kernel extension that never received an ARM64 build. This project replaces it entirely in userspace.

`mk1-bridge` kills the real `NIHardwareAgent` and impersonates it: it registers the `NIHWMainHandler` CFMessagePort itself, performs the full IPC handshake with Maschine software, and owns direct IOKit USB bulk transfers to the hardware.

```
[Maschine.app] ←─CFMessagePort─→ [mk1-bridge] ←─IOKit USB─→ [MK1 hardware]
```

### Components

| Target | Type | Purpose |
|--------|------|---------|
| `mk1-usb` | static lib | Claim USB device, bulk transfers |
| `mk1-ipc` | static lib | CFMessagePort handshake + NI IPC protocol |
| `mk1-bridge` | daemon | Glues USB ↔ IPC, runs as launchd agent |
| `mk1-menubar` | app bundle | Menu bar app: service control + settings |
| `mk1-shim` | dylib | DYLD_INSERT_LIBRARIES shim for debugging |

## Prior art

- **[biappi/Macchina](https://github.com/biappi/Macchina)** — Reverse engineered the CFMessagePort handshake between NIHardwareAgent and Maschine software (MK1, 2012).
- **[SamL98/NIProtocol](https://github.com/SamL98/NIProtocol)** — Full MK2 handshake; `NIHWMainHandler` port name; complete client + server in C. ([article](https://lerner98.medium.com/rage-against-the-maschine-3357be1abc48))
- **[terminar/rebellion](https://github.com/terminar/rebellion)** — Most complete IPC implementation (MK3/KK); key insight that USB and IPC data share the same format.
- **[fzero/maschine-mk1](https://github.com/fzero/maschine-mk1)** — MK1 HID protocol on Linux; confirms VID `0x17CC` / PID `0x0808`.
- **[hansfbaier/open-maschine](https://github.com/hansfbaier/open-maschine)** — MK2 proof-of-concept using hidapi.

## Changelog

### v0.3.3 — 2026-04-24
- Fix invisible tractor beam in idle animation — grey pixel values render as black on this display; replaced with white alternating scanlines

### v0.3.2 — 2026-04-23
- UFO tractor beam idle animation: small cat stands at bottom of left display while a UFO sweeps back and forth, homes in, fires a tractor beam, sucks the cat up, flies around, and returns the cat to the ground (~26.8 s loop)

### v0.3.1 — 2026-04-20
- Fix rogue pad hits from scan table reports leaking into the pad pressure classifier
- Fix orphaned hit-off events caused by `was_active` tracking pressure state instead of sent-event state
- Improve scan table classifier: compare top nibbles only, require both ADC channels to agree

### v0.3.0 — 2026-04-17
- Fix pad stuck notes and bounce: sustain gate, reliable hit_off retry, hit_on conditional on IPC success
- Fix encoder direction reversal; filter single-count jitter; suppress electrical crosstalk between adjacent encoders

### v0.2.1 — 2026-04-16
- Move logs to `/tmp/maschine-mk1-revive/`; cap at 10 MiB with wraparound

### v0.2.0 — 2026-04-14
- MK1 Revive menu bar app: service management and settings panel
- Runtime-configurable pad thresholds and encoder sensitivity via env vars

### Idle games, clock and Freedoom

Turn the **third encoder below the left display** to select **Pong / Freedoom**;
**Play** starts the selected game. **Browse** returns to selection; **Restart**
starts the selected game over.

- Pong: encoder 1 controls the left paddle. Encoder 4 controls the right paddle
  and switches it from AI to a second player.
- Freedoom: the real doomgeneric engine runs Freedoom Phase 1 0.13.0 (E1M1).
  Pads 14/10 move forward/back (W/S), pads 9/11 turn left/right (A/D),
  pad 12 fires, pad 16 uses doors/switches. Pads 13/15 strafe left/right, so the
  game is fully playable using pads. Encoder 1 also turns and encoder 4 moves
  forward/back. Pads 1–4 select weapons, pad 5 confirms menus, pad 6 toggles the automap.
  Play also fires. This first backend has no audio.
  WASD pads light at level 40/63, strafe pads 13/15 at 12/63, fire at 63/63,
  and use at 28/63; active presses brighten lit controls to 63/63.
  Unused pads remain off. Game selection lights Play; Pong clears pad indicators.
  The idle lighting does not modify the cached Maschine LED state.
  Physical pad numbering is calibrated separately from USB IDs: idle input
  flips the USB's top-first row order to the printed bottom-first numbering
  (zero-based physical index = USB ID XOR 12). Normal Maschine forwarding
  retains its native USB pad numbering. Calibration is recorded in
  `diagnostics/20261025-idle-pad-layout/`.
- During Freedoom, the engine renders its native 320x168 playfield and 320x32
  status bar. These are separated and independently scaled to the two LCDs:
  left = game only; right = original Freedoom status-bar artwork (face, health,
  armor, ammo, weapons and keys). Both images come from the same engine frame.
  A cached frame avoids loading-screen flashes when a shared-memory update is in flight.
  During Pong and game selection, the right LCD shows local time and date.

The Doom subprocess starts only when selected. It is stopped and reaped when
returning to selection, starting Pong, restarting, or receiving Maschine display
content. It also exits if its parent bridge dies. Shared-framebuffer files are
private and removed on shutdown. The game itself ticks at Doom's native rate;
the bridge samples its grayscale framebuffer at 20 Hz. HUD and clock USB writes
are suppressed when unchanged; static menus redraw only on selection changes.
Idle controls are consumed locally rather than forwarded to Maschine.

Build the game helper with `python3 scripts/build-freedoom.py`. The menu-bar app
packaging script includes it and the WAD/license files. Upstream engine sources,
revision and licenses are documented in `third_party/README.md`.

Checks from the project directory:

```sh
clang -Wall -Wextra tests/test_idle_pong.c -o /tmp/test-idle-pong
/tmp/test-idle-pong
clang -O2 -Wall -Wextra -fsanitize=undefined tests/test_idle_games.c -o /tmp/test-idle-games
/tmp/test-idle-games
python3 tests/test_display_updates.py
python3 tests/test_freedoom_runtime.py
python3 tests/test_freedoom_keys.py
clang -O2 -Wall -Wextra tests/test_idle_game_leds.c -o /tmp/test-idle-game-leds
/tmp/test-idle-game-leds
```

The real-engine smoke test validates WAD rendering, actual player-coordinate
movement, firing/ammo consumption, native status-bar pixels and HUD telemetry. The production client
lifecycle test is in `tests/test_freedoom_client.c` (run from a bundle layout
containing the helper and Resources/freedoom directory).
