import type { FastifyReply, FastifyRequest, preHandlerHookHandler } from "fastify";
import type { Config } from "./config.js";

/**
 * Optional bearer-token guard. When `API_TOKEN` is unset the guard is a no-op
 * (open on a trusted LAN); when set, requests must carry
 * `Authorization: Bearer <token>`.
 */
export function makeAuthHook(cfg: Config): preHandlerHookHandler {
  const expected = cfg.apiToken;

  return function authHook(req: FastifyRequest, reply: FastifyReply, done) {
    if (!expected) return done(); // auth disabled

    const header = req.headers.authorization;
    const token = header?.startsWith("Bearer ") ? header.slice(7).trim() : undefined;

    if (token !== expected) {
      reply.code(401).send({ error: "unauthorized" });
      return; // do not call done() — request is finished
    }
    done();
  };
}
