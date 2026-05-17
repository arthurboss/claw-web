/**
 * Keyboard Capture for OpenClaw WASM
 *
 * Prevents browser/OS shortcuts from interfering with game controls.
 * - Blocks Ctrl/Cmd + common shortcuts when game is focused
 * - Blocks Alt + keys (macOS special characters)
 * - Captures game keys and prevents default browser behavior
 *
 * Note: Ctrl + Arrow keys cannot be blocked (macOS Spaces navigation
 * is handled at the system level). Disable in System Settings if needed.
 */

(function() {
  'use strict';

  var canvas = document.getElementById("canvas");
  if (!canvas) return;

  // Keys that should always be captured by the game when canvas is focused
  var gameKeys = [
    'Control', 'Alt', 'Shift', 'Meta',
    'ArrowUp', 'ArrowDown', 'ArrowLeft', 'ArrowRight',
    'Space', 'Enter', 'Escape', 'Tab',
    'KeyA', 'KeyS', 'KeyD', 'KeyW', 'KeyZ', 'KeyX', 'KeyC',
    'Digit1', 'Digit2', 'Digit3', 'Digit4', 'Digit5',
    'F1', 'F2', 'F3', 'F4', 'F5', 'F6', 'F7', 'F8', 'F9', 'F10', 'F11', 'F12'
  ];

  // Shortcuts that should be blocked when the game is active
  function shouldBlockShortcut(e) {
    // Block Ctrl/Cmd + common shortcuts that conflict with game
    if (e.ctrlKey || e.metaKey) {
      var blockedWithModifier = ['KeyC', 'KeyV', 'KeyX', 'KeyZ', 'KeyA', 'KeyS', 'KeyW', 'KeyQ', 'KeyR', 'KeyT'];
      if (blockedWithModifier.indexOf(e.code) !== -1) return true;
    }
    // Block Alt + keys on macOS (special characters)
    if (e.altKey && !e.ctrlKey && !e.metaKey) {
      return true;
    }
    return false;
  }

  function handleKeyEvent(e) {
    // Only intercept when canvas has focus or game container is active
    var isGameFocused = document.activeElement === canvas ||
                        document.activeElement === document.body ||
                        canvas.contains(document.activeElement);

    if (!isGameFocused) return;

    // Always let F11 through for browser fullscreen toggle
    if (e.code === 'F11') return;

    // Block system shortcuts that conflict with game controls
    if (shouldBlockShortcut(e)) {
      e.preventDefault();
      e.stopPropagation();
      return;
    }

    // For game keys, prevent default browser behavior
    if (gameKeys.indexOf(e.code) !== -1 || gameKeys.indexOf(e.key) !== -1) {
      e.preventDefault();
    }
  }

  // Use capture phase to intercept before other handlers
  document.addEventListener('keydown', handleKeyEvent, true);
  document.addEventListener('keyup', handleKeyEvent, true);

  // Ensure canvas can receive focus
  canvas.tabIndex = 0;

  // Auto-focus canvas when clicked
  canvas.addEventListener('click', function() {
    canvas.focus();
  });

  // Focus canvas when game container is clicked
  var gameContainer = document.getElementById('gameContainer');
  if (gameContainer) {
    gameContainer.addEventListener('click', function(e) {
      if (e.target !== canvas) {
        canvas.focus();
      }
    });
  }

  console.log('[Keyboard] Game key interceptor enabled');
})();
