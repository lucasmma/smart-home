# Uptime Kuma

## What it does
A self-hosted uptime monitor. You add "monitors" (HTTP(s), TCP port, ping, DNS,
Docker container, etc.) and it continuously checks them, records uptime % and
response time, draws status history, and can alert you (Telegram, email,
webhooks, ntfy, and 90+ others) when something goes down.

## Why you need it
Prometheus/Grafana tell you *how* a service is performing; Uptime Kuma tells you
*whether it's reachable at all* — including from the LAN's perspective — and
keeps you honest with uptime history and instant notifications.

## URLs
- `http://<SERVER_IP>:3001`

## Default credentials
**None.** On first visit you'll get a setup wizard to create the admin
username + password. Do this immediately.

## Ports
| Port | Purpose |
|------|---------|
| 3001 | web UI  |

## Persistent storage
| Path (host) | Container | Contents |
|---|---|---|
| `volumes/uptime-kuma/data/` | `/app/data` | SQLite DB (monitors, history, settings) |

## Resource usage
~100–150 MB RAM, capped at 256 MB.

## Suggested first monitors
Add one per service so a single page shows the whole stack:
| Monitor type | Target |
|---|---|
| HTTP | `http://<SERVER_IP>:3000` (Grafana) |
| HTTP | `http://<SERVER_IP>:9090/-/healthy` (Prometheus) |
| HTTP | `http://<SERVER_IP>:8081/admin` (Pi-hole) |
| HTTP | `http://<SERVER_IP>:8080/healthz` (cAdvisor) |
| DNS  | resolve `github.com` via `<SERVER_IP>` (Pi-hole DNS working) |
| Ping | `1.1.1.1` (internet reachability) |

> Because Uptime Kuma is on the `homelab` network, you can also target
> containers by name, e.g. `http://prometheus:9090/-/healthy`.

## Troubleshooting
| Symptom | Fix |
|---|---|
| Locked out / lost password | `docker exec -it uptime-kuma node extra/reset-password.js` then follow prompts. |
| Can't reach a monitor by container name | Confirm both are on `homelab` and use the container name + internal port. |
| High CPU | Too many monitors with very short intervals — raise check intervals. |
