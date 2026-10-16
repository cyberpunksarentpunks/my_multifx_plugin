# AetherTone — Guitar FX & Ambient VST3

AetherTone is a modular guitar effects plugin. You can add, remove, bypass, duplicate, and drag-and-drop reorder real DSP modules. The audio path follows the order you see on screen.

This README is written for someone who is new to building plugins.

## What was checked on this Mac

| Item | Status on this machine |
|---|---|
| Chip | Apple Silicon (`arm64`, M-series) |
| macOS | 26.2 |
| Xcode | 26.6 (installed) |
| Command Line Tools | Active (`xcode-select` points at Command Line Tools) |
| C compiler | Apple Clang 21 |
| Homebrew | Installed |
| CMake | **Was missing** — installed with Homebrew (`cmake` 4.4.4) |
| Ninja | **Was missing** — installed with Homebrew (`ninja` 1.13.2) |
| JUCE | **Was not installed** — CMake downloads JUCE 8.0.8 into `build/_deps` (not system-wide) |
| User VST3 folder | Created at `~/Library/Audio/Plug-Ins/VST3` |

Nothing else was installed. Full Xcode is present but was not required to compile. JUCE is GPL-licensed; this plugin is a JUCE-based project and should be treated as GPL-3 compatible if you distribute it.

## What actually built

A **Release, native arm64 VST3** compiled successfully on this Mac:

- Build tree: `build/AetherTone_artefacts/Release/VST3/AetherTone.vst3`
- Installed copy: `~/Library/Audio/Plug-Ins/VST3/AetherTone.vst3`
- Binary type: `Mach-O 64-bit bundle arm64`
- Signature: ad-hoc (fine for local use, not Apple-notarized)
- Standalone app (for debugging only): `build/AetherTone_artefacts/Release/Standalone/AetherTone.app`

This session verified **compile, link, bundle layout, architecture, and copy into the user VST3 folder**. It did **not** open Studio One and play guitar through the plugin. You still need to rescan and listen in Studio One.

## Install in Studio One 7

1. Confirm this folder exists and contains the plugin:
   `Macintosh HD/Users/YOURNAME/Library/Audio/Plug-Ins/VST3/AetherTone.vst3`
2. Open **Studio One 7**.
3. Open **Studio One → Settings** (or Preferences) → **Locations** (plugin paths).
4. Make sure the user VST3 folder above is scanned. If Studio One has a **Rescan** / **Reset blacklist** control, use it.
5. Create an audio track, insert **AetherTone** as an insert effect.
6. Play a guitar clip or live input. You should hear processed audio, not silence.

If the plugin does not appear:

- Quit Studio One fully and reopen it.
- In Studio One’s plugin manager, check whether AetherTone is disabled or blacklisted.
- Confirm you are using the **native Apple Silicon** version of Studio One, not Rosetta-only scanning of a different format.
- The plugin is ad-hoc signed. If macOS blocks it, in Terminal run:
  `xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/VST3/AetherTone.vst3`

## How to rebuild after you change code

In Terminal:

```bash
cd ~/Documents/Coding/my_audio_plugin
./scripts/build.sh
```

Or manually:

```bash
mkdir -p build ~/Library/Audio/Plug-Ins/VST3
cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES=arm64 ..
cmake --build . --target AetherTone_VST3 --parallel
```

Then rescan plugins in Studio One. The build copies the VST3 into your user VST3 folder automatically.

Optional standalone (does not replace the VST3):

```bash
open build/AetherTone_artefacts/Release/Standalone/AetherTone.app
```

## Using the plugin

**Top bar:** preset menu, previous/next factory preset, Save, Import, Export, global Mix, Bypass.

**Chain strip:** each card is a real processor. Drag a card onto another card to reorder. Order changes the sound. Input stays first; Output stays last.

**On a card:**

- **B** bypasses that module
- **+** duplicates it (not Input/Output)
- **×** removes it (not Input/Output)
- **+** on the right of the strip adds Drive, Modulation, Delay, Reverb, Ambient, EQ, or Cabinet

**Lower panel:** knobs and menus for the selected module. Every control is connected to DSP.

**Meters** on the right show input and output peak.

### Factory presets

These load complete effect chains with real parameter values:

- Dreamscape Clean
- Huge Ambient Clean
- Shimmering Cathedral
- Infinite Delay
- Post-Hardcore Intro
- Atmospheric Breakdown
- Wide Modulated Clean
- Dark Drone
- Reverse-Like Swells
- Experimental Texture

**Save** writes to `~/Library/Application Support/AetherTone/Presets/`.  
**Export / Import** use `.aethertone` XML files. Studio One project save/reload stores the full chain through the plugin state.

### Modules

1. **Input** — gain, optional saturation, input meter  
2. **Drive** — Warm / Crunch / Fuzz / High-Gain, drive, tone, output  
3. **Modulation** — chorus, vibrato, flanger, phaser  
4. **Delay** — free time or tempo sync, feedback, mix, mono / ping-pong / reverse grains, HP/LP in the feedback path, freeze  
5. **Reverb** — 8-line FDN, decay, pre-delay, damping, size, width, mix, octave shimmer, freeze  
6. **Ambient** — input-followed pad/drone (oscillators + filtered noise + slow movement). Not a full synthesizer.  
7. **EQ** — low shelf, two peaks, high shelf  
8. **Cabinet / IR** — convolution from **your** WAV/AIFF/FLAC impulse. No commercial IRs are bundled. Until you load a file, this module is a true bypass.  
9. **Output** — gain, stereo width, safety limiter, output meter  

Global Mix blends dry input with the processed chain. A hidden safety limiter also runs on the output.

## Implemented features

- Real-time stereo (and mono) insert processing
- Modular chain with add / remove / duplicate / bypass / drag-reorder
- Processing order matches the UI
- Parameter smoothing on several controls to reduce zipper noise
- Delay/reverb freeze clamped to avoid runaway (still be careful with high feedback)
- Host state save/restore
- Factory + user presets, file export/import
- Dark resizable editor
- Native Apple Silicon VST3

## Known limitations (honest)

- **Studio One load/play was not tested in this session.** Compile and install were.
- **Mix** and **Bypass** are the only parameters the DAW can automate as named host parameters. Module knobs are stored in plugin state (they recall with the project, but they do not all appear as individual automation lanes).
- Drive has **no oversampling**, so high gain can alias.
- Reverse mode is **reverse delay grains**, not reversing the whole track.
- Shimmer is a **+12 semitone pitch shift into the reverb**, not a large granular engine.
- Ambient layer is a **simpler useful pad/drone**, not a full ambient synth.
- Cabinet IR loading can cause a short audio hitch (file load is not done in the audio callback, but it takes the processing lock).
- No bundled impulse responses (by design).
- Ad-hoc code signature; not notarized for distribution.
- JUCE splash may appear in the GPL build.

## Troubleshooting

| Problem | What to try |
|---|---|
| Plugin missing in Studio One | Rescan VST3 paths; reset blacklist; restart the DAW |
| Studio One is Intel/Rosetta and will not load arm64 | Use native Apple Silicon Studio One, or rebuild as a universal binary later |
| Silence | Uncheck Bypass; raise Input/Output gain; set Global Mix above 0; unmute the track |
| Harsh distortion | Lower Drive; change character to Warm; enable Output limiter |
| Delay/reverb explodes | Lower feedback/decay; turn Freeze off |
| Cabinet does nothing | Load your own IR WAV; mix > 0; module not bypassed |
| UI knobs do nothing after reorder | Click the card again so the lower panel rebuilds |
| Build fails after a macOS update | `xcode-select -p` should show developer tools; reinstall Command Line Tools if `clang` is missing |

## Adding a new effect later

1. Copy an existing module under `Source/dsp/modules/` (for example `DelayModule`).
2. Give it a unique `typeId` string and real `process()` code. Do not add dummy knobs.
3. Register it in `Source/dsp/ModuleFactory.cpp` (`createModule` and `allModuleTypeIds`).
4. Rebuild. The add menu and presets can then use the new type.

Architecture: `ChainProcessor` owns modules. The audio thread walks that list in order under a lock. UI edits change the same list, so reorder is not cosmetic.

## Project layout

```
CMakeLists.txt          Build recipe (fetches JUCE)
scripts/build.sh        One-command rebuild
Source/PluginProcessor.*   Audio plugin + state
Source/PluginEditor.*      Window layout
Source/dsp/                Chain + modules
Source/ui/                 Look and feel, chain strip, knobs
Source/presets/            Factory chains
```
