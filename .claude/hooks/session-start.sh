#!/bin/bash
# SessionStart hook for Claude Code cloud sessions (CLAUDE_CODE_REMOTE=true).
# Installs the toolchain (arm-none-eabi, libmgba, libpng, Pillow), sets up a
# devkitPro stand-in under ~/.cache/c-skillsys (grit/gbalzss -> Tools/cloud/gritshim.py,
# arm-none-eabi-* symlinks), builds the gbarun headless emulator runner, fetches the
# pinned Tools/check_patch and Tools/FE-PyTools checkouts, and locates the FE8U ROM
# from user uploads. Idempotent. See Tools/cloud/README.md.
set -euo pipefail

[ "${CLAUDE_CODE_REMOTE:-}" = "true" ] || exit 0

ROOT="${CLAUDE_PROJECT_DIR:-$(git rev-parse --show-toplevel)}"
CACHE="$HOME/.cache/c-skillsys"
DKP="$CACHE/devkitpro"

log() { echo "[session-start] $*" >&2; }

# 1. apt packages
PKGS="gcc-arm-none-eabi binutils-arm-none-eabi libnewlib-arm-none-eabi libmgba-dev libpng-dev"
missing=""
for p in $PKGS; do
  dpkg -s "$p" >/dev/null 2>&1 || missing="$missing $p"
done
if [ -n "$missing" ]; then
  log "installing apt packages:$missing"
  export DEBIAN_FRONTEND=noninteractive
  apt-get update -qq >/dev/null || true
  apt-get install -y -qq --no-install-recommends $missing >/dev/null || log "WARNING: apt-get install failed for:$missing"
fi

# 2. Pillow
if ! python3 -c "import PIL" >/dev/null 2>&1; then
  log "installing Pillow"
  python3 -m pip install -q Pillow || python3 -m pip install -q --break-system-packages Pillow || log "WARNING: could not install Pillow"
fi

# 3. devkitPro stand-in
mkdir -p "$CACHE" "$DKP/devkitARM/bin" "$DKP/tools/bin"
for f in /usr/bin/arm-none-eabi-*; do
  [ -e "$f" ] || continue
  ln -sf "$f" "$DKP/devkitARM/bin/$(basename "$f")"
done
ln -sf "$ROOT/Tools/cloud/gritshim.py" "$DKP/tools/bin/grit"
ln -sf "$ROOT/Tools/cloud/gritshim.py" "$DKP/tools/bin/gbalzss"

# 4. gbarun headless emulator runner
if [ ! -e "$CACHE/gbarun" ] || [ "$ROOT/Tools/cloud/gbarun.c" -nt "$CACHE/gbarun" ]; then
  log "building gbarun"
  gcc -O2 -o "$CACHE/gbarun" "$ROOT/Tools/cloud/gbarun.c" -lmgba -lpng || log "WARNING: could not build gbarun"
fi
[ -e "$CACHE/gbarun" ] && ln -sf "$CACHE/gbarun" "$DKP/tools/bin/gbarun" || true

# 5. environment for later Bash calls
if [ -n "${CLAUDE_ENV_FILE:-}" ]; then
  {
    echo "export DEVKITPRO=\"$DKP\""
    echo "export DEVKITARM=\"$DKP/devkitARM\""
    echo "export PATH=\"$DKP/tools/bin:\$PATH\""
  } >> "$CLAUDE_ENV_FILE"
fi

# 6. pinned tool checkouts (gitlinks with no .gitmodules come out of a clone empty)
fetch_pinned() {
  local dir="$1" url="$2" sha
  sha=$(git -C "$ROOT" ls-tree HEAD "$dir" | awk '{print $3}')
  [ -n "$sha" ] || return 0
  if [ ! -e "$ROOT/$dir/.git" ]; then
    log "fetching $dir @ $sha"
    rmdir "$ROOT/$dir" 2>/dev/null || true
    git clone -q --recursive "$url" "$ROOT/$dir" && git -C "$ROOT/$dir" checkout -q "$sha" \
      || log "WARNING: could not fetch $dir from $url"
  fi
}
fetch_pinned Tools/check_patch https://github.com/MokhaLeee/check_patch.git
fetch_pinned Tools/FE-PyTools https://github.com/StanHash/FE-PyTools.git

# 7. FE8U ROM from user uploads
FE8_SHA1=c25b145e37456171ada4b0d440bf88a19f4d509f
if [ ! -f "$ROOT/fe8.gba" ]; then
  while IFS= read -r f; do
    if [ "$(sha1sum "$f" | awk '{print $1}')" = "$FE8_SHA1" ]; then
      log "using $f as fe8.gba"
      cp "$f" "$ROOT/fe8.gba"
      break
    fi
  done < <(find "$HOME/.claude/uploads" "$ROOT" -maxdepth 3 -name '*.gba' -size 16384k 2>/dev/null || true)
fi

# 8. summary (stdout becomes session context)
echo "C-SkillSystem cloud setup done."
echo "Build: Tools/cloud/build.sh (prints BUILD rc=N; log in /tmp/c-skillsys-build.log)."
echo "Emulator: gbarun ROM SCRIPT (see Tools/cloud/README.md)."
if [ -f "$ROOT/fe8.gba" ]; then
  echo "ROM: fe8.gba present."
else
  echo "ROM: fe8.gba MISSING - ask the user to upload a clean FE8U ROM (sha1 c25b145e...)."
fi
