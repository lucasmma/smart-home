# Speedtest Tracker

## What it does
Runs Ookla Speedtest on a schedule (default hourly), stores the results
(download/upload/ping/jitter) in SQLite, and shows history in a web UI. Also
exposes the results as Prometheus metrics.

## Why you need it
Objective, timestamped proof of what your ISP actually delivers over time —
invaluable for spotting evening slowdowns or opening a support ticket. Feeds the
**Internet Quality** row on the Grafana dashboard.

## Cost
**Free.** Open-source app, free Ookla CLI, free public test servers. The
`APP_KEY` and the Prometheus token are local secrets you generate yourself — not
paid API keys.

## URLs
- `http://<SERVER_IP>:8083`

## Default credentials
- User: `admin@example.com`
- Password: `password`
- **Change these immediately** after first login (Profile → edit).

## Ports
| Port | Purpose |
|------|---------|
| 8083 | web UI (maps to container's 80) |

## Persistent storage
| Path (host) | Container | Contents |
|---|---|---|
| `volumes/speedtest-tracker/config/` | `/config` | SQLite DB + app config |

## Resource usage
~150–250 MB RAM (can spike briefly during a test), capped at 384 MB.

---

## Required setup: APP_KEY (before first boot)
Generate and put it in `.env` as `SPEEDTEST_APP_KEY`:
```bash
echo "base64:$(openssl rand -base64 32)"
```
Without a valid key the container will error on startup.

## Wiring Prometheus metrics (one-time, after first boot)
The Prometheus exporter is **disabled by default** and must be enabled in the
UI. It is served at **`/prometheus`** and gated by an **IP allow-list — not a
token**.

1. In the Speedtest UI: **Settings → Data Integration → Prometheus** → toggle
   **Enabled** on.
2. **Allowed IPs:** leave **blank** to allow any client (fine here — the port is
   LAN-only), or restrict to the Docker bridge subnet so only Prometheus can
   scrape. Find the subnet with:
   ```bash
   docker network inspect homelab -f '{{(index .IPAM.Config 0).Subnet}}'
   ```
   and paste that CIDR (e.g. `172.18.0.0/16`) into the allowed-IPs field.
3. No Prometheus reload needed (the scrape job already targets
   `speedtest-tracker:80/prometheus`). Confirm the `speedtest` target is **UP**
   at `http://<SERVER_IP>:9090/targets` (it may show `# no data available`
   until the first test runs — that's still "up").

> The endpoint returns `# no data available` until at least one speedtest has
> completed. Trigger one from the UI (**Run Speedtest**) or wait for the
> schedule.

## ⚠️ Verify the metric names (if Grafana internet panels are empty)
The dashboard assumes `speedtest_download_bits_per_second`,
`speedtest_upload_bits_per_second`, `speedtest_ping`. After a test has run, list
the real metric names and adjust
`configs/grafana/dashboards/home-server-overview.json` if they differ:
```bash
docker exec prometheus wget -qO- http://speedtest-tracker:80/prometheus \
  | grep -E '^speedtest' | cut -d'{' -f1 | sort -u
```
Edit the three panel `expr` values to match, then Grafana reloads within ~30s.

## Troubleshooting
| Symptom | Fix |
|---|---|
| Container won't start | Missing/invalid `SPEEDTEST_APP_KEY` — regenerate (above). |
| `speedtest` target DOWN | Token file missing/empty or wrong; recheck steps 1–3. |
| No results appearing | Wait for the schedule, or trigger a test from the UI. |
| Internet panels empty in Grafana | Verify metric names (above). |
| Tests failing | The Pi may be picking a bad server; set `SPEEDTEST_SERVERS` (a server ID) in `.env`/compose. |
