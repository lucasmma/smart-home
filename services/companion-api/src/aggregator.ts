import type { FastifyBaseLogger } from "fastify";
import type { Config } from "./config.js";
import { SnapshotCache } from "./cache.js";
import { fetchPihole } from "./sources/pihole.js";
import { fetchPrometheus } from "./sources/prometheus.js";
import { fetchSpeedtest } from "./sources/speedtest.js";
import type { DisplayData } from "./types.js";

/**
 * Fans out to every source concurrently and merges the results into the single
 * DisplayData contract. Each source is independent: `Promise.allSettled` means
 * a failing source never fails the whole response — its fields fall back to the
 * last known good snapshot, and the failure is logged.
 */
export class Aggregator {
  constructor(
    private readonly cfg: Config,
    private readonly cache: SnapshotCache,
    private readonly log: FastifyBaseLogger,
  ) {}

  /** Returns a snapshot, refetching sources only when the cache has gone stale. */
  async getDisplay(): Promise<DisplayData> {
    const now = Date.now();
    if (this.cache.isFresh(now)) return this.cache.lastGood;

    const prev = this.cache.lastGood;

    const [prom, pihole, speed] = await Promise.allSettled([
      fetchPrometheus(this.cfg),
      this.cfg.pihole.url ? fetchPihole(this.cfg) : Promise.reject(new Error("pihole disabled")),
      fetchSpeedtest(this.cfg),
    ]);

    const next: DisplayData = { ...prev };

    if (prom.status === "fulfilled") {
      next.cpu = prom.value.cpu;
      next.ram = prom.value.ram;
      next.disk = prom.value.disk;
      next.raspberry = prom.value.raspberry;
      next.ubuntu = prom.value.ubuntu;
    } else {
      this.log.warn({ err: prom.reason?.message ?? prom.reason }, "prometheus source failed");
    }

    if (pihole.status === "fulfilled") {
      next.adsBlocked = pihole.value.adsBlocked;
      next.queries = pihole.value.queries;
    } else if (this.cfg.pihole.url) {
      this.log.warn({ err: pihole.reason?.message ?? pihole.reason }, "pihole source failed");
    }

    if (speed.status === "fulfilled") {
      next.download = speed.value.download;
      next.upload = speed.value.upload;
      next.ping = speed.value.ping;
      next.internet = speed.value.internet;
    } else {
      this.log.warn({ err: speed.reason?.message ?? speed.reason }, "speedtest source failed");
    }

    this.cache.store(next, now);
    return next;
  }
}
