import type { Config } from "../config.js";
import { fetchJson, HttpError } from "./http.js";

/**
 * Pi-hole v6 source: ads blocked + total queries for the current day.
 *
 * The v6 REST API is session-based: POST the app password to /api/auth to get a
 * SID, then pass it as the `X-FTL-SID` header on subsequent calls. We cache the
 * SID and transparently re-authenticate when it expires (a 401 clears it).
 */

interface AuthResponse {
  session?: { valid: boolean; sid?: string; validity?: number };
}

interface SummaryResponse {
  queries?: { total?: number; blocked?: number };
}

export interface PiholeReadout {
  adsBlocked: number;
  queries: number;
}

// Module-level SID cache. Keyed only by base URL — a single Pi-hole per deploy.
let cachedSid: { sid: string; expiresAt: number } | null = null;

// `Date.now()` is fine in the backend (this runs on the Pi, not in a workflow).
function now(): number {
  return Date.now();
}

async function authenticate(cfg: Config): Promise<string> {
  if (!cfg.pihole.url || !cfg.pihole.password) {
    throw new Error("pihole not configured (PIHOLE_URL / PIHOLE_PASSWORD)");
  }
  const body = await fetchJson<AuthResponse>(`${cfg.pihole.url}/api/auth`, {
    method: "POST",
    timeoutMs: cfg.sourceTimeoutMs,
    headers: { "content-type": "application/json" },
    body: JSON.stringify({ password: cfg.pihole.password }),
  });
  const session = body.session;
  if (!session?.valid || !session.sid) {
    throw new Error("pihole auth rejected (check PIHOLE_PASSWORD)");
  }
  // Renew a little before the server-reported validity window to avoid races.
  const validityMs = (session.validity ?? 1800) * 1000;
  cachedSid = { sid: session.sid, expiresAt: now() + validityMs - 30_000 };
  return session.sid;
}

async function getSid(cfg: Config): Promise<string> {
  if (cachedSid && cachedSid.expiresAt > now()) return cachedSid.sid;
  return authenticate(cfg);
}

export async function fetchPihole(cfg: Config): Promise<PiholeReadout> {
  const runSummary = async (sid: string) =>
    fetchJson<SummaryResponse>(`${cfg.pihole.url}/api/stats/summary`, {
      timeoutMs: cfg.sourceTimeoutMs,
      headers: { "X-FTL-SID": sid },
    });

  let sid = await getSid(cfg);
  let summary: SummaryResponse;
  try {
    summary = await runSummary(sid);
  } catch (err) {
    // Stale/invalid session -> drop it and retry once with a fresh login.
    if (err instanceof HttpError && err.status === 401) {
      cachedSid = null;
      sid = await getSid(cfg);
      summary = await runSummary(sid);
    } else {
      throw err;
    }
  }

  return {
    adsBlocked: Math.max(0, Math.round(summary.queries?.blocked ?? 0)),
    queries: Math.max(0, Math.round(summary.queries?.total ?? 0)),
  };
}
