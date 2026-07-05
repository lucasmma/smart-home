# companion-api

A tiny **Fastify + TypeScript** service that aggregates the homelab's state into
a single JSON document for the [Homelab Companion](../../firmware/homelab-companion)
ESP32 desk display.

The ESP32 speaks one protocol to one endpoint; this service does the messy work
of talking to Prometheus, Pi-hole and Speedtest Tracker and normalising their
data.

## The contract

```
GET /api/display  ->  200 application/json
{
  "internet": true, "raspberry": true, "ubuntu": false,
  "cpu": 22, "ram": 31, "disk": 58,          // percent 0-100
  "download": 912, "upload": 114, "ping": 7,  // Mbps, Mbps, ms
  "adsBlocked": 183922, "queries": 312912
}
```

`GET /health` is an unauthenticated liveness probe.

If `API_TOKEN` is set, `/api/display` requires `Authorization: Bearer <token>`.

## Where the data comes from

| Field                          | Source            | Detail |
|--------------------------------|-------------------|--------|
| `cpu` / `ram` / `disk`         | Prometheus        | node-exporter instant queries |
| `raspberry` / `ubuntu`         | Prometheus        | `up{...}` per host |
| `download` / `upload` / `ping` | Speedtest Tracker | public `/api/speedtest/latest` (Mbps; no token) |
| `internet`                     | Speedtest Tracker | latest result is recent **and** non-zero |
| `adsBlocked` / `queries`       | Pi-hole v6        | `/api/auth` → `/api/stats/summary` |

Each source is fetched concurrently and independently: a failing source falls
back to the **last known good** value instead of failing the whole response.
Snapshots are cached for `CACHE_TTL_MS` so frequent ESP32 polls don't hammer the
upstreams.

## Run in the homelab stack (production)

Configuration comes from the repo root `.env`. From the repo root:

```bash
docker compose up -d companion-api
curl http://<SERVER_IP>:8090/api/display | jq
```

## Run standalone (development)

```bash
cp .env.example .env      # point PROM_URL/PIHOLE_URL/SPEEDTEST_URL at the live Pi
npm install
npm run dev               # tsx watch, hot reload
curl localhost:8090/api/display | jq
```

With the upstreams unreachable you still get a well-formed object (fallback
values) and a `200` from `/health` — handy for wiring up the firmware first.

## Scripts

- `npm run dev` — hot-reloading dev server (tsx)
- `npm run build` — compile TypeScript to `dist/`
- `npm start` — run the compiled server
- `npm run typecheck` — type-check without emitting

## Configuration

See [`.env.example`](./.env.example) for every variable and its default.
