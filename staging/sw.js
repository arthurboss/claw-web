// PWA shell cache — includes all app bootstrap assets
// Vite will serve these with consistent, deterministic paths
const CACHE_NAME = "openclaw-v3";

// Essential assets needed to boot the game
// These are served by Vite and have stable, hashed filenames in production
const SHELL_ASSETS = [
  "./openclaw.html",
  "./game-init.js",
  "./asset-storage.js",
  "./asset-loader.js",
  "./resource-loader.js",
  "./graphics-bridge.js",
  "./texture-bridge.js",
  "./save-storage.js",
  "./gamepad-bridge.js",
  "./keyboard-capture.js",
  "./site.webmanifest",
  "./android-chrome-192x192.png",
  "./android-chrome-512x512.png",
  "./apple-touch-icon.png",
  "./favicon.ico",
  "./favicon-16x16.png",
  "./favicon-32x32.png",
];

// On install: cache the app shell
self.addEventListener("install", (event) => {
  event.waitUntil(
    caches
      .open(CACHE_NAME)
      .then((cache) => {
        // Use addAll to fail fast if any asset is missing
        return cache.addAll(SHELL_ASSETS);
      })
      .then(() => {
        // Force waiting service worker to become active immediately
        return self.skipWaiting();
      })
  );
});

// On activate: remove old caches if CACHE_NAME changes
self.addEventListener("activate", (event) => {
  event.waitUntil(
    caches
      .keys()
      .then((keys) =>
        Promise.all(
          keys
            .filter((k) => k !== CACHE_NAME)
            .map((k) => {
              console.log("[SW] Deleting old cache:", k);
              return caches.delete(k);
            })
        )
      )
      .then(() => {
        // Claim all clients immediately (no need to reload page)
        return self.clients.claim();
      })
  );
});

// Fetch strategy: cache-first for the small shell, hands-off for everything
// else. Anything the SW does not explicitly handle is left to the browser by
// NOT calling event.respondWith — critical for large/streamed assets.
self.addEventListener("fetch", (event) => {
  const req = event.request;
  const url = new URL(req.url);

  // Only ever touch same-origin GET requests.
  if (req.method !== "GET" || url.origin !== self.location.origin) return;

  // Never intercept range requests (WASM/data/media stream with these; a SW
  // returning a 200 for a Range request breaks playback and wasm streaming).
  if (req.headers.has("range")) return;

  // Large binaries are loaded/streamed directly and have their own IndexedDB
  // cache — the SW must not touch them at all (letting a failed inner fetch
  // surface as a FetchEvent error would abort the game load).
  const HANDS_OFF = [".wasm", ".data", ".zip", ".mp4"];
  const path = url.pathname.toLowerCase();
  if (HANDS_OFF.some((ext) => path.endsWith(ext)) || path.includes("assets.zip")) {
    return; // browser handles it natively
  }

  // Cache-first for the shell; fall back to network and cache the result.
  // If the network fails, return any cached copy rather than throwing.
  event.respondWith(
    caches.match(req).then((cached) => {
      if (cached) return cached;
      return fetch(req)
        .then((response) => {
          if (response && response.status === 200 && response.type === "basic") {
            const clone = response.clone();
            caches.open(CACHE_NAME).then((cache) => cache.put(req, clone));
          }
          return response;
        })
        .catch(() => caches.match(req));
    })
  );
});
