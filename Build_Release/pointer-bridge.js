/**
 * Pointer Events Bridge for OpenClaw WASM
 *
 * Single modern input path for mouse, touch and pen. Listens to the browser's
 * Pointer Events API on the canvas and forwards each event to C++ via the
 * exported OnJSPointer* functions (same pattern as gamepad-bridge.js).
 *
 * Coordinates are converted from CSS pixels to the canvas backing-store
 * (device-pixel) space that the engine treats as its window space, so the
 * C++ side never has to deal with DPI or CSS scaling.
 */

(function () {
  "use strict";

  var debugMode = true;
  function log(msg) {
    if (debugMode) console.log("[Pointer] " + msg);
  }

  console.log(
    "%c[Pointer] pointer-bridge.js loaded — branch: feat/native-pointer-input (native Pointer Events, no SDL input)",
    "color:#fff;background:#7c3aed;padding:2px 6px;border-radius:3px;font-weight:bold"
  );

  // "mouse" | "touch" | "pen" -> 0 | 1 | 2
  function pointerTypeCode(e) {
    if (e.pointerType === "touch") return 1;
    if (e.pointerType === "pen") return 2;
    return 0;
  }

  // Expose whether the most recent pointer was touch/pen (vs mouse) so the
  // haptics layer can vibrate the device instead of a gamepad. Mirrors the
  // C++-side WasLastInputTouch() used for the cursor.
  function recordPointerType(e) {
    window.__lastPointerWasTouch = e.pointerType !== "mouse";
  }

  // Convert a PointerEvent to canvas backing-store coordinates.
  // The canvas element box is sized to exactly match the rendered bitmap
  // (see fitCanvasToContainer), so a simple rect-relative scale is correct.
  function toCanvasSpace(canvas, e) {
    var rect = canvas.getBoundingClientRect();
    var scaleX = rect.width > 0 ? canvas.width / rect.width : 1;
    var scaleY = rect.height > 0 ? canvas.height / rect.height : 1;
    return {
      x: Math.round((e.clientX - rect.left) * scaleX),
      y: Math.round((e.clientY - rect.top) * scaleY),
    };
  }

  function cppReady() {
    return (
      typeof Module !== "undefined" &&
      typeof Module._OnJSPointerDown === "function"
    );
  }

  // Report touch capability to C++ once the module is ready. Uses
  // navigator.maxTouchPoints (the canonical touch-capability check).
  var touchCapabilityReported = false;
  function reportTouchCapability() {
    if (touchCapabilityReported) return;
    if (typeof Module === "undefined" ||
        typeof Module._OnJSTouchCapability !== "function") {
      setTimeout(reportTouchCapability, 200);
      return;
    }
    var isTouch =
      (navigator.maxTouchPoints && navigator.maxTouchPoints > 0) ||
      "ontouchstart" in window
        ? 1
        : 0;
    Module._OnJSTouchCapability(isTouch);
    touchCapabilityReported = true;
    log("touch capability reported: " + isTouch);
  }

  function attach() {
    var canvas = document.getElementById("canvas");
    if (!canvas) {
      setTimeout(attach, 200);
      return;
    }

    reportTouchCapability();

    canvas.addEventListener("pointerdown", function (e) {
      if (!cppReady()) return;
      recordPointerType(e);
      var p = toCanvasSpace(canvas, e);
      // Capture so we keep receiving move/up even if the pointer leaves the canvas.
      try { canvas.setPointerCapture(e.pointerId); } catch (err) {}
      Module._OnJSPointerDown(e.pointerId, p.x, p.y, pointerTypeCode(e), e.button);
      log("down id=" + e.pointerId + " " + p.x + "," + p.y + " type=" + e.pointerType);
      e.preventDefault();
    });

    canvas.addEventListener("pointermove", function (e) {
      if (!cppReady()) return;
      recordPointerType(e);
      var p = toCanvasSpace(canvas, e);
      Module._OnJSPointerMove(e.pointerId, p.x, p.y, pointerTypeCode(e));
      e.preventDefault();
    });

    function onUp(e) {
      if (!cppReady()) return;
      recordPointerType(e);
      var p = toCanvasSpace(canvas, e);
      try { canvas.releasePointerCapture(e.pointerId); } catch (err) {}
      Module._OnJSPointerUp(e.pointerId, p.x, p.y, pointerTypeCode(e), e.button);
      log("up id=" + e.pointerId + " " + p.x + "," + p.y);
      e.preventDefault();
    }
    canvas.addEventListener("pointerup", onUp);
    canvas.addEventListener("pointercancel", onUp);

    log("attached to canvas");
  }

  if (document.readyState === "loading") {
    document.addEventListener("DOMContentLoaded", attach);
  } else {
    attach();
  }
})();
