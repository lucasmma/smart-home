import type { Config } from "../config.js";
import { fetchJson } from "./http.js";

/**
 * Speedtest Tracker source: latest download/upload/ping, plus a derived
 * "internet up" flag based on result freshness.
 *
 * Speedtest Tracker stores bandwidth in BYTES per second; the display contract
 * wants Mbps, so we convert with `* 8 / 1e6`.
 */

interface LatestResult {
  data?: {
    ping?: number | string;
    download?: number | string; // bytes/sec
    upload?: number | string; // bytes/sec
    created_at?: string;
    updated_at?: string;
  };
}

export interface SpeedtestReadout {
  download: number; // Mbps
  upload: number; // Mbps
  ping: number; // ms
  internet: boolean; // fresh result => link is considered up
}

const bytesPerSecToMbps = (v: number): number => Math.max(0, Math.round((v * 8) / 1e6));
const toNum = (v: number | string | undefined): number => {
  const n = typeof v === "string" ? Number(v) : (v ?? 0);
  return Number.isFinite(n) ? n : 0;
};

export async function fetchSpeedtest(cfg: Config): Promise<SpeedtestReadout> {
  const headers: Record<string, string> = { Accept: "application/json" };
  if (cfg.speedtest.token) headers.Authorization = `Bearer ${cfg.speedtest.token}`;

  const body = await fetchJson<LatestResult>(
    `${cfg.speedtest.url}/api/v1/results/latest`,
    { timeoutMs: cfg.sourceTimeoutMs, headers },
  );

  const d = body.data ?? {};
  const download = bytesPerSecToMbps(toNum(d.download));
  const upload = bytesPerSecToMbps(toNum(d.upload));
  const ping = Math.max(0, Math.round(toNum(d.ping)));

  // Internet is "up" if the most recent test is recent enough and produced a
  // non-zero download. A stale result likely means the tester couldn't run.
  const stamp = d.updated_at ?? d.created_at;
  const ageMs = stamp ? Date.now() - Date.parse(stamp) : Number.POSITIVE_INFINITY;
  const fresh = Number.isFinite(ageMs) && ageMs <= cfg.speedtest.maxAgeMs;
  const internet = fresh && download > 0;

  return { download, upload, ping, internet };
}
