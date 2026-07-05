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
  /**
   * The refresh currently fanning out to sources, if any. Guarantees
   * single-flight: a burst of polls (or a slow upstream) can never launch more
   * than one concurrent fanout, which is what turned any transient slowdown
   * into a self-amplifying stampede across Prometheus/Pi-hole/Speedtest.
   */
  private inflight: Promise<DisplayData> | null = null;

  constructor(
    private readonly cfg: Config,
    private readonly cache: SnapshotCache,
    private readonly log: FastifyBaseLogger,
  ) {}

  /**
   * Returns a snapshot. Fresh cache is served directly; a stale cache is served
   * immediately (stale-while-revalidate) while a single background refresh runs.
   * Only a cold start — before any refresh has landed — awaits the fetch, so a
   * slow source never blocks the response or lets requests pile up.
   */
  async getDisplay(): Promise<DisplayData> {
    const now = Date.now();
    if (this.cache.isFresh(now)) {
      const snap = this.cache.lastGood;
      snap.epoch = Math.floor(now / 1000); // keep the clock current even from cache
      return snap;
    }

    // Kick a refresh only if one isn't already running (single-flight).
    if (!this.inflight) {
      this.inflight = this.refresh().finally(() => {
        this.inflight = null;
      });
      // A background refresh must not crash the process on rejection; the
      // awaiting cold-start path still sees the error, warm callers don't care.
      this.inflight.catch(() => {});
    }

    // Warm cache: hand back the last-good snapshot now, let the refresh land in
    // the background. Polling frequency is thus decoupled from upstream load.
    if (this.cache.hasData) {
      const snap = this.cache.lastGood;
      snap.epoch = Math.floor(now / 1000);
      return snap;
    }

    // Cold start: no snapshot yet — all concurrent callers await the one fetch.
    return this.inflight;
  }

  /** Fans out to every source once and stores the merged snapshot. */
  private async refresh(): Promise<DisplayData> {
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

    // Per-source liveness for THIS refresh. A disabled Pi-hole isn't a failure.
    next.health = {
      prometheus: prom.status === "fulfilled",
      pihole: this.cfg.pihole.url ? pihole.status === "fulfilled" : true,
      speedtest: speed.status === "fulfilled",
    };
    // Timestamp at completion, not at request time: the fanout can take up to
    // SOURCE_TIMEOUT_MS, and dating the snapshot from when data actually landed
    // is what keeps the TTL honest.
    const doneAt = Date.now();
    next.epoch = Math.floor(doneAt / 1000);

    this.cache.store(next, doneAt);
    return next;
  }
}
