/**
 * Tiny fetch helpers with a hard timeout so a single slow/hung source can never
 * stall the aggregated response. Uses the global `fetch` (Node >= 20).
 */

export class HttpError extends Error {
  constructor(
    message: string,
    readonly status?: number,
  ) {
    super(message);
    this.name = "HttpError";
  }
}

export interface RequestOpts {
  timeoutMs: number;
  headers?: Record<string, string>;
  method?: "GET" | "POST";
  body?: string;
}

async function request(url: string, opts: RequestOpts): Promise<Response> {
  const controller = new AbortController();
  const timer = setTimeout(() => controller.abort(), opts.timeoutMs);
  try {
    const res = await fetch(url, {
      method: opts.method ?? "GET",
      headers: opts.headers,
      body: opts.body,
      signal: controller.signal,
    });
    if (!res.ok) {
      throw new HttpError(`${opts.method ?? "GET"} ${url} -> ${res.status}`, res.status);
    }
    return res;
  } catch (err) {
    if (err instanceof HttpError) throw err;
    const reason = err instanceof Error ? err.message : String(err);
    throw new HttpError(`${opts.method ?? "GET"} ${url} failed: ${reason}`);
  } finally {
    clearTimeout(timer);
  }
}

export async function fetchJson<T = unknown>(url: string, opts: RequestOpts): Promise<T> {
  const res = await request(url, opts);
  return (await res.json()) as T;
}
