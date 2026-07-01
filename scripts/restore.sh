#!/usr/bin/env bash
# =============================================================================
#  restore.sh — restore config + data from a backup archive
#
#  ⚠️  Overwrites .env, configs/ and volumes/ with the archive's contents.
#
#  Usage:  ./scripts/restore.sh backups/home-server-YYYYMMDD-HHMMSS.tar.gz
# =============================================================================
set -euo pipefail

c_grn='\033[0;32m'; c_yel='\033[0;33m'; c_red='\033[0;31m'; c_off='\033[0m'
info() { echo -e "${c_grn}==>${c_off} $*"; }
warn() { echo -e "${c_yel}!! ${c_off} $*"; }
err()  { echo -e "${c_red}xx ${c_off} $*" >&2; }

REPO_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$REPO_DIR"

ARCHIVE="${1:-}"
if [ -z "$ARCHIVE" ] || [ ! -f "$ARCHIVE" ]; then
  err "Usage: ./scripts/restore.sh <path-to-backup.tar.gz>"
  echo "Available backups:"
  ls -1t backups/home-server-*.tar.gz 2>/dev/null || echo "  (none found)"
  exit 1
fi

warn "This will OVERWRITE .env, configs/ and volumes/ from:"
echo "     $ARCHIVE"
read -r -p "Type 'yes' to continue: " confirm
[ "$confirm" = "yes" ] || { info "Aborted."; exit 0; }

# Show what's inside before touching anything.
info "Archive contents (top level):"
tar tzf "$ARCHIVE" | awk -F/ '{print $1}' | sort -u | sed 's/^/   /'

info "Stopping stack..."
docker compose down || true

info "Extracting archive into repo root..."
tar xzf "$ARCHIVE" -C "$REPO_DIR"

# Re-assert ownership for uid-sensitive services (tar may not preserve it).
info "Re-applying volume ownership (Prometheus 65534, Grafana 472)..."
[ -d volumes/prometheus/data ] && sudo chown -R 65534:65534 volumes/prometheus/data
[ -d volumes/grafana/data ]    && sudo chown -R 472:472 volumes/grafana/data

# Ensure the shared network exists (in case this is a fresh host).
docker network inspect homelab >/dev/null 2>&1 || docker network create homelab >/dev/null

info "Starting stack..."
docker compose up -d --remove-orphans

echo
info "Restore complete. Status:"
docker compose ps
