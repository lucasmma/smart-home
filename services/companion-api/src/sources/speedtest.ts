import type { Config } from "../config.js";
import { fetchJson } from "./http.js";

/**
 * Speedtest Tracker source: latest download/upload/ping, plus a derived
 * "internet up" flag based on result freshness.
 *
 * We use the unauthenticated legacy endpoint `/api/speedtest/latest` (the same
 * one Homepage's widget uses). Unlike the token-gated `/api/v1/results/latest`,
 * it needs no API token AND already reports speeds in Mbps, so no conversion.
 */

interface LatestResult {
  data?: {
    ping?: number | string;
    download?: number | string; // Mbps
    upload?: number | string;   // Mbps
    failed?: boolean;
    created_at?: string;
    updated_at?: string;
  };
}

export interface SpeedtestReadout {
  download: number; // Mbps
  upload: number; // Mbps
  ping: number; // ms
  internet: boolean; // fresh, non-failed result => link is considered up
}

const toNum = (v: number | string | undefined): number => {
  const n = typeof v === "string" ? Number(v) : (v ?? 0);
  return Number.isFinite(n) ? n : 0;
};
const mbps = (v: number | string | undefined): number => Math.max(0, Math.round(toNum(v)));

export async function fetchSpeedtest(cfg: Config): Promise<SpeedtestReadout> {
  const headers: Record<string, string> = { Accept: "application/json" };
  // Token is optional — the legacy endpoint is public — but pass it if set.
  if (cfg.speedtest.token) headers.Authorization = `Bearer ${cfg.speedtest.token}`;

  const body = await fetchJson<LatestResult>(
    `${cfg.speedtest.url}/api/speedtest/latest`,
    { timeoutMs: cfg.sourceTimeoutMs, headers },
  );

  const d = body.data ?? {};
  const download = mbps(d.download);
  const upload = mbps(d.upload);
  const ping = Math.max(0, Math.round(toNum(d.ping)));

  // Internet is "up" if the latest test succeeded, is recent enough, and
  // produced a non-zero download. A stale/failed result likely means the
  // tester couldn't reach the internet.
  const stamp = d.updated_at ?? d.created_at;
  const ageMs = stamp ? Date.now() - Date.parse(stamp) : Number.POSITIVE_INFINITY;
  const fresh = Number.isFinite(ageMs) && ageMs <= cfg.speedtest.maxAgeMs;
  const internet = fresh && d.failed !== true && download > 0;

  return { download, upload, ping, internet };
}
