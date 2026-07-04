import { z } from "zod";

/**
 * The single contract shared with the ESP32 firmware.
 *
 * `GET /api/display` returns exactly this shape. The firmware's
 * `models/DashboardData.h` struct mirrors these fields 1:1, so keep the two in
 * sync when either changes.
 *
 *   - percentages (`cpu`, `ram`, `disk`) are integers 0-100
 *   - `download` / `upload` are Mbps (rounded)
 *   - `ping` is milliseconds (rounded)
 *   - `adsBlocked` / `queries` are cumulative counters for the current day
 */
export const DisplayDataSchema = z.object({
  internet: z.boolean(),
  raspberry: z.boolean(),
  ubuntu: z.boolean(),
  cpu: z.number().int().min(0).max(100),
  ram: z.number().int().min(0).max(100),
  disk: z.number().int().min(0).max(100),
  download: z.number().int().min(0),
  upload: z.number().int().min(0),
  ping: z.number().int().min(0),
  adsBlocked: z.number().int().min(0),
  queries: z.number().int().min(0),
});

export type DisplayData = z.infer<typeof DisplayDataSchema>;

/**
 * A safe, all-zero / all-down snapshot. Used as the very first "last known
 * good" value before any source has ever succeeded, so the endpoint can answer
 * immediately on cold start without a 500.
 */
export const EMPTY_DISPLAY: DisplayData = {
  internet: false,
  raspberry: false,
  ubuntu: false,
  cpu: 0,
  ram: 0,
  disk: 0,
  download: 0,
  upload: 0,
  ping: 0,
  adsBlocked: 0,
  queries: 0,
};
