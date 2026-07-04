import { z } from "zod";

/**
 * Typed, validated runtime configuration.
 *
 * Every value has a sensible default aimed at running as a container on the
 * `homelab` Docker network, where Prometheus and Speedtest Tracker are
 * reachable by service name. Pi-hole runs in host-network mode, so it is
 * reached via the Pi's LAN IP (${SERVER_IP}) on its web port (8081).
 *
 * Anything blank/optional degrades gracefully — a missing source simply
 * contributes fallback values instead of crashing the aggregator.
 */
const trimmed = () => z.string().transform((s) => s.trim());
const optionalStr = () =>
  trimmed().optional().transform((s) => (s && s.length > 0 ? s : undefined));

const EnvSchema = z.object({
  PORT: z.coerce.number().int().positive().default(8090),
  HOST: trimmed().default("0.0.0.0"),

  // Optional shared bearer token. Blank => endpoint is open (trusted LAN).
  API_TOKEN: optionalStr(),

  // How long an aggregated snapshot is served before we refetch sources.
  CACHE_TTL_MS: z.coerce.number().int().nonnegative().default(2000),

  // Per-source HTTP timeout so one slow service can't stall the whole response.
  SOURCE_TIMEOUT_MS: z.coerce.number().int().positive().default(4000),

  // --- Prometheus ---
  PROM_URL: trimmed().default("http://prometheus:9090"),
  // Root filesystem mountpoint used for the disk-usage query.
  DISK_MOUNTPOINT: trimmed().default("/"),
  // Prometheus job label whose `up` value means "the Pi is alive".
  RASPBERRY_UP_JOB: trimmed().default("node-exporter"),
  // Optional `up{...}` selector for a second (Ubuntu) host. Blank => reports false.
  //   e.g. up{instance="ubuntu-box:9100"}
  UBUNTU_UP_QUERY: optionalStr(),

  // --- Pi-hole v6 ---
  PIHOLE_URL: optionalStr(), // e.g. http://192.168.1.10:8081
  PIHOLE_PASSWORD: optionalStr(),

  // --- Speedtest Tracker ---
  SPEEDTEST_URL: trimmed().default("http://speedtest-tracker"),
  SPEEDTEST_TOKEN: optionalStr(), // Sanctum API token created in the Speedtest UI
  // A speedtest result older than this is treated as "internet down".
  SPEEDTEST_MAX_AGE_MS: z.coerce
    .number()
    .int()
    .positive()
    .default(3 * 60 * 60 * 1000), // 3h (tests run hourly by default)
});

export type Config = {
  port: number;
  host: string;
  apiToken?: string;
  cacheTtlMs: number;
  sourceTimeoutMs: number;
  prometheus: {
    url: string;
    diskMountpoint: string;
    raspberryUpJob: string;
    ubuntuUpQuery?: string;
  };
  pihole: {
    url?: string;
    password?: string;
  };
  speedtest: {
    url: string;
    token?: string;
    maxAgeMs: number;
  };
};

export function loadConfig(env: NodeJS.ProcessEnv = process.env): Config {
  const parsed = EnvSchema.parse(env);
  return {
    port: parsed.PORT,
    host: parsed.HOST,
    apiToken: parsed.API_TOKEN,
    cacheTtlMs: parsed.CACHE_TTL_MS,
    sourceTimeoutMs: parsed.SOURCE_TIMEOUT_MS,
    prometheus: {
      url: parsed.PROM_URL.replace(/\/+$/, ""),
      diskMountpoint: parsed.DISK_MOUNTPOINT,
      raspberryUpJob: parsed.RASPBERRY_UP_JOB,
      ubuntuUpQuery: parsed.UBUNTU_UP_QUERY,
    },
    pihole: {
      url: parsed.PIHOLE_URL?.replace(/\/+$/, ""),
      password: parsed.PIHOLE_PASSWORD,
    },
    speedtest: {
      url: parsed.SPEEDTEST_URL.replace(/\/+$/, ""),
      token: parsed.SPEEDTEST_TOKEN,
      maxAgeMs: parsed.SPEEDTEST_MAX_AGE_MS,
    },
  };
}
