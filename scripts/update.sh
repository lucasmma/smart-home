#!/usr/bin/env bash
# =============================================================================
#  update.sh — pull the latest images and recreate changed containers
#
#  Only services whose image actually changed are recreated (Compose handles
#  this). Dangling images are pruned to reclaim SD-card space.
#
#  Usage:  ./scripts/update.sh
#  Tip: pin versions in .env for reproducible updates instead of :latest.
# =============================================================================
set -euo pipefail

c_grn='\033[0;32m'; c_off='\033[0m'
info() { echo -e "${c_grn}==>${c_off} $*"; }

REPO_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$REPO_DIR"

# Optional: sync the repo itself if it's a git checkout with a remote.
if [ -d .git ] && git remote get-url origin >/dev/null 2>&1; then
  info "Pulling latest repo changes..."
  git pull --ff-only || info "(skipped git pull — resolve manually if needed)"
fi

info "Backing up before update (safety net)..."
"${REPO_DIR}/scripts/backup.sh" || info "(backup skipped/failed — continuing)"

info "Pulling latest images..."
docker compose pull

info "Recreating changed containers..."
docker compose up -d --remove-orphans

info "Pruning dangling images..."
docker image prune -f >/dev/null

echo
info "Update complete. Current status:"
docker compose ps
