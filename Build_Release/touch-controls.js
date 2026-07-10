/**
 * On-screen touch controls (joystick + action buttons) for OpenClaw WASM.
 *
 * Renders an HTML/CSS overlay on top of the canvas and drives the game by
 * dispatching KeyboardEvents on the window — the same input path the engine's
 * OnKey handler already consumes (DOM `code` -> SDL scancode). No C++ changes.
 *
 * Visible only on touch devices, and only during active gameplay (not menus,
 * not the quick menu). Polls the exported game state, reusing gamepad-bridge's
 * getGameState() when present.
 */

(function () {
  "use strict";

  // Control key mapping — DOM KeyboardEvent.code values the engine maps to SDL.
  var KEY = {
    left: "ArrowLeft",
    right: "ArrowRight",
    up: "ArrowUp",
    down: "ArrowDown",
    jump: "Space",
    attack: "ControlLeft",
    fire: "AltLeft",
    weapon: "ShiftLeft",
    pause: "Escape",
    select: "Enter",
    back: "Escape",
  };
  var KEYCODE = {
    ArrowLeft: 37, ArrowUp: 38, ArrowRight: 39, ArrowDown: 40,
    Space: 32, ControlLeft: 17, AltLeft: 18, ShiftLeft: 16, Escape: 27, Enter: 13,
  };

  // GAME_STATE mirrors gamepad-bridge / GetJSGameState. 2 = in-game running.
  var STATE_INGAME = 2;
  var STATE_MENU = 1;

  // Reuse gamepad-bridge's HAPTIC_PRESETS via the exposed window.triggerHaptic
  // so touch buttons always match the gamepad timings/strength (single source
  // of truth — no duplicated numbers to drift). Preset names map to actions:
  // light=nav/weapon/back, medium=select, attack=attack/fire, jump=jump.
  var HAPTIC = { light: "light", medium: "medium", attack: "attack", jump: "jump" };
  function vibrate(preset) {
    if (typeof window.triggerHaptic === "function") {
      window.triggerHaptic(0, preset);
    } else if ("vibrate" in navigator) {
      try { navigator.vibrate(80); } catch (e) {}
    }
  }

  var heldKeys = {}; // code -> true while pressed

  function keyEvent(type, code) {
    return new KeyboardEvent(type, {
      key: code,
      code: code,
      keyCode: KEYCODE[code] || 0,
      which: KEYCODE[code] || 0,
      bubbles: true,
      cancelable: true,
    });
  }
  function pressKey(code) {
    if (heldKeys[code]) return;
    heldKeys[code] = true;
    window.dispatchEvent(keyEvent("keydown", code));
  }
  function releaseKey(code) {
    if (!heldKeys[code]) return;
    heldKeys[code] = false;
    window.dispatchEvent(keyEvent("keyup", code));
  }
  function releaseAll() {
    Object.keys(heldKeys).forEach(function (code) {
      if (heldKeys[code]) releaseKey(code);
    });
  }

  function getGameState() {
    if (typeof window.getGameState === "function") return window.getGameState();
    if (typeof Module !== "undefined" && typeof Module._GetJSGameState === "function") {
      try { return Module._GetJSGameState(); } catch (e) { return 0; }
    }
    return 0;
  }

  function isTouchDevice() {
    return (navigator.maxTouchPoints && navigator.maxTouchPoints > 0) ||
      "ontouchstart" in window;
  }

  // Show the overlay only when the last input was touch/pen (mirrors the
  // cursor-hiding logic). window.__lastPointerWasTouch is set by pointer-bridge
  // on canvas pointer events; the overlay's own touches keep it true (see
  // markTouchInput). A mouse move flips it false and hides the overlay again.
  function lastInputWasTouch() {
    return window.__lastPointerWasTouch === true;
  }
  function markTouchInput(e) {
    if (!e || e.pointerType !== "mouse") window.__lastPointerWasTouch = true;
  }

  // ---- DOM / CSS ------------------------------------------------------------

  function injectStyles() {
    var css = [
      "#touchControls{position:fixed;inset:0;z-index:9000;pointer-events:none;",
      "  display:none;touch-action:none;user-select:none;-webkit-user-select:none;",
      "  -webkit-touch-callout:none;}",
      "#touchControls.visible{display:block;}",
      "#touchControls *{-webkit-touch-callout:none;-webkit-user-select:none;user-select:none;",
      "  image-rendering:pixelated;}",
      // Match the score display: hard-stop GOLD BANDS (no smooth blend), black
      // outline, blocky feel — but translucent so it doesn't obscure gameplay.
      // Band palette sampled from the game's BODY_COLORS, brightest at top.
      // BAND = repeated as a reusable 5-step hard-stop gradient (fill, translucent):
      //   pale (255,255,240) / gold (253,228,107) / (223,207,94) / (198,183,80) / amber (147,136,59)
      // FILL alpha kept low (~0.4) for transparency; borders are solid black.
      // Joystick base (bottom-left) — same score-system bands, kept subtler
      // (lower alpha) as it covers more play area.
      "#tcJoyBase{position:absolute;left:24px;bottom:24px;width:140px;height:140px;",
      "  border-radius:50%;border:2px solid #000;box-shadow:0 0 0 1px rgba(0,0,0,0.5);",
      "  background:linear-gradient(to bottom,",
      "    rgba(252,239,82,0.4) 0 10%,rgba(248,232,110,0.4) 10% 20%,",
      "    rgba(253,253,183,0.4) 20% 30%,rgba(251,244,214,0.4) 30% 40%,",
      "    rgba(172,120,53,0.4) 40% 50%,rgba(208,170,62,0.4) 50% 60%,",
      "    rgba(242,204,77,0.4) 60% 70%,rgba(244,219,99,0.4) 70% 80%,",
      "    rgba(249,237,146,0.4) 80% 90%,rgba(239,216,112,0.4) 90% 100%);",
      "  pointer-events:auto;touch-action:none;}",
      "#tcJoyThumb{position:absolute;left:50%;top:50%;width:60px;height:60px;",
      "  margin:-30px 0 0 -30px;border-radius:50%;border:2px solid #000;",
      "  background:linear-gradient(to bottom,",
      "    rgba(252,239,82,0.4) 0 10%,rgba(248,232,110,0.4) 10% 20%,",
      "    rgba(253,253,183,0.4) 20% 30%,rgba(251,244,214,0.4) 30% 40%,",
      "    rgba(172,120,53,0.4) 40% 50%,rgba(208,170,62,0.4) 50% 60%,",
      "    rgba(242,204,77,0.4) 60% 70%,rgba(244,219,99,0.4) 70% 80%,",
      "    rgba(249,237,146,0.4) 80% 90%,rgba(239,216,112,0.4) 90% 100%);}",
      // Action buttons (bottom-right cluster)
      "#tcButtons{position:absolute;right:24px;bottom:24px;width:180px;height:180px;pointer-events:none;}",
      // 10-band hard-stop gradient sampled from the score display (top->bottom),
      // slightly translucent so gameplay still reads through. Black border.
      ".tcBtn{position:absolute;width:60px;height:60px;border-radius:50%;border:2px solid #000;",
      "  background:linear-gradient(to bottom,",
      "    rgba(252,239,82,0.4) 0 10%,rgba(248,232,110,0.4) 10% 20%,",
      "    rgba(253,253,183,0.4) 20% 30%,rgba(251,244,214,0.4) 30% 40%,",
      "    rgba(172,120,53,0.4) 40% 50%,rgba(208,170,62,0.4) 50% 60%,",
      "    rgba(242,204,77,0.4) 60% 70%,rgba(244,219,99,0.4) 70% 80%,",
      "    rgba(249,237,146,0.4) 80% 90%,rgba(239,216,112,0.4) 90% 100%);",
      "  color:#fff;font:bold 13px monospace;letter-spacing:0.5px;",
      "  text-shadow:-1px -1px 0 #000,1px -1px 0 #000,-1px 1px 0 #000,1px 1px 0 #000;",
      "  display:flex;align-items:center;justify-content:center;pointer-events:auto;",
      "  touch-action:none;text-align:center;}",
      // Pressed: same score-system bands, fully opaque (lights up solid).
      ".tcBtn.active{background:linear-gradient(to bottom,",
      "    rgb(252,239,82) 0 10%,rgb(248,232,110) 10% 20%,",
      "    rgb(253,253,183) 20% 30%,rgb(251,244,214) 30% 40%,",
      "    rgb(172,120,53) 40% 50%,rgb(208,170,62) 50% 60%,",
      "    rgb(242,204,77) 60% 70%,rgb(244,219,99) 70% 80%,",
      "    rgb(249,237,146) 80% 90%,rgb(239,216,112) 90% 100%);}",
      // Diamond layout: 4 buttons at N/E/S/W of the cluster, each 54px from
      // center so every adjacent gap equals the JUMP-ATK spacing (~76px).
      "#tcFire{right:6px;bottom:60px;}",     // E (outer right)
      "#tcJump{right:60px;bottom:6px;}",     // S (bottom)
      "#tcWeapon{right:60px;bottom:114px;}", // N (top)
      "#tcAttack{right:114px;bottom:60px;}", // W (inner left)
      // Pause (top-center)
      "#tcPause{position:absolute;left:50%;top:12px;transform:translateX(-50%);",
      "  width:42px;height:30px;border-radius:4px;border:2px solid #000;",
      "  background:linear-gradient(to bottom,",
      "    rgba(252,239,82,0.4) 0 10%,rgba(248,232,110,0.4) 10% 20%,",
      "    rgba(253,253,183,0.4) 20% 30%,rgba(251,244,214,0.4) 30% 40%,",
      "    rgba(172,120,53,0.4) 40% 50%,rgba(208,170,62,0.4) 50% 60%,",
      "    rgba(242,204,77,0.4) 60% 70%,rgba(244,219,99,0.4) 70% 80%,",
      "    rgba(249,237,146,0.4) 80% 90%,rgba(239,216,112,0.4) 90% 100%);",
      "  color:#fff;font:bold 13px monospace;",
      "  text-shadow:-1px -1px 0 #000,1px -1px 0 #000,-1px 1px 0 #000,1px 1px 0 #000;",
      "  display:flex;align-items:center;justify-content:center;pointer-events:auto;touch-action:none;}",
      "#tcPause.active{background:linear-gradient(to bottom,",
      "    rgb(252,239,82) 0 10%,rgb(248,232,110) 10% 20%,",
      "    rgb(253,253,183) 20% 30%,rgb(251,244,214) 30% 40%,",
      "    rgb(172,120,53) 40% 50%,rgb(208,170,62) 50% 60%,",
      "    rgb(242,204,77) 60% 70%,rgb(244,219,99) 70% 80%,",
      "    rgb(249,237,146) 80% 90%,rgb(239,216,112) 90% 100%);}",
      // Menu buttons (Select / Back) — reuse the JUMP / ATK slots so switching
      // modes doesn't shift button positions.
      "#tcBack{right:6px;bottom:60px;}",     // E (same slot as ATK)
      "#tcSelect{right:60px;bottom:6px;}",   // S (same slot as JUMP)
      // Mode-based visibility: gameplay shows action buttons + pause; menu shows
      // Select/Back only. Movement control (joystick/d-pad) shows in both.
      "#touchControls.mode-gameplay #tcSelect,#touchControls.mode-gameplay #tcBack{display:none;}",
      "#touchControls.mode-menu #tcJump,#touchControls.mode-menu #tcAttack,",
      "#touchControls.mode-menu #tcFire,#touchControls.mode-menu #tcWeapon,",
      "#touchControls.mode-menu #tcPause{display:none;}",
      // D-pad — plus layout, 4 directions only (no diagonals), same footprint
      // as the joystick base. Each key is a 46px rounded button.
      "#tcDpad{position:absolute;left:24px;bottom:24px;width:140px;height:140px;",
      "  pointer-events:none;}",
      ".tcDbtn{position:absolute;width:46px;height:46px;border-radius:8px;border:2px solid #000;",
      "  background:linear-gradient(to bottom,",
      "    rgba(252,239,82,0.4) 0 10%,rgba(248,232,110,0.4) 10% 20%,",
      "    rgba(253,253,183,0.4) 20% 30%,rgba(251,244,214,0.4) 30% 40%,",
      "    rgba(172,120,53,0.4) 40% 50%,rgba(208,170,62,0.4) 50% 60%,",
      "    rgba(242,204,77,0.4) 60% 70%,rgba(244,219,99,0.4) 70% 80%,",
      "    rgba(249,237,146,0.4) 80% 90%,rgba(239,216,112,0.4) 90% 100%);",
      "  color:#fff;font:bold 18px monospace;",
      "  text-shadow:-1px -1px 0 #000,1px -1px 0 #000,-1px 1px 0 #000,1px 1px 0 #000;",
      "  display:flex;align-items:center;justify-content:center;pointer-events:auto;",
      "  touch-action:none;}",
      ".tcDbtn.active{background:linear-gradient(to bottom,",
      "    rgb(252,239,82) 0 10%,rgb(248,232,110) 10% 20%,",
      "    rgb(253,253,183) 20% 30%,rgb(251,244,214) 30% 40%,",
      "    rgb(172,120,53) 40% 50%,rgb(208,170,62) 50% 60%,",
      "    rgb(242,204,77) 60% 70%,rgb(244,219,99) 70% 80%,",
      "    rgb(249,237,146) 80% 90%,rgb(239,216,112) 90% 100%);}",
      "#tcDup{left:47px;top:0;}",
      "#tcDdown{left:47px;top:94px;}",
      "#tcDleft{left:0;top:47px;}",
      "#tcDright{left:94px;top:47px;}",
      // Movement-mode toggle — small button above the movement control.
      "#tcMoveToggle{position:absolute;left:24px;bottom:172px;width:48px;height:48px;",
      "  border-radius:50%;border:2px solid #000;",
      "  background:linear-gradient(to bottom,",
      "    rgba(252,239,82,0.25) 0 10%,rgba(248,232,110,0.25) 10% 20%,",
      "    rgba(253,253,183,0.25) 20% 30%,rgba(251,244,214,0.25) 30% 40%,",
      "    rgba(172,120,53,0.25) 40% 50%,rgba(208,170,62,0.25) 50% 60%,",
      "    rgba(242,204,77,0.25) 60% 70%,rgba(244,219,99,0.25) 70% 80%,",
      "    rgba(249,237,146,0.25) 80% 90%,rgba(239,216,112,0.25) 90% 100%);",
      "  color:#fff;font:bold 10px monospace;",
      "  text-shadow:-1px -1px 0 #000,1px -1px 0 #000,-1px 1px 0 #000,1px 1px 0 #000;",
      "  display:flex;align-items:center;justify-content:center;pointer-events:auto;touch-action:none;}",
      // Show joystick OR d-pad based on the movement mode class on the root.
      "#touchControls.move-joystick #tcDpad{display:none;}",
      "#touchControls.move-dpad #tcJoyBase{display:none;}",
    ].join("");
    var style = document.createElement("style");
    style.id = "touchControlsStyles";
    style.textContent = css;
    document.head.appendChild(style);
  }

  function buildDom() {
    var root = document.createElement("div");
    root.id = "touchControls";
    root.innerHTML =
      '<div id="tcJoyBase"><div id="tcJoyThumb"></div></div>' +
      '<div id="tcDpad">' +
      '  <div class="tcDbtn" id="tcDup">▲</div>' +
      '  <div class="tcDbtn" id="tcDdown">▼</div>' +
      '  <div class="tcDbtn" id="tcDleft">◀</div>' +
      '  <div class="tcDbtn" id="tcDright">▶</div>' +
      "</div>" +
      '<div id="tcMoveToggle">STICK</div>' +
      '<div id="tcButtons">' +
      '  <div class="tcBtn" id="tcJump">JUMP</div>' +
      '  <div class="tcBtn" id="tcAttack">ATK</div>' +
      '  <div class="tcBtn" id="tcFire">FIRE</div>' +
      '  <div class="tcBtn" id="tcWeapon">WEAP</div>' +
      '  <div class="tcBtn" id="tcSelect">OK</div>' +
      '  <div class="tcBtn" id="tcBack">BACK</div>' +
      "</div>" +
      '<div id="tcPause">II</div>';
    document.body.appendChild(root);

    // iOS starts a long-press gesture (callout/selection/magnifier) ~0.5s into a
    // held touch, which causes a visible one-second hitch — most noticeable when
    // holding JUMP. Pointer-event preventDefault doesn't reliably suppress it, so
    // swallow the underlying touchstart/touchmove (non-passive) and contextmenu.
    var swallow = function (e) { if (e.cancelable) e.preventDefault(); };
    root.addEventListener("touchstart", swallow, { passive: false });
    root.addEventListener("touchmove", swallow, { passive: false });
    root.addEventListener("contextmenu", swallow);

    return root;
  }

  // ---- Joystick -------------------------------------------------------------

  function setupJoystick(base, thumb) {
    var activeId = null;
    var cx = 0, cy = 0, radius = 0;
    // Direction thresholds: normalized magnitude and 8-way mapping.
    var DEADZONE = 0.28;

    function updateDirection(nx, ny) {
      // nx,ny in -1..1 (screen coords: +y is down)
      var mag = Math.sqrt(nx * nx + ny * ny);
      var wantLeft = false, wantRight = false, wantUp = false, wantDown = false;
      if (mag >= DEADZONE) {
        var ang = Math.atan2(ny, nx); // -pi..pi, 0 = right, +pi/2 = down
        var deg = (ang * 180) / Math.PI;
        // 8-direction: horizontal if within +/-67.5 of left/right axis, etc.
        if (deg > -112.5 && deg < -67.5) { wantUp = true; }
        else if (deg > 67.5 && deg < 112.5) { wantDown = true; }
        else if (deg >= -67.5 && deg <= 67.5) {
          wantRight = true;
          if (deg < -22.5) wantUp = true;
          else if (deg > 22.5) wantDown = true;
        } else {
          wantLeft = true;
          if (deg > 112.5) wantDown = true;
          else if (deg < -112.5) wantUp = true;
        }
      }
      wantLeft ? pressKey(KEY.left) : releaseKey(KEY.left);
      wantRight ? pressKey(KEY.right) : releaseKey(KEY.right);
      wantUp ? pressKey(KEY.up) : releaseKey(KEY.up);
      wantDown ? pressKey(KEY.down) : releaseKey(KEY.down);
    }

    function moveThumb(clientX, clientY) {
      var dx = clientX - cx;
      var dy = clientY - cy;
      var dist = Math.sqrt(dx * dx + dy * dy);
      var clamped = Math.min(dist, radius);
      var ux = dist > 0 ? dx / dist : 0;
      var uy = dist > 0 ? dy / dist : 0;
      thumb.style.left = 50 + (ux * clamped / radius) * 50 + "%";
      thumb.style.top = 50 + (uy * clamped / radius) * 50 + "%";
      updateDirection(dx / radius, dy / radius);
    }
    function resetThumb() {
      thumb.style.left = "50%";
      thumb.style.top = "50%";
      releaseKey(KEY.left); releaseKey(KEY.right);
      releaseKey(KEY.up); releaseKey(KEY.down);
    }

    base.addEventListener("pointerdown", function (e) {
      if (activeId !== null) return;
      markTouchInput(e);
      activeId = e.pointerId;
      var r = base.getBoundingClientRect();
      cx = r.left + r.width / 2;
      cy = r.top + r.height / 2;
      radius = r.width / 2;
      try { base.setPointerCapture(e.pointerId); } catch (err) {}
      moveThumb(e.clientX, e.clientY);
      e.preventDefault();
    });
    base.addEventListener("pointermove", function (e) {
      if (e.pointerId !== activeId) return;
      moveThumb(e.clientX, e.clientY);
      e.preventDefault();
    });
    function end(e) {
      if (e.pointerId !== activeId) return;
      activeId = null;
      try { base.releasePointerCapture(e.pointerId); } catch (err) {}
      resetThumb();
      e.preventDefault();
    }
    base.addEventListener("pointerup", end);
    base.addEventListener("pointercancel", end);
  }

  // ---- Buttons --------------------------------------------------------------

  // Hold-style button: key held while pressed (jump, fire).
  function setupHoldButton(el, code, hapticMs) {
    el.addEventListener("pointerdown", function (e) {
      markTouchInput(e);
      el.classList.add("active");
      pressKey(code);
      vibrate(hapticMs || HAPTIC.light);
      try { el.setPointerCapture(e.pointerId); } catch (err) {}
      e.preventDefault();
    });
    function up(e) {
      el.classList.remove("active");
      releaseKey(code);
      e.preventDefault();
    }
    el.addEventListener("pointerup", up);
    el.addEventListener("pointercancel", up);
  }

  // Tap-style button: quick down+up (attack, weapon, pause, select, back).
  function setupTapButton(el, code, hapticMs) {
    el.addEventListener("pointerdown", function (e) {
      markTouchInput(e);
      el.classList.add("active");
      pressKey(code);
      vibrate(hapticMs || HAPTIC.light);
      e.preventDefault();
    });
    function up(e) {
      el.classList.remove("active");
      releaseKey(code);
      e.preventDefault();
    }
    el.addEventListener("pointerup", up);
    el.addEventListener("pointercancel", up);
  }

  // ---- Visibility -----------------------------------------------------------

  function isMenuVisible() {
    if (typeof Module !== "undefined" && typeof Module._IsMenuVisibleJS === "function") {
      try { return Module._IsMenuVisibleJS() === 1; } catch (e) { return false; }
    }
    return false;
  }

  // Determine which control set to show:
  //   "gameplay" — playing (INGAME and no menu visible): joystick + actions
  //   "menu"     — main menu OR in-game quick menu: joystick + Select/Back
  //   ""         — hidden
  function computeMode() {
    if (!isTouchDevice() || !lastInputWasTouch()) return "";
    var state = getGameState();
    if (state === STATE_MENU || isMenuVisible()) return "menu";
    if (state === STATE_INGAME) return "gameplay";
    return "";
  }

  function setupVisibility(root) {
    var mode = null;
    function tick() {
      var next = computeMode();
      if (next !== mode) {
        mode = next;
        releaseAll(); // never carry held keys across a mode change
        root.classList.toggle("visible", next !== "");
        root.classList.toggle("mode-gameplay", next === "gameplay");
        root.classList.toggle("mode-menu", next === "menu");
      }
      setTimeout(tick, 200);
    }
    tick();
  }

  // ---- Movement mode toggle (joystick <-> d-pad) ----------------------------

  var MOVE_STORAGE_KEY = "openclaw.touchMoveMode";
  function loadMoveMode() {
    try {
      var v = localStorage.getItem(MOVE_STORAGE_KEY);
      if (v === "joystick" || v === "dpad") return v;
    } catch (e) {}
    return "dpad"; // default
  }
  function saveMoveMode(mode) {
    try { localStorage.setItem(MOVE_STORAGE_KEY, mode); } catch (e) {}
  }

  function setupMoveToggle(root, toggleEl) {
    function apply(mode) {
      root.classList.toggle("move-dpad", mode === "dpad");
      root.classList.toggle("move-joystick", mode === "joystick");
      // Label shows the mode you'll switch TO (the opposite of current).
      toggleEl.textContent = mode === "dpad" ? "STICK" : "D-PAD";
    }
    var mode = loadMoveMode();
    apply(mode);

    toggleEl.addEventListener("pointerdown", function (e) {
      markTouchInput(e);
      releaseAll(); // drop any held direction before switching
      mode = mode === "dpad" ? "joystick" : "dpad";
      saveMoveMode(mode);
      apply(mode);
      vibrate(HAPTIC.light);
      e.preventDefault();
    });
  }

  // ---- Init -----------------------------------------------------------------

  function init() {
    if (!isTouchDevice()) {
      // Desktop: still allow via responsive/touch emulation. maxTouchPoints
      // becomes >0 in device mode, so re-check happens in setupVisibility.
    }
    injectStyles();
    var root = buildDom();
    setupJoystick(document.getElementById("tcJoyBase"), document.getElementById("tcJoyThumb"));
    // D-pad direction buttons (hold-style, 4 directions only, no diagonals)
    setupHoldButton(document.getElementById("tcDup"), KEY.up, HAPTIC.light);
    setupHoldButton(document.getElementById("tcDdown"), KEY.down, HAPTIC.light);
    setupHoldButton(document.getElementById("tcDleft"), KEY.left, HAPTIC.light);
    setupHoldButton(document.getElementById("tcDright"), KEY.right, HAPTIC.light);
    setupMoveToggle(root, document.getElementById("tcMoveToggle"));
    // Gameplay buttons
    setupHoldButton(document.getElementById("tcJump"), KEY.jump, HAPTIC.jump);
    setupHoldButton(document.getElementById("tcFire"), KEY.fire, HAPTIC.attack);
    setupTapButton(document.getElementById("tcAttack"), KEY.attack, HAPTIC.attack);
    setupTapButton(document.getElementById("tcWeapon"), KEY.weapon, HAPTIC.light);
    setupTapButton(document.getElementById("tcPause"), KEY.pause, HAPTIC.light);
    // Menu buttons
    setupTapButton(document.getElementById("tcSelect"), KEY.select, HAPTIC.medium);
    setupTapButton(document.getElementById("tcBack"), KEY.back, HAPTIC.light);
    setupVisibility(root);
    console.log("[TouchControls] overlay initialized");
  }

  if (document.readyState === "loading") {
    document.addEventListener("DOMContentLoaded", init);
  } else {
    init();
  }
})();
