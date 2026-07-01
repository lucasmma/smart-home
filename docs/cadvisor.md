# cAdvisor (Container Advisor)

## What it does
Analyzes resource usage of running containers and exposes it in Prometheus
format: per-container CPU, memory, network and disk I/O. Also has a small
built-in web UI.

## Why you need it
Node Exporter tells you how the *Pi* is doing; cAdvisor tells you *which
container* is responsible. Essential on 4 GB RAM for spotting a service that's
leaking memory or pegging the CPU.

## URLs
- Web UI: `http://<SERVER_IP>:8080`
- Metrics: scraped by Prometheus at `cadvisor:8080/metrics`

## Default credentials
None.

## Ports
| Port | Purpose         |
|------|-----------------|
| 8080 | web UI + metrics|

## Persistent storage
None — stateless. Bind-mounts host paths **read-only** (`/`, `/sys`,
`/var/lib/docker`, `/var/run`, `/dev/disk`) plus the `/dev/kmsg` device.

## Resource usage
~60–120 MB RAM (heaviest of the exporters), capped at 256 MB. `--docker_only`
and a 30s housekeeping interval keep it modest on the Pi.

## Useful metrics
| Metric | Meaning |
|---|---|
| `container_memory_usage_bytes` | RAM per container |
| `container_cpu_usage_seconds_total` | CPU per container |
| `container_network_receive_bytes_total` | network RX per container |
| `container_fs_usage_bytes` | disk used per container |

## Troubleshooting
| Symptom | Fix |
|---|---|
| **Per-container metrics empty / only machine metrics** | Some kernels need extra privileges. In `compose/cadvisor.yml` add `privileged: true` (and you may remove the individual `devices:`/mounts). This is the documented fallback to running non-privileged. |
| High RAM usage | Keep `--docker_only=true`; raise `--housekeeping_interval`. |
| UI shows nothing | It only shows *live* data; confirm `/var/lib/docker` is mounted. |
| Port 8080 conflict | Change the published port in `compose/cadvisor.yml` (left side of `8080:8080`). |
