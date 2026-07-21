// Service Worker registration: installability + offline shell, with a
// self-heal safety net so a bad deploy can never permanently strand a client
// (important on mobile, where clearing a stuck SW by hand is painful).
(function () {
  if (!('serviceWorker' in navigator)) return;

  // --- Remote kill-switch -----------------------------------------------
  // sw-control.json carries an "unregister" id. When that id changes, every
  // client unregisters its SW + clears caches exactly once (we remember the
  // last-handled id), then reloads clean. Page-side on purpose: it works even
  // if the installed SW is broken. Fetched network-fresh (no-store) and
  // bypassed by the SW.
  function checkKillSwitch() {
    return fetch('./sw-control.json', { cache: 'no-store' })
      .then(function (r) { return r.ok ? r.json() : null; })
      .then(function (ctl) {
        if (!ctl) return false;
        var id = String(ctl.unregister || '');
        if (!id) return false;
        var seen = localStorage.getItem('sw_unregister_id');
        if (id === seen) return false; // already handled this signal
        localStorage.setItem('sw_unregister_id', id);
        console.warn('[SW] Kill-switch tripped (' + id + ') — clearing.');
        // Scope the teardown to THIS environment. On a shared origin,
        // getRegistrations()/caches.keys() see sibling deployments too; an
        // unscoped clear would unregister production's SW and delete its
        // caches when only staging's switch was flipped. We match our own
        // scope path (the page's directory) for registrations and the sw.js
        // cache-name convention ("openclaw::<scope>::") for caches.
        var scopeDir = location.pathname.replace(/[^/]*$/, '');
        var cachePrefix = 'openclaw::' + scopeDir + '::';
        return navigator.serviceWorker.getRegistrations()
          .then(function (regs) {
            return Promise.all(regs
              .filter(function (r) {
                try { return new URL(r.scope).pathname === scopeDir; }
                catch (e) { return false; }
              })
              .map(function (r) { return r.unregister(); }));
          })
          .then(function () {
            return (self.caches ? caches.keys() : Promise.resolve([]));
          })
          .then(function (keys) {
            return Promise.all(keys
              .filter(function (k) { return k.indexOf(cachePrefix) === 0; })
              .map(function (k) { return caches.delete(k); }));
          })
          .then(function () { location.reload(); return true; });
      })
      .catch(function () { return false; });
  }

  // --- Auto-reload when a new SW takes control --------------------------
  // Guarded so it fires at most once (no reload loop).
  var refreshing = false;
  navigator.serviceWorker.addEventListener('controllerchange', function () {
    if (refreshing) return;
    refreshing = true;
    location.reload();
  });

  window.addEventListener('load', function () {
    checkKillSwitch().then(function (tripped) {
      if (tripped) return; // reloading; skip registration this pass
      navigator.serviceWorker
        .register('./sw.js', { scope: './' })
        .then(function (reg) {
          // Periodic update check (once per day).
          setInterval(function () { reg.update(); }, 24 * 60 * 60 * 1000);
        })
        .catch(function (err) {
          console.warn('[SW] Registration failed:', err);
        });
    });
  });
})();
