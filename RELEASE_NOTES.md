## Why this fork exists

This started as a repair job for a real MASCHINE MK1. Pads sometimes failed to register or dropped hits, and highlights could appear on the wrong controls. Display updates also left stale content behind. We wanted the controller to behave reliably and the helper to show what the hardware was actually doing.

The fixes decode pad reports by their embedded pad IDs, preserve releases, correct physical control/LED mappings, and retain all pending display updates, including black clears. Controller Studio now follows the MK1 layout and makes input, sensitivity and mapping adjustments visible.

Somehow, we also couldn't find Doom running on the controller. Clearly an oversight. Doom support has therefore been restored — using the Doom engine and Freedoom data, with gameplay on the left display and the original status bar on the right.

## What is included

MK1 Revive 0.4.0-beta.1 adds Controller Studio with live MK1 pad/encoder/button monitoring, per-pad thresholds and gain, encoder direction/gain, and pad/encoder destination overrides. The helper includes a game launcher and a new app/menu-bar icon.

Idle games now include Pong, Freedoom and Breakout. Hardware arrows select games; each Pong paddle switches to AI after ten seconds without input. Freedoom uses the left LCD for gameplay and the right for the original status bar, with illuminated physical pad controls. This release also includes the USB pad decoding and display update fixes.

Apple Silicon, macOS 14+. Drag the app to Applications and launch. The beta uses ad hoc signing and is not notarized. Full Xcode is not needed to build; Command Line Tools, Python 3 and Git suffice.

Validation: release build, pad decoder and LED tests, display regression, game selection and AI handoff tests, profile validation, and real Freedoom movement/firing/status-bar smoke test passed locally. Helper-to-hardware game launch was checked live. Automated tests do not replace musical latency or extended hardware testing.

The source archive includes the pinned GPL Doom engine sources and the MK1 backend. Freedoom data license/credits are bundled. Original MK1 Revive author: andy83-projects. See NOTICE.md for component licensing details.
