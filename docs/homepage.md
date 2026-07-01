# Homepage

## What it does
A fast, file-configured dashboard that acts as the front door to your server. It
shows a card for every service (with a clickable link) and live-status widgets
for several of them (Pi-hole block %, Grafana, Prometheus, latest Speedtest).

## Why you need it
One bookmark to rule them all. Instead of remembering ports, you open the Pi's
address and click through to anything — with at-a-glance health on the page.

## URLs
- `http://<SERVER_IP>/`  (port 80 — the front door)

## Default credentials
None (no auth). Keep on the LAN; expose remotely only via Tailscale.

## Ports
| Port | Purpose |
|------|---------|
| 80   | web UI (maps to container's 3000) |

## Persistent storage
| Path (host) | Container | Contents |
|---|---|---|
| `configs/homepage/` | `/app/config` (rw) | all Homepage YAML config |

> Mounted read-write because Homepage creates any missing config files on first
> run. All files are provided, so it won't need to.

## Resource usage
~80–150 MB RAM, capped at 256 MB.

## Configuration
| File | Purpose |
|---|---|
| `settings.yaml` | title, theme, group layout |
| `services.yaml` | the service cards + widgets |
| `widgets.yaml` | top bar (greeting, clock, search) |
| `bookmarks.yaml` | quick external links |
| `docker.yaml` | optional socket discovery (disabled) |

- URLs use `{{HOMEPAGE_VAR_SERVER_IP}}`, injected from `.env` — no hardcoded IP.
- Widget credentials come from `HOMEPAGE_VAR_*` env vars (also from `.env`), so
  no secrets live in the committed YAML.

### `HOMEPAGE_ALLOWED_HOSTS` (important)
Recent Homepage rejects requests whose `Host` header isn't allow-listed. Set
`HOMEPAGE_ALLOWED_HOSTS` in `.env` to include exactly how you reach it:
- LAN on port 80 → the bare IP, e.g. `192.168.1.10`
- via Tailscale → add the MagicDNS name, e.g. `pi.tailnet-name.ts.net`

### Adding a new service to the dashboard
Add an entry under the right group in `services.yaml` (href + icon +
description; optional `widget`). Changes apply on refresh — Homepage watches the
config.

### Optional: auto-discover containers via Docker labels
See `configs/homepage/docker.yaml` for how to enable socket-based discovery with
`homepage.*` labels instead of the static list.

## Troubleshooting
| Symptom | Fix |
|---|---|
| "Host validation failed" / blank page | Add your address to `HOMEPAGE_ALLOWED_HOSTS` and `make restart`. |
| A widget shows an error, but the link works | Check that service's credentials/URL; a broken widget never breaks the page. |
| Pi-hole widget empty | v6 uses the web password as the API key — confirm `PIHOLE_PASSWORD` is set. |
| Icons missing | Icons load from a CDN — needs internet; or use `mdi-<name>` icons. |
| Port 80 in use | Something else grabbed 80 (Pi-hole's UI is on 8081, not 80). Change the left side of `80:3000`. |
