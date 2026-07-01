# Tailscale

## What it does
Creates a private, encrypted mesh VPN (built on WireGuard) between your devices
using your Tailscale account. The Pi joins your "tailnet" and gets a stable
private IP + MagicDNS name reachable from any of your enrolled devices —
anywhere — without opening a single port on your router.

## Why you need it
Safe remote access to the whole stack. Instead of port-forwarding (risky) you
reach Grafana, Pi-hole, Homepage, etc. over the tailnet as if you were home.
It's also the recommended way to reach the LAN-only services (Dozzle, Homepage)
from outside.

## Cost
Free tier is generous (personal use: up to 100 devices, 3 users).

## URLs
No local web UI. Managed at the admin console:
- `https://login.tailscale.com/admin/machines`

## Credentials / setup
1. Create an **auth key**: https://login.tailscale.com/admin/settings/keys
   (a *reusable* key makes reinstalls repeatable). Put it in `.env` as
   `TAILSCALE_AUTHKEY`.
2. `make up` — the Pi appears in the admin console as `home-server`.

## Ports
None published. Runs in host-network mode; traffic is WireGuard (UDP 41641
outbound, auto-handled).

## Persistent storage
| Path (host) | Container | Contents |
|---|---|---|
| `volumes/tailscale/state/` | `/var/lib/tailscale` | node identity/state (so it stays logged in) |

## Resource usage
~30–60 MB RAM, capped at 128 MB.

---

## Host prerequisite: IP forwarding
Needed for kernel networking / subnet routing (automated by `install.sh`):
```bash
echo 'net.ipv4.ip_forward = 1'  | sudo tee /etc/sysctl.d/99-tailscale.conf
echo 'net.ipv6.conf.all.forwarding = 1' | sudo tee -a /etc/sysctl.d/99-tailscale.conf
sudo sysctl -p /etc/sysctl.d/99-tailscale.conf
```

## Reaching your services remotely
Once connected, from any device on your tailnet:
```
http://home-server:3000        # Grafana (MagicDNS name)
http://home-server:80          # Homepage
http://100.x.y.z:8081/admin    # Pi-hole (tailnet IP also works)
```
> Add the MagicDNS name to `HOMEPAGE_ALLOWED_HOSTS` in `.env` (e.g.
> `home-server.<tailnet>.ts.net`) so Homepage accepts it.

## Optional powers
- **Subnet router** — reach *other* LAN devices remotely: set
  `TAILSCALE_ROUTES=192.168.1.0/24`, `make restart`, then **approve the route**
  in the admin console (Machines → home-server → Edit routes).
- **Exit node** — route all your traffic through home: set
  `TAILSCALE_EXTRA_ARGS=--advertise-exit-node`, restart, approve in console.
- **Pi-hole on the go** — in the admin console (DNS settings) set your Pi-hole's
  tailnet IP as a global nameserver + enable "Override local DNS" to get
  ad-blocking on all tailnet devices, anywhere.

## Troubleshooting
| Symptom | Fix |
|---|---|
| Machine never appears | Bad/expired `TAILSCALE_AUTHKEY`; regenerate. `docker compose logs tailscale`. |
| Can't reach services over tailnet | Confirm `TS_USERSPACE=false` and host IP forwarding is on. |
| Subnet route not working | You must **approve** it in the admin console after advertising. |
| DNS acting weird | Keep `TS_ACCEPT_DNS=false` so Tailscale doesn't override Pi-hole. |
| Re-auth loop after reboot | Ensure `volumes/tailscale/state` persists and is writable. |
