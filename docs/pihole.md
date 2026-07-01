# Pi-hole

## What it does
Network-wide DNS resolver and ad/tracker blocker. Every device on your LAN asks
Pi-hole for DNS; it drops requests to known ad/tracking/malware domains and
forwards everything else to an upstream resolver (Cloudflare `1.1.1.1` by
default). Pi-hole v6 also serves its own admin web UI.

## Why you need it
- Blocks ads and trackers for **every** device on the network (phones, TVs,
  IoT) without per-device software.
- Gives you visibility into what your devices are actually talking to.
- Acts as the single, controllable DNS server for your home.

## URLs
- Admin UI: `http://<SERVER_IP>:8081/admin`  (e.g. http://192.168.1.10:8081/admin)
- TLS UI:   `https://<SERVER_IP>:8443/admin`

## Default credentials
- No username; password is whatever you set in `PIHOLE_PASSWORD` (`.env`).
- Reset it any time: `docker exec -it pihole pihole setpassword`

## Ports
| Port     | Proto   | Purpose                          |
|----------|---------|----------------------------------|
| 53       | TCP+UDP | DNS (the important one)          |
| 8081     | TCP     | Admin web UI (HTTP)              |
| 8443     | TCP     | Admin web UI (HTTPS)            |
| 67       | UDP     | DHCP — only if you enable it     |

Runs in **host network mode**, so these bind directly on the Pi.

## Persistent storage
| Path (host)                          | Container       | Contents                       |
|--------------------------------------|-----------------|--------------------------------|
| `volumes/pihole/etc-pihole/`         | `/etc/pihole`   | config, gravity + FTL databases|
| `configs/pihole/dnsmasq.d/`          | `/etc/dnsmasq.d`| custom DNS records (read-only)  |

## Resource usage
~120 MB RAM idle, capped at 256 MB. Negligible CPU. DB grows slowly with query
history.

---

## ⚠️ Prerequisite: free up port 53 (Ubuntu 24.04)
`systemd-resolved` binds `127.0.0.53:53` and will block Pi-hole. Run once on the
host (this is automated by `scripts/install.sh`):

```bash
# 1. Stop resolved from occupying port 53, but keep it running for local lookups
sudo mkdir -p /etc/systemd/resolved.conf.d
printf '[Resolve]\nDNSStubListener=no\n' | \
  sudo tee /etc/systemd/resolved.conf.d/99-disable-stub.conf

# 2. Point the host's own resolver at a real upstream (so the host still works
#    even before Pi-hole is up)
sudo ln -sf /run/systemd/resolve/resolv.conf /etc/resolv.conf

sudo systemctl restart systemd-resolved

# 3. Verify nothing is on :53 anymore
sudo ss -tulpn | grep ':53' || echo "port 53 is free ✅"
```

---

## 📡 Router settings to change AFTER deployment
Pi-hole only blocks ads for devices that actually use it for DNS. Pick ONE:

**Option A — whole network (recommended):**
1. Log into your router's admin page.
2. Find **LAN / DHCP settings → DNS server**.
3. Set the **primary DNS** to your Pi's static IP (`SERVER_IP`, e.g.
   `192.168.1.10`). Leave the secondary blank, or set it to the same IP.
   > Do **not** set a public secondary (like `8.8.8.8`) — devices will
   > round-robin and bypass blocking.
4. Reboot the router (or renew DHCP leases) so clients pick up the new DNS.

**Also:** give the Pi a **static IP / DHCP reservation** in the router so its
address never changes.

**Option B — single device:** set that device's DNS manually to the Pi's IP.

### Verify it's working
```bash
# From another LAN device:
nslookup doubleclick.net <SERVER_IP>   # should return 0.0.0.0 (blocked)
nslookup github.com      <SERVER_IP>   # should resolve normally
```
Then watch queries appear in the admin dashboard.

---

## Troubleshooting
| Symptom | Fix |
|---|---|
| Container won't start, "port 53 in use" | Do the systemd-resolved step above. |
| All queries show one client IP | You're not in host mode / router NAT — confirm `network_mode: host`. |
| No internet after pointing router at Pi-hole | Pi-hole is down. Set router DNS back temporarily; `docker compose logs pihole`. |
| Forgot password | `docker exec -it pihole pihole setpassword` |
| Web UI not on :8081 | Check `FTLCONF_webserver_port` and that nothing else grabbed the port. |
| Ads still showing | Device cached old DNS — reboot it; confirm it uses the Pi via `nslookup`. |
