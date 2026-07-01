# Prometheus

## What it does
A metrics database. It periodically **scrapes** (pulls) numeric metrics from
HTTP endpoints exposed by other services, stores them as time series, and lets
you query them (PromQL). It's the data source Grafana graphs.

## Why you need it
It's the backbone of the monitoring stack. Node Exporter, cAdvisor, Pi-hole and
Speedtest all just *expose* numbers — Prometheus is what collects and retains
them so you can see CPU, RAM, temperature, disk, container usage and internet
quality over time.

## URLs
- Web UI / query browser: `http://<SERVER_IP>:9090`
- Health: `http://<SERVER_IP>:9090/-/healthy`
- Targets (what's up/down): `http://<SERVER_IP>:9090/targets`

## Default credentials
None (no auth). Keep it on the LAN only; do not expose to the internet.

## Ports
| Port | Purpose            |
|------|--------------------|
| 9090 | Web UI + query API |

## Persistent storage
| Path (host)                 | Container     | Contents        |
|-----------------------------|---------------|-----------------|
| `volumes/prometheus/data/`  | `/prometheus` | TSDB (metrics)  |
| `configs/prometheus/prometheus.yml` | `/etc/prometheus/prometheus.yml` (ro) | scrape config |

**Retention:** 15 days OR 2 GB, whichever comes first — bounded so it can't
fill the SD card.

## Resource usage
~150–300 MB RAM with a handful of targets; capped at 512 MB. Modest disk
(bounded to 2 GB).

## Reloading config
After editing `configs/prometheus/prometheus.yml`:
```bash
docker exec prometheus kill -HUP 1
# or, since --web.enable-lifecycle is on:
curl -X POST http://<SERVER_IP>:9090/-/reload
```

## Adding a scrape target
Add a block under `scrape_configs:` pointing at the container name + metrics
port (services share the `homelab` network, so DNS-by-name works), then reload.

## Troubleshooting
| Symptom | Fix |
|---|---|
| Container restarts / "permission denied" on `/prometheus` | Data dir not owned by uid 65534: `sudo chown -R 65534:65534 volumes/prometheus/data` |
| A target shows `DOWN` on `/targets` | That service isn't up yet, or wrong port/name. Expected until we add it. |
| Config change not applied | You must reload (see above); Prometheus doesn't auto-watch the file. |
| Disk usage growing | Lower `--storage.tsdb.retention.*` in `compose/prometheus.yml`. |
