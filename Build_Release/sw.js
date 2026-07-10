const CACHE_NAME = "openclaw-v1";

// Shell assets to cache on install (everything needed to boot)
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
];

self.addEventListener("install", (event) => {
  event.waitUntil(
    caches
      .open(CACHE_NAME)
      .then((cache) => cache.addAll(SHELL_ASSETS))
      .then(() => self.skipWaiting())
  );
});

self.addEventListener("activate", (event) => {
  event.waitUntil(
    caches
      .keys()
      .then((keys) =>
        Promise.all(
          keys.filter((k) => k !== CACHE_NAME).map((k) => caches.delete(k))
        )
      )
      .then(() => self.clients.claim())
  );
});

self.addEventListener("fetch", (event) => {
  const url = new URL(event.request.url);

  // Never cache WASM, game data or ASSETS.ZIP — they are large and loaded by the
  // asset-loader with its own IndexedDB caching strategy.
  const bypass = [".wasm", ".data", "ASSETS.ZIP"].some((ext) =>
    url.pathname.endsWith(ext)
  );
  if (bypass) {
    event.respondWith(fetch(event.request));
    return;
  }

  // Cache-first for everything else (shell + small assets)
  event.respondWith(
    caches.match(event.request).then((cached) => {
      if (cached) return cached;
      return fetch(event.request).then((response) => {
        if (!response || response.status !== 200 || response.type === "opaque")
          return response;
        const clone = response.clone();
        caches.open(CACHE_NAME).then((cache) => cache.put(event.request, clone));
        return response;
      });
    })
  );
});
