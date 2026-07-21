#!/usr/bin/env bash
# Shared helpers for the gh-pages deploy scripts (prod + staging).
# Sourced by deploy-prod.sh and deploy-staging.sh. Not meant to run standalone.
#
# gh-pages layout on origin:
#   /            -> production (served at https://arthurboss.github.io/captain-claw-web/)
#   /staging/    -> staging    (served at .../captain-claw-web/staging/)
# Both live on the single origin/gh-pages branch. deploy-prod excludes staging/
# from its --delete; deploy-staging scopes its rsync target to staging/ only.

set -euo pipefail

REPO_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$REPO_DIR/Build_Release"
WORKTREE="/tmp/gh-pages-deploy"
GH_BRANCH="gh-pages"

# Files inside Build_Release/ that must never be published to gh-pages.
RSYNC_EXCLUDES=(
  --exclude=".git"
  --exclude="ASSETS/"
  --exclude="ASSETS.ZIP"
  --exclude="Makefile"
  --exclude="SAVES.XML"
  --exclude="config_linux.xml"
  --exclude="config_linux_release.xml"
  --exclude="startup_commands.txt"
  --exclude="ClawLauncher.exe.config"
)

log()  { printf '\033[1;34m[deploy]\033[0m %s\n' "$*"; }
warn() { printf '\033[1;33m[deploy]\033[0m %s\n' "$*" >&2; }
die()  { printf '\033[1;31m[deploy] ERROR:\033[0m %s\n' "$*" >&2; exit 1; }

# Verify the version-locked WASM artifact set exists and is fresh. openclaw.js
# calls into openclaw.wasm by address (ASM_CONSTS); a stale .js against a fresh
# .wasm crashes at boot with "ASM_CONSTS[emAsmAddr] is not a function". We guard
# by (a) requiring all three artifacts to exist and be non-empty, (b) requiring
# each to be newer than the newest C++ source under OpenClaw/, and (c) requiring
# the three to share a build window (mtimes within 10 min) so a partially
# rebuilt set is caught. Abort with instructions rather than ship a mismatch.
verify_wasm_fresh() {
  local artifacts=("$BUILD_DIR/openclaw.wasm" "$BUILD_DIR/openclaw.js" "$BUILD_DIR/openclaw.data")
  local a
  for a in "${artifacts[@]}"; do
    [ -s "$a" ] || die "Missing or empty artifact: $a
  Rebuild first:  source ~/emsdk/emsdk_env.sh && cd $REPO_DIR/build && make -j\$(nproc)"
  done

  local newest_src
  newest_src=$(find "$REPO_DIR/OpenClaw" -type f \
      \( -name '*.cpp' -o -name '*.cxx' -o -name '*.h' -o -name '*.hpp' -o -name '*.c' \) \
      -printf '%T@\n' 2>/dev/null | sort -n | tail -1)
  newest_src=${newest_src%.*}   # strip sub-second fraction

  local min="" max="" m
  for a in "${artifacts[@]}"; do
    m=$(stat -c %Y "$a")
    if [ -n "$newest_src" ] && [ "$m" -lt "$newest_src" ]; then
      die "Stale artifact: $(basename "$a") is older than the newest source in OpenClaw/.
  The deployed loader/wasm would mismatch (ASM_CONSTS crash). Rebuild first:
    source ~/emsdk/emsdk_env.sh && cd $REPO_DIR/build && make -j\$(nproc)"
    fi
    if [ -z "$min" ] || [ "$m" -lt "$min" ]; then min=$m; fi
    if [ -z "$max" ] || [ "$m" -gt "$max" ]; then max=$m; fi
  done

  if [ $(( max - min )) -gt 600 ]; then
    die "WASM artifacts span $(( max - min ))s of build time (>10min apart).
  openclaw.wasm/.js/.data look like they came from different builds and may
  mismatch at boot. Do a clean rebuild before deploying."
  fi
  log "WASM artifact set verified fresh (built within $(( max - min ))s window)."
}

# Create a fresh temp worktree tracking origin/gh-pages, with a local branch
# literally named 'gh-pages' so the global pre-push hook (which requires the
# local branch name to match the remote target) permits the push.
setup_worktree() {
  cd "$REPO_DIR"
  git worktree remove "$WORKTREE" --force 2>/dev/null || true
  git branch -D "$GH_BRANCH" 2>/dev/null || true
  git worktree prune 2>/dev/null || true
  git config --global --get-all safe.directory | grep -qx "$WORKTREE" \
    || git config --global --add safe.directory "$WORKTREE"
  log "Fetching origin/$GH_BRANCH ..."
  git fetch origin "$GH_BRANCH"
  git worktree add "$WORKTREE" -b "$GH_BRANCH" "origin/$GH_BRANCH" >/dev/null
}

cleanup_worktree() {
  cd "$REPO_DIR" 2>/dev/null || return 0
  git worktree remove "$WORKTREE" --force 2>/dev/null || true
  git branch -D "$GH_BRANCH" 2>/dev/null || true
  git worktree prune 2>/dev/null || true
}

# Commit staged content in the worktree and push. Skips cleanly if nothing
# changed. Verifies the local branch tracks the right remote before pushing.
commit_and_push() {
  local msg="$1"
  cd "$WORKTREE"
  git add -A
  if git diff --cached --quiet; then
    log "No changes to publish. Skipping commit/push."
    return 0
  fi
  git commit -m "$msg" >/dev/null
  local tracking
  tracking=$(git rev-parse --abbrev-ref --symbolic-full-name '@{u}' 2>/dev/null || echo "")
  log "Local branch: $(git symbolic-ref --short HEAD)  ->  tracking: ${tracking:-<none>}"
  log "Pushing to origin/$GH_BRANCH ..."
  git push origin "$GH_BRANCH"
  log "Pushed. GitHub Pages will update in ~1 min."
}
