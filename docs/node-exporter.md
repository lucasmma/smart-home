# Node Exporter

## What it does
Exposes the Raspberry Pi's hardware and OS metrics in Prometheus format: CPU
load, RAM/swap, disk space & I/O, filesystem usage, network throughput, uptime,
and **CPU temperature** (via the `thermal_zone` collector reading
`/sys/class/thermal`).

## Why you need it
It's the source of every "how is the Pi itself doing?" number — the RAM, CPU,
disk and temperature panels in Grafana all come from here.

## URLs
Not published to the LAN by design. Reachable only inside the Docker network by
Prometheus at `node-exporter:9100`. To inspect manually:
```bash
docker exec prometheus wget -qO- http://node-exporter:9100/metrics | head
```

## Default credentials
None.

## Ports
| Port | Purpose | Exposed? |
|------|---------|----------|
| 9100 | metrics | internal only (not published) |

## Persistent storage
None — it's stateless. It bind-mounts host paths **read-only**:
`/proc`, `/sys`, `/` (as `/rootfs`).

## Resource usage
Tiny: ~15–25 MB RAM, capped at 128 MB. Negligible CPU.

## Useful metrics
| Metric | Meaning |
|---|---|
| `node_thermal_zone_temp` | CPU temperature (°C) |
| `node_memory_MemAvailable_bytes` | free RAM |
| `node_filesystem_avail_bytes` | free disk per mount |
| `node_load1` / `node_load5` | load average |
| `node_network_receive_bytes_total` | network RX (host interfaces) |

## Troubleshooting
| Symptom | Fix |
|---|---|
| No temperature metric | Confirm `/sys` is mounted; check `ls /sys/class/thermal/` on the host. |
| Target DOWN in Prometheus | `docker compose logs node-exporter`; confirm both share the `homelab` network. |
| Disk metrics missing a mount | Adjust `--collector.filesystem.mount-points-exclude` in `compose/node-exporter.yml`. |
