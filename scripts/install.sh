#!/usr/bin/env bash
# =============================================================================
#  install.sh — bootstrap a fresh Ubuntu Server 24.04 (ARM64) host for the stack
#
#  Idempotent: safe to re-run. Installs Docker + Compose, prepares the host
#  (port 53, IP forwarding), creates the shared network, seeds .env, and fixes
#  volume ownership for the uid-sensitive services.
#
#  Usage:  ./scripts/install.sh
# =============================================================================
set -euo pipefail

# --- pretty output ----------------------------------------------------------
c_grn='\033[0;32m'; c_yel='\033[0;33m'; c_red='\033[0;31m'; c_off='\033[0m'
info() { echo -e "${c_grn}==>${c_off} $*"; }
warn() { echo -e "${c_yel}!! ${c_off} $*"; }
err()  { echo -e "${c_red}xx ${c_off} $*" >&2; }

REPO_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$REPO_DIR"

if [ "$(id -u)" -eq 0 ]; then
  warn "Running as root. It's fine, but normally run as your regular user (it uses sudo)."
fi

# --- sanity: OS/arch (warn only) --------------------------------------------
info "Host: $(. /etc/os-release 2>/dev/null && echo "$PRETTY_NAME" || echo unknown) / $(uname -m)"
if ! grep -qi ubuntu /etc/os-release 2>/dev/null; then
  warn "This script targets Ubuntu. Continuing anyway."
fi

# --- 1. base packages -------------------------------------------------------
info "Installing base packages (curl, git, ca-certificates, gnupg)..."
sudo apt-get update -qq
sudo apt-get install -y -qq ca-certificates curl git gnupg

# --- 2. Docker Engine + Compose plugin --------------------------------------
if command -v docker >/dev/null 2>&1 && docker compose version >/dev/null 2>&1; then
  info "Docker + Compose already installed ($(docker --version))."
else
  info "Installing Docker Engine from the official repository..."
  sudo install -m 0755 -d /etc/apt/keyrings
  if [ ! -f /etc/apt/keyrings/docker.asc ]; then
    sudo curl -fsSL https://download.docker.com/linux/ubuntu/gpg -o /etc/apt/keyrings/docker.asc
    sudo chmod a+r /etc/apt/keyrings/docker.asc
  fi
  echo "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/docker.asc] \
https://download.docker.com/linux/ubuntu $(. /etc/os-release && echo "${VERSION_CODENAME}") stable" \
    | sudo tee /etc/apt/sources.list.d/docker.list >/dev/null
  sudo apt-get update -qq
  sudo apt-get install -y -qq docker-ce docker-ce-cli containerd.io \
    docker-buildx-plugin docker-compose-plugin
fi

# --- 3. docker group --------------------------------------------------------
if ! id -nG "$USER" | grep -qw docker; then
  info "Adding $USER to the docker group..."
  sudo usermod -aG docker "$USER"
  warn "Log out and back in (or run 'newgrp docker') for group changes to take effect."
fi

# --- 4. free port 53 (systemd-resolved) -------------------------------------
if ss -tulpn 2>/dev/null | grep -q '127.0.0.53:53'; then
  info "Freeing port 53 from systemd-resolved (needed by Pi-hole)..."
  sudo mkdir -p /etc/systemd/resolved.conf.d
  printf '[Resolve]\nDNSStubListener=no\n' \
    | sudo tee /etc/systemd/resolved.conf.d/99-disable-stub.conf >/dev/null
  sudo ln -sf /run/systemd/resolve/resolv.conf /etc/resolv.conf
  sudo systemctl restart systemd-resolved
  if ss -tulpn 2>/dev/null | grep -q ':53 '; then
    warn "Something is still on port 53 — check before starting Pi-hole."
  else
    info "Port 53 is free."
  fi
else
  info "Port 53 already free."
fi

# --- 5. IP forwarding (Tailscale subnet routing / kernel mode) --------------
info "Enabling IP forwarding (for Tailscale)..."
{
  echo 'net.ipv4.ip_forward = 1'
  echo 'net.ipv6.conf.all.forwarding = 1'
} | sudo tee /etc/sysctl.d/99-tailscale.conf >/dev/null
sudo sysctl -q -p /etc/sysctl.d/99-tailscale.conf || true

# --- 6. shared docker network -----------------------------------------------
if docker network inspect homelab >/dev/null 2>&1; then
  info "Docker network 'homelab' already exists."
else
  info "Creating docker network 'homelab'..."
  sudo docker network create homelab >/dev/null
fi

# --- 7. .env ----------------------------------------------------------------
if [ ! -f .env ]; then
  info "Creating .env from .env.example and auto-filling host values..."
  cp .env.example .env
  chmod 600 .env
  detected_ip="$(hostname -I 2>/dev/null | awk '{print $1}')"
  sed -i "s#^DATA_ROOT=.*#DATA_ROOT=${REPO_DIR}#" .env
  sed -i "s#^PUID=.*#PUID=$(id -u)#" .env
  sed -i "s#^PGID=.*#PGID=$(id -g)#" .env
  [ -n "$detected_ip" ] && sed -i "s#^SERVER_IP=.*#SERVER_IP=${detected_ip}#" .env
  [ -n "$detected_ip" ] && sed -i "s#^HOMEPAGE_ALLOWED_HOSTS=.*#HOMEPAGE_ALLOWED_HOSTS=localhost:3000,${detected_ip}#" .env
  warn "Now edit .env and set: PIHOLE_PASSWORD, GRAFANA_ADMIN_PASSWORD, SPEEDTEST_APP_KEY, TAILSCALE_AUTHKEY."
else
  info ".env already present — leaving it untouched."
fi

# --- 8. volume dirs + uid-sensitive ownership -------------------------------
info "Creating volume directories..."
mkdir -p \
  volumes/pihole/etc-pihole \
  volumes/prometheus/data \
  volumes/grafana/data \
  volumes/uptime-kuma/data \
  volumes/speedtest-tracker/config \
  volumes/tailscale/state

info "Fixing ownership for uid-sensitive services (Prometheus 65534, Grafana 472)..."
sudo chown -R 65534:65534 volumes/prometheus/data
sudo chown -R 472:472 volumes/grafana/data

# Ensure Prometheus' speedtest token is a FILE (so the bind mount isn't a dir).
if [ ! -f configs/prometheus/speedtest_token ]; then
  cp configs/prometheus/speedtest_token.example configs/prometheus/speedtest_token
  info "Seeded configs/prometheus/speedtest_token (replace with a real token later)."
fi

# --- done -------------------------------------------------------------------
echo
info "Bootstrap complete. Next steps:"
cat <<EOF
  1. Edit secrets:            nano .env
  2. (If prompted) re-login:  newgrp docker
  3. Start the stack:         make up   (or: docker compose up -d)
  4. Open Homepage:           http://<SERVER_IP>/
  5. Post-deploy tasks:       docs/pihole.md (router DNS),
                              docs/speedtest-tracker.md (metrics token)
EOF
