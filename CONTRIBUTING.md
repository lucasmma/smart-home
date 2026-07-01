# Contributing

Thanks for improving **home-server**! This repo is infrastructure-as-code, so
the bar is: *everything reproducible, nothing manual, no secrets in git.*

## Ground rules

- **No secrets in commits.** Secrets live in `.env` (git-ignored). Add new keys
  to `.env.example` with a placeholder and a comment on how to generate them.
- **Config is code, data is not.** Version-controlled config → `configs/`.
  Runtime data → `volumes/` (git-ignored).
- **Pin image versions.** Use a real tag in `.env.example` (not `latest`), and
  keep a matching `${SERVICE_VERSION:-<tag>}` fallback in the compose file.

## Adding a service

1. Create `compose/<service>.yml`. Copy an existing file and keep the house
   style (see checklist below).
2. Config under `configs/<service>/`, data under `volumes/<service>/`.
3. Add `SERVICE_VERSION` + any secrets to `.env.example`.
4. Add an `include:` line in `docker-compose.yml`.
5. Add a card to `configs/homepage/services.yaml`.
6. Write `docs/<service>.md` (what / why / creds / URLs / ports / storage /
   resources / troubleshooting).
7. Link it in the `README.md` service table.

### Compose house style (checklist)

Every service should have:

- [ ] `image:` pinned via `${SERVICE_VERSION:-<tag>}`
- [ ] `container_name:` + `hostname:`
- [ ] `restart: unless-stopped`
- [ ] `mem_limit:` (this is a 4 GB Pi — always cap memory)
- [ ] `security_opt: [no-new-privileges:true]`; least-privilege `cap_add`
      (avoid `privileged:` — document a fallback if truly needed)
- [ ] `healthcheck:` where the image supports one
- [ ] `logging:` json-file with `max-size`/`max-file` (rotation)
- [ ] the shared network block, unless it genuinely needs host networking
      (document *why* if so — see Pi-hole/Tailscale):
      ```yaml
      networks:
        homelab:
          external: true
          name: homelab
      ```
- [ ] bind mounts under `${DATA_ROOT}/volumes/<service>/…`
- [ ] read-only (`:ro`) mounts for anything the container shouldn't write

## Validate before you push

CI runs these automatically; run them locally first:

```bash
cp .env.example .env
docker compose config -q          # merged config is valid
docker compose config --services  # your service shows up
shellcheck -S warning scripts/*.sh
```

## Commits & PRs

- Small, focused commits; imperative messages (e.g. "Add Jellyfin service").
- One service or concern per PR where possible.
- Update the relevant `docs/*.md` and the README in the same PR.
- CI (`.github/workflows/ci.yml`) must be green: Compose validates, ShellCheck
  passes, all YAML/JSON parses.
