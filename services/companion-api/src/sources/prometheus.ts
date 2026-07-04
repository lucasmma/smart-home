import type { Config } from "../config.js";
import { fetchJson } from "./http.js";

/**
 * Prometheus source: CPU / RAM / disk percentages plus host "up" booleans,
 * derived from node-exporter metrics via the instant-query API.
 */

interface PromInstantResponse {
  status: "success" | "error";
  data?: {
    resultType: string;
    result: Array<{ metric: Record<string, string>; value: [number, string] }>;
  };
  error?: string;
}

export interface PrometheusReadout {
  cpu: number; // 0-100
  ram: number; // 0-100
  disk: number; // 0-100
  raspberry: boolean;
  ubuntu: boolean;
}

/** Run a single instant PromQL query; returns the first sample's value or null. */
async function queryScalar(cfg: Config, promql: string): Promise<number | null> {
  const url = `${cfg.prometheus.url}/api/v1/query?query=${encodeURIComponent(promql)}`;
  const body = await fetchJson<PromInstantResponse>(url, {
    timeoutMs: cfg.sourceTimeoutMs,
  });
  if (body.status !== "success" || !body.data) {
    throw new Error(`prometheus query error: ${body.error ?? "unknown"}`);
  }
  const first = body.data.result[0];
  if (!first) return null;
  const value = Number(first.value[1]);
  return Number.isFinite(value) ? value : null;
}

const CPU_QUERY =
  '100 - (avg(rate(node_cpu_seconds_total{mode="idle"}[1m])) * 100)';
const RAM_QUERY =
  "100 * (1 - (node_memory_MemAvailable_bytes / node_memory_MemTotal_bytes))";
const diskQuery = (mountpoint: string) =>
  `100 * (1 - (node_filesystem_avail_bytes{mountpoint="${mountpoint}"} / node_filesystem_size_bytes{mountpoint="${mountpoint}"}))`;
const upQuery = (job: string) => `up{job="${job}"}`;

const clampPct = (n: number | null): number =>
  n === null ? 0 : Math.max(0, Math.min(100, Math.round(n)));

export async function fetchPrometheus(cfg: Config): Promise<PrometheusReadout> {
  const [cpu, ram, disk, raspberryUp, ubuntuUp] = await Promise.all([
    queryScalar(cfg, CPU_QUERY),
    queryScalar(cfg, RAM_QUERY),
    queryScalar(cfg, diskQuery(cfg.prometheus.diskMountpoint)),
    queryScalar(cfg, upQuery(cfg.prometheus.raspberryUpJob)),
    cfg.prometheus.ubuntuUpQuery
      ? queryScalar(cfg, cfg.prometheus.ubuntuUpQuery)
      : Promise.resolve(null),
  ]);

  return {
    cpu: clampPct(cpu),
    ram: clampPct(ram),
    disk: clampPct(disk),
    raspberry: raspberryUp === 1,
    ubuntu: ubuntuUp === 1,
  };
}
