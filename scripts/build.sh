#!/usr/bin/env zsh
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
mkdir -p "$ROOT/build" "$HOME/Library/Audio/Plug-Ins/VST3"
cd "$ROOT/build"
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES=arm64 "$ROOT"
cmake --build . --target AetherTone_VST3 AetherTone_Standalone --parallel
echo
echo "VST3 bundle (build tree):"
find "$ROOT/build" -name "AetherTone.vst3" -type d | head
echo "User VST3 folder:"
ls -ld "$HOME/Library/Audio/Plug-Ins/VST3/AetherTone.vst3" 2>/dev/null || true
