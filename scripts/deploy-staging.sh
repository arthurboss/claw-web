#!/usr/bin/env bash
# Deploy Build_Release/ to the /staging/ subdirectory of origin/gh-pages.
#   https://arthurboss.github.io/captain-claw-web/staging/
#
# Safe against production: the rsync --delete target is scoped to the staging/
# subdirectory only, so it can never remove or modify any root (production)
# file. The PWA uses relative paths (manifest scope "./", SW registers with
# scope "./"), so the staging service worker registers with scope
# /captain-claw-web/staging/ and cannot cache or interfere with production's
# root-scoped SW. staging/sw-control.json is its own kill-switch copy.
#
# Usage:  scripts/deploy-staging.sh ["commit message"]
set -euo pipefail
source "$(dirname "${BASH_SOURCE[0]}")/_deploy-lib.sh"

MSG="${1:-chore: deploy to gh-pages /staging/}"

trap cleanup_worktree EXIT

verify_wasm_fresh
setup_worktree

mkdir -p "$WORKTREE/staging"

log "Syncing Build_Release/ -> gh-pages /staging/ (root untouched) ..."
rsync -a --delete "${RSYNC_EXCLUDES[@]}" \
  "$BUILD_DIR/" "$WORKTREE/staging/"

# Entry point for /staging/ (mirrors how root uses index.html).
cp "$BUILD_DIR/captain-claw-web.html" "$WORKTREE/staging/index.html"

commit_and_push "$MSG"
log "Staging live at: https://arthurboss.github.io/captain-claw-web/staging/"
