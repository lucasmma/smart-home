#!/usr/bin/env bash
# =============================================================================
#  fetch-dashboards.sh  (OPTIONAL)
#
#  Downloads community dashboards from grafana.com, rewrites their datasource
#  placeholder to our provisioned uid ("prometheus"), and drops the JSON into
#  configs/grafana/dashboards/ so Grafana auto-provisions them — no manual
#  import through the UI.
#
#  Usage:
#     ./scripts/fetch-dashboards.sh [ID ...]
#  Defaults to Node Exporter Full (1860) if no IDs are given.
#
#  After running, reload Grafana (it re-scans every 30s, or `make restart`).
# =============================================================================
set -euo pipefail

REPO_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DEST="${REPO_DIR}/configs/grafana/dashboards"
DS_UID="prometheus"

# id -> friendly filename
declare -A KNOWN=(
  [1860]="node-exporter-full"
  [19792]="cadvisor"
)

IDS=("$@")
if [ ${#IDS[@]} -eq 0 ]; then
  IDS=(1860)
fi

mkdir -p "$DEST"

for id in "${IDS[@]}"; do
  name="${KNOWN[$id]:-dashboard-$id}"
  out="${DEST}/${name}.json"
  url="https://grafana.com/api/dashboards/${id}/revisions/latest/download"
  echo "→ Fetching #${id} (${name}) ..."
  # Download, then replace the DS_PROMETHEUS input variable with our fixed uid.
  if curl -fsSL "$url" \
      | sed -e 's/\${DS_PROMETHEUS}/'"${DS_UID}"'/g' \
            -e 's/"\${datasource}"/"'"${DS_UID}"'"/g' \
      > "$out"; then
    echo "  saved → ${out#$REPO_DIR/}"
  else
    echo "  ✗ failed to fetch #${id}" >&2
    rm -f "$out"
  fi
done

echo "Done. Reload Grafana to provision (auto within ~30s, or 'make restart')."
