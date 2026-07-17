// PWA shell cache — includes all app bootstrap assets
// Vite will serve these with consistent, deterministic paths
const CACHE_NAME = "openclaw-v1";

// Essential assets needed to boot the game
// These are served by Vite and have stable, hashed filenames in production
const SHELL_ASSETS = [
  "./openclaw.html",
  "./openclaw.js",
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

// Fetch strategy: cache-first for shell, network-first for game data
self.addEventListener("fetch", (event) => {
  const url = new URL(event.request.url);
  const pathname = url.pathname;

  // Never cache large WASM/binary assets — they have their own IndexedDB caching
  const BYPASS_CACHE = [".wasm", ".data", "ASSETS.ZIP"];
  if (BYPASS_CACHE.some((ext) => pathname.endsWith(ext))) {
    event.respondWith(fetch(event.request));
    return;
  }

  // Cache-first: shell assets (JS, HTML, icons, manifest)
  // Return from cache if available, fall back to network
  event.respondWith(
    caches.match(event.request).then((cached) => {
      if (cached) return cached;

      return fetch(event.request).then((response) => {
        // Only cache successful responses with valid content-type
        if (
          !response ||
          response.status !== 200 ||
          response.type === "opaque"
        ) {
          return response;
        }

        // Store a clone in cache for future use
        const clone = response.clone();
        caches.open(CACHE_NAME).then((cache) => {
          cache.put(event.request, clone);
        });

        return response;
      });
    })
  );
});
