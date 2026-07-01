#!/usr/bin/env bash
# =============================================================================
#  backup.sh — snapshot config + persistent data to ./backups/
#
#  Archives: .env, configs/, volumes/  (a full, self-contained snapshot).
#  Prometheus' TSDB is EXCLUDED by default (bulky and regenerable) — set
#  INCLUDE_PROMETHEUS=1 to include it.
#
#  Usage:
#     ./scripts/backup.sh                 # hot backup (no downtime)
#     STOP=1 ./scripts/backup.sh          # stop stack first (most consistent)
#     INCLUDE_PROMETHEUS=1 ./scripts/backup.sh
#     KEEP=14 ./scripts/backup.sh         # retention (default: keep 7)
# =============================================================================
set -euo pipefail

c_grn='\033[0;32m'; c_yel='\033[0;33m'; c_off='\033[0m'
info() { echo -e "${c_grn}==>${c_off} $*"; }
warn() { echo -e "${c_yel}!! ${c_off} $*"; }

REPO_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$REPO_DIR"

BACKUP_DIR="${REPO_DIR}/backups"
KEEP="${KEEP:-7}"
STAMP="$(date +%Y%m%d-%H%M%S)"
ARCHIVE="${BACKUP_DIR}/home-server-${STAMP}.tar.gz"
mkdir -p "$BACKUP_DIR"

STOPPED=0
if [ "${STOP:-0}" = "1" ]; then
  info "Stopping stack for a consistent backup..."
  docker compose stop
  STOPPED=1
else
  warn "Hot backup (stack running). For DB-consistent backups: STOP=1 ./scripts/backup.sh"
fi

# Build tar exclude list. Patterns must match the stored member names
# (we archive `.env configs volumes` relative to REPO_DIR, so no './' prefix).
EXCLUDES=(--exclude='backups')
if [ "${INCLUDE_PROMETHEUS:-0}" != "1" ]; then
  EXCLUDES+=(--exclude='volumes/prometheus/data')
  info "Excluding Prometheus TSDB (set INCLUDE_PROMETHEUS=1 to include)."
fi

info "Creating archive: ${ARCHIVE#$REPO_DIR/}"
# shellcheck disable=SC2068
tar czf "$ARCHIVE" ${EXCLUDES[@]} \
  --warning=no-file-changed \
  -C "$REPO_DIR" \
  .env configs volumes 2>/dev/null || true

if [ "$STOPPED" = "1" ]; then
  info "Restarting stack..."
  docker compose start
fi

SIZE="$(du -h "$ARCHIVE" | cut -f1)"
info "Backup complete: ${ARCHIVE#$REPO_DIR/} (${SIZE})"

# --- retention --------------------------------------------------------------
info "Applying retention (keep ${KEEP} most recent)..."
ls -1t "${BACKUP_DIR}"/home-server-*.tar.gz 2>/dev/null | tail -n +"$((KEEP + 1))" | while read -r old; do
  echo "   removing old backup: $(basename "$old")"
  rm -f "$old"
done

echo
info "To restore:  ./scripts/restore.sh ${ARCHIVE#$REPO_DIR/}"
