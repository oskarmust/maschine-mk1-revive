## Out of SPITE

**Saving Products Intended To Expire.** A useful controller deserves another
track, not a retirement date. We did it out of SPITE.

Built on **[Andy83's original MK1 Revive](https://github.com/andy83-projects/maschine-mk1-revive)**.
His userspace USB/MASCHINE bridge is the foundation; this fork adds the fixes,
Controller Studio and idle games described below.

Support this fork: [![Buy Me a Coffee — muumin1nja](https://raw.githubusercontent.com/oskarmust/maschine-mk1-revive/master/docs/badges/support-muumin1nja.svg)](https://buymeacoffee.com/muumin1nja)

Support the original project: [![Buy Me a Coffee — andy83](https://raw.githubusercontent.com/oskarmust/maschine-mk1-revive/master/docs/badges/support-andy83.svg)](https://buymeacoffee.com/andy83)

## Why this fork exists

This started as a repair job for a real MASCHINE MK1. Pads sometimes failed to register or dropped hits, and highlights could appear on the wrong controls. Display updates also left stale content behind. We wanted the controller to behave reliably and the helper to show what the hardware was actually doing.

The fixes decode pad reports by their embedded pad IDs, preserve releases, correct physical control/LED mappings, and retain all pending display updates, including black clears. Controller Studio now follows the MK1 layout and makes input, sensitivity and mapping adjustments visible.

Somehow, we also couldn't find Doom running on the controller. Clearly an oversight. Doom support has therefore been restored — using the Doom engine and Freedoom data, with gameplay on the left display and the original status bar on the right.

## New in 0.4.0-beta.2

The helper's **Games…** view can now select a custom Doom WAD. IWADs replace the
base game; PWADs load as add-ons. Choices are validated, copied to Application
Support and saved for the next launch. Restore bundled Freedoom or remove only
the add-on with separate controls. The launcher passes paths as arguments,
selects the custom map and reports startup failures. Classic IWAD/PWAD files up
to 256 MiB; no PK3 or GZDoom-only extensions.

Includes the existing Controller Studio, physical pad/display fixes, Pong,
Freedoom, Breakout, ten-second Pong AI handoff and SPITE documentation.

Validation: custom IWAD+PWAD production-launcher test (real E1M2 scene, HUD and replacement-map spawn position), WAD validation/picker tests, release build, pad decoder and LED tests, display regression, game selection and AI handoff tests, profile validation, and real Freedoom movement/firing/status-bar smoke test passed locally. Helper-to-hardware game launch was checked live. Automated tests do not replace musical latency or extended hardware testing.

The source archive includes the pinned GPL Doom engine sources and the MK1 backend. Freedoom data license/credits are bundled. Original MK1 Revive author: andy83-projects. See NOTICE.md for component licensing details.

<!-- INSTALL:START -->
## Installation

### 1. Check the requirements

- An **Apple Silicon Mac** (M-series), running **macOS 14 or later**.
- A **Native Instruments MASCHINE MK1** and a working USB data connection.
- MASCHINE desktop software installed separately if you want DAW integration.
  This download does not include MASCHINE software or NI sound libraries.
- This release is a **macOS beta**, not an Intel, Windows or iPadOS build.

**Installing the ready-made app does not require Xcode, Python, Homebrew, a
kernel extension or DriverKit.** Those are not installation steps.

### 2. Download the app ZIP

Open [release v0.4.0-beta.2](https://github.com/oskarmust/maschine-mk1-revive/releases/tag/v0.4.0-beta.2)
and expand **Assets** if needed. Download:

**[maschine-mk1-revive-v0.4.0-beta.2-macos-arm64.zip](https://github.com/oskarmust/maschine-mk1-revive/releases/download/v0.4.0-beta.2/maschine-mk1-revive-v0.4.0-beta.2-macos-arm64.zip)**

Choose the file ending in **`macos-arm64.zip`**. The `source.zip` and GitHub's
“Source code” links are for development; they are not the installable app.
Downloads are public and do not require a GitHub account.

Optional integrity check: download **SHA256SUMS** from the same release, run
`shasum -a 256` on the app ZIP, and compare its hash with the matching filename
in that file.

### 3. Put the app in Applications

1. Double-click the downloaded ZIP in Finder to extract it.
2. Drag **MK1 Revive.app** into **Applications** (`/Applications`).
3. Launch that copy from Applications. Do this before opening it for the first
   time: the background bridge is registered using the app's installed path.

Do not run the app directly from Downloads or from inside an archive preview.
For an existing installation, use the update steps below first.

### 4. Complete the first launch

This beta is **ad hoc signed and not notarized**. If macOS blocks the app, first
confirm that you downloaded the ZIP from this repository's release. After trying
to open it, go to **System Settings → Privacy & Security → Open Anyway**, then
confirm **Open** if you choose to trust it. Follow
[Apple's app-opening guidance](https://support.apple.com/en-us/102445).

On the first launch, MK1 Revive automatically creates its user LaunchAgent and
starts the background bridge. Look for the MK1 Revive icon in the **macOS menu
bar**; a permanent Dock icon is not required.

### 5. Verify the controller connection

1. Connect the MK1 using a working USB data cable.
2. Open the menu-bar icon and check for **● Running**. If stopped, choose **Start**.
3. Choose **Controller Studio…**. Check that it reports **MK1 connected**.
4. Hit a pad and turn an encoder. The matching control should highlight and its
   pressure/value should change in the window.
5. Open or restart MASCHINE desktop software with the bridge running. Check its
   controller selection and select the MK1 if necessary.

**Running** means the service is alive; **MK1 connected** confirms that the
hardware was found. Neither alone proves that MASCHINE is receiving input.
Check a real pad response in the software. Audio input/output selection remains
in MASCHINE's normal audio preferences.

Optional: enable **Launch at Login** in the menu-bar app if you want the helper
UI to open when you log in.

### 6. Try the added features

- **Controller Studio…** shows live pads, buttons and encoders. Select a pad or
  encoder to adjust its supported sensitivity/mapping settings, then choose
  **Apply live**. Release held pads before applying changes.
- Choose **Games…** when MASCHINE is not using the controller. Select Pong,
  Freedoom or Breakout in the helper, or use the hardware arrows under
  Browse/Sampling. **Play** starts a game; **Browse** returns to selection.
- Pong gives each idle player to AI after 10 seconds. Turning that player's
  encoder takes control again. Freedoom uses the left LCD for gameplay and
  the right LCD for the original status bar; this beta has no Freedoom audio.

### Choosing a custom Doom WAD

In **Games…**, choose **Choose custom WAD…**:

- An **IWAD** becomes the base game and clears the previous add-on.
- A **PWAD** is loaded as an add-on over the current base game. Doom II map
  packs need a compatible Doom II-style IWAD; the bundled Freedoom is Phase 1.
- **Remove add-on** keeps your selected base. **Use bundled Freedoom** resets
  both choices to the included game data.

The selected WAD is validated and copied into your user Application Support
folder. Changes apply on the next Doom launch; an active Doom session returns
to game selection. Choose **Play Freedoom** to launch the selected configuration.
Classic Doom IWAD/PWAD files up to 256 MiB are supported; PK3 files and mods
requiring GZDoom features are not. No custom game data is downloaded or bundled.

### Updating an existing copy

1. Save your MASCHINE project and quit MASCHINE.
2. In MK1 Revive's menu, choose **Stop**, then **Quit**. Quitting the helper alone
   does not unload the background bridge.
3. Replace `/Applications/MK1 Revive.app` with the app from the new ZIP.
4. Launch the new app from Applications. Choose **Start** if the service is stopped,
   then repeat the controller check above and reopen MASCHINE.

Normal updates do not require deleting your settings or reinstalling MASCHINE.
Do not run the old and new bridge copies together.

### If something does not respond

- **Service stopped:** choose **Start** or **Restart** from the menu-bar app.
- **Service running, hardware missing:** reconnect the data cable and try another
  USB port/adapter, then check Controller Studio again.
- **Hardware reacts in Studio, MASCHINE does not:** restart MASCHINE after the
  bridge is running and check controller selection.
- **Games unavailable:** MASCHINE may be using the controller. Save and quit it,
  then retry Games.

For an issue report, include your macOS/MASCHINE versions, controller connection
state and relevant files from `/tmp/maschine-mk1-revive/`. Do not publish private
project files or unrelated logs.

### Uninstalling

Choose **Stop**, disable **Launch at Login** if enabled, then **Quit**. Move
`/Applications/MK1 Revive.app` to Trash. In Finder's **Go → Go to Folder…**, open
`~/Library/LaunchAgents/` and remove **only** `com.dragco.mk1-bridge.plist` to
prevent it loading at login. Keep settings unless you explicitly want a reset:
`~/Library/Application Support/MK1 Revive/` contains your controller profile.
<!-- INSTALL:END -->

[Controller Studio and game screenshots](https://github.com/oskarmust/maschine-mk1-revive#screenshots)
