#!/usr/bin/env bash
# Deploy Build_Release/ to the ROOT of origin/gh-pages == PRODUCTION.
#   https://arthurboss.github.io/claw-web/
#
# Safe against staging: the rsync --delete explicitly EXCLUDES staging/, so a
# production deploy never touches the /staging/ subdirectory.
#
# Usage:  scripts/deploy-prod.sh ["commit message"]
set -euo pipefail
source "$(dirname "${BASH_SOURCE[0]}")/_deploy-lib.sh"

MSG="${1:-chore: deploy master to gh-pages (production)}"

trap cleanup_worktree EXIT

verify_wasm_fresh
setup_worktree

log "Syncing Build_Release/ -> gh-pages root (preserving staging/) ..."
rsync -a --delete --exclude="staging/" "${RSYNC_EXCLUDES[@]}" \
  "$BUILD_DIR/" "$WORKTREE/"

# GitHub Pages serves / from index.html; the game entry point is claw-web.html.
cp "$BUILD_DIR/claw-web.html" "$WORKTREE/index.html"

commit_and_push "$MSG"
log "Production live at: https://arthurboss.github.io/claw-web/"
