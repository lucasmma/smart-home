import type { FastifyInstance } from "fastify";
import type { Aggregator } from "../aggregator.js";
import { makeAuthHook } from "../auth.js";
import type { Config } from "../config.js";

/**
 * Routes:
 *   GET /health        — unauthenticated liveness probe (used by the healthcheck)
 *   GET /api/display   — the aggregated snapshot the ESP32 consumes
 */
export function registerRoutes(
  app: FastifyInstance,
  cfg: Config,
  aggregator: Aggregator,
): void {
  const authHook = makeAuthHook(cfg);

  app.get("/health", async () => ({ status: "ok" }));

  app.get("/api/display", { preHandler: authHook }, async (_req, reply) => {
    const data = await aggregator.getDisplay();
    // Small clients + rotating display: never let an intermediary cache this.
    reply.header("cache-control", "no-store");
    return data;
  });
}
