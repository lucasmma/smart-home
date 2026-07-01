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
1. In the Speedtest UI: **Settings → API Tokens** (or Profile → API) → create a
   token. Copy it.
2. On the Pi, write it to the token file Prometheus reads:
   ```bash
   cp configs/prometheus/speedtest_token.example configs/prometheus/speedtest_token
   printf '%s' '<PASTE_TOKEN>' > configs/prometheus/speedtest_token
   ```
   (This file is git-ignored.)
3. Reload Prometheus: `docker exec prometheus kill -HUP 1`
4. Confirm the `speedtest` target is **UP** at `http://<SERVER_IP>:9090/targets`.

## ⚠️ Verify the metric names (if Grafana internet panels are empty)
The dashboard assumes these metric names:
`speedtest_download_bits_per_second`, `speedtest_upload_bits_per_second`,
`speedtest_ping`. If your image version names them differently, list the real
ones and adjust `configs/grafana/dashboards/home-server-overview.json`:
```bash
curl -s -H "Authorization: Bearer <TOKEN>" \
  http://<SERVER_IP>:8083/api/v1/prometheus | grep -E '^speedtest' | cut -d'{' -f1 | sort -u
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
