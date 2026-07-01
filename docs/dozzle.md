# Dozzle

## What it does
A lightweight, real-time log viewer for Docker. Open it in a browser to see
live, colorized, searchable logs for every container — no need to SSH in and
run `docker logs`. It stores nothing; logs are streamed straight from the
Docker daemon.

## Why you need it
Fast troubleshooting. When a service misbehaves you can watch its logs live
from any device on the LAN, filter across containers, and follow multiple
containers at once.

## URLs
- `http://<SERVER_IP>:8082`

## Default credentials
None by default (open on the LAN). To require a login, enable **simple auth**:
1. Uncomment `DOZZLE_AUTH_PROVIDER: "simple"` and the `users.yml` mount in
   `compose/dozzle.yml`.
2. Create `configs/dozzle/users.yml` (generate a bcrypt hash with
   `docker run --rm amir20/dozzle generate <user> --password <pass> --name "<Name>"`).
3. `make restart`.

## Ports
| Port | Purpose |
|------|---------|
| 8082 | web UI (maps to container's 8080) |

## Persistent storage
None — stateless.

## Resource usage
Tiny: ~10–20 MB RAM, capped at 128 MB.

## Security notes
- The Docker socket is mounted **read-only**; Dozzle can read logs but not
  control containers.
- Socket access is still sensitive — **keep Dozzle on the LAN, never expose it
  to the internet.** If you need remote access, reach it through Tailscale.

## Troubleshooting
| Symptom | Fix |
|---|---|
| "permission denied" on the socket | The host socket must be readable by the container; confirm `/var/run/docker.sock` exists and the mount is present. |
| No containers listed | Daemon not reachable — check the socket mount path. |
| Want persistent auth across restarts | Use simple auth with a mounted `users.yml` (above). |
