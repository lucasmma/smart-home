import Fastify from "fastify";
import { Aggregator } from "./aggregator.js";
import { SnapshotCache } from "./cache.js";
import { loadConfig } from "./config.js";
import { registerRoutes } from "./routes/display.js";

async function main(): Promise<void> {
  const cfg = loadConfig();

  const app = Fastify({
    logger: {
      level: process.env.LOG_LEVEL ?? "info",
    },
  });

  const cache = new SnapshotCache(cfg.cacheTtlMs);
  const aggregator = new Aggregator(cfg, cache, app.log);

  registerRoutes(app, cfg, aggregator);

  app.log.info(
    {
      prometheus: cfg.prometheus.url,
      pihole: cfg.pihole.url ?? "(disabled)",
      speedtest: cfg.speedtest.url,
      auth: cfg.apiToken ? "bearer-token" : "open",
      cacheTtlMs: cfg.cacheTtlMs,
    },
    "companion-api config",
  );

  const shutdown = async (signal: string) => {
    app.log.info({ signal }, "shutting down");
    await app.close();
    process.exit(0);
  };
  process.on("SIGTERM", () => void shutdown("SIGTERM"));
  process.on("SIGINT", () => void shutdown("SIGINT"));

  try {
    await app.listen({ port: cfg.port, host: cfg.host });
  } catch (err) {
    app.log.error(err, "failed to start");
    process.exit(1);
  }
}

void main();
