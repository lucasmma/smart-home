# Grafana

## What it does
Visualization layer. It queries Prometheus and renders dashboards — the graphs
for CPU, RAM, temperature, disk, network, per-container usage and (later)
internet quality.

## Why you need it
Prometheus stores numbers but is clumsy to read. Grafana turns them into the
at-a-glance dashboards you'll actually look at.

## Zero-import provisioning (how it works)
Everything is configured from files on disk — you never import anything by hand:
- **Datasource**: `configs/grafana/provisioning/datasources/datasource.yml`
  wires Prometheus with a fixed `uid: prometheus`.
- **Dashboard provider**: `configs/grafana/provisioning/dashboards/provider.yml`
  auto-loads every `*.json` under `configs/grafana/dashboards/` into a
  "Home Server" folder, rescanning every 30s.
- **Dashboard**: `configs/grafana/dashboards/home-server-overview.json` — a
  self-contained overview (CPU, RAM, temp, disk, uptime, container count, load,
  memory, network, disk space, per-container CPU & memory).

### Adding rich community dashboards (optional)
```bash
./scripts/fetch-dashboards.sh 1860        # Node Exporter Full
./scripts/fetch-dashboards.sh 1860 19792  # + cAdvisor
```
This downloads them and rewrites their datasource to our `prometheus` uid, so
they provision automatically (no UI import). Reload within ~30s or `make restart`.

## URLs
- `http://<SERVER_IP>:3000`  → dashboards live under **Dashboards → Home Server**

## Default credentials
- User: `GRAFANA_ADMIN_USER` (default `admin`)
- Password: `GRAFANA_ADMIN_PASSWORD` (from `.env`)
- Sign-up and anonymous access are disabled.

## Ports
| Port | Purpose |
|------|---------|
| 3000 | web UI  |

## Persistent storage
| Path (host) | Container | Contents |
|---|---|---|
| `volumes/grafana/data/` | `/var/lib/grafana` | Grafana DB (users, prefs, dashboard state) |
| `configs/grafana/provisioning/` | `/etc/grafana/provisioning` (ro) | datasource + provider config |
| `configs/grafana/dashboards/` | `/var/lib/grafana/dashboards` (ro) | dashboard JSON |

## Resource usage
~120–200 MB RAM, capped at 256 MB.

## Troubleshooting
| Symptom | Fix |
|---|---|
| Container restarts / "permission denied" | Data dir must be owned by uid 472: `sudo chown -R 472:472 volumes/grafana/data` |
| Dashboard panels say "No data" | Prometheus targets are DOWN, or you're viewing before data accrued — check `http://<SERVER_IP>:9090/targets`. |
| "Datasource not found" on a community dashboard | Re-run `fetch-dashboards.sh` (it rewrites the datasource uid). |
| Forgot admin password | `docker exec -it grafana grafana cli admin reset-admin-password '<newpass>'` |
| Temperature panel empty | node-exporter isn't exposing `node_thermal_zone_temp` — see docs/node-exporter.md. |
