# =============================================================================
#  companion-api — multi-stage build
#  Stage 1 compiles TypeScript; stage 2 is a slim runtime with prod deps only.
# =============================================================================

# --- build stage ---
FROM node:22-alpine AS build
WORKDIR /app
COPY package.json package-lock.json* ./
RUN npm install
COPY tsconfig.json ./
COPY src ./src
RUN npm run build

# --- runtime stage ---
FROM node:22-alpine AS runtime
ENV NODE_ENV=production
WORKDIR /app
COPY package.json package-lock.json* ./
RUN npm install --omit=dev && npm cache clean --force
COPY --from=build /app/dist ./dist

# Drop to the built-in non-root user shipped with the node image.
USER node

EXPOSE 8090
CMD ["node", "dist/index.js"]
