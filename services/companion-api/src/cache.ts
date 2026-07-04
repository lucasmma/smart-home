import { EMPTY_DISPLAY, type DisplayData } from "./types.js";

/**
 * Holds the last successfully aggregated snapshot and a short TTL.
 *
 * Two jobs:
 *   1. Serve a cached snapshot within `ttlMs` so bursts of ESP32 polls don't
 *      hammer the upstream services.
 *   2. Provide a "last known good" value that the aggregator falls back to when
 *      an individual source fails — mirroring the firmware's own rule of
 *      keeping the last known data on HTTP failure.
 */
export class SnapshotCache {
  private snapshot: DisplayData = { ...EMPTY_DISPLAY };
  private updatedAt = 0;

  constructor(private readonly ttlMs: number) {}

  /** The freshest snapshot we have (may be the cold-start empty value). */
  get lastGood(): DisplayData {
    return this.snapshot;
  }

  /** True while the current snapshot is still within its TTL. */
  isFresh(nowMs: number): boolean {
    return this.updatedAt !== 0 && nowMs - this.updatedAt < this.ttlMs;
  }

  store(data: DisplayData, nowMs: number): void {
    this.snapshot = data;
    this.updatedAt = nowMs;
  }
}
