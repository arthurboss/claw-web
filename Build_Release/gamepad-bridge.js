/**
 * Gamepad Bridge for OpenClaw WASM
 *
 * Polls the browser's native Gamepad API and sends events to C++ via ccall.
 * Also handles video skipping via gamepad buttons (Start/A).
 */

// State tracking
const gamepadStates = [{}, {}, {}, {}];
let pollingActive = false;
let debugMode = true; // Enable debug logging

function log(msg) {
    if (debugMode) {
        console.log('[Gamepad] ' + msg);
    }
}

/**
 * Main polling function
 */
function pollGamepads() {
    if (!pollingActive) return;

    const gamepads = navigator.getGamepads();

    for (let i = 0; i < 4; i++) {
        const gp = gamepads[i];
        const prev = gamepadStates[i];

        const isConnected = gp && gp.connected;
        const wasConnected = prev.connected || false;

        // Handle connect/disconnect
        if (isConnected && !wasConnected) {
            log('Controller ' + i + ' connected: ' + gp.id);
            log('  Buttons: ' + gp.buttons.length + ', Axes: ' + gp.axes.length);

            if (typeof Module !== 'undefined' && Module.ccall) {
                try {
                    Module.ccall('OnJSGamepadConnected', 'void', ['number'], [i]);
                    log('  -> C++ notified');
                } catch (e) {
                    log('  -> C++ call failed: ' + e);
                }
            } else {
                log('  -> Module.ccall not available yet');
            }

            prev.connected = true;
            prev.buttons = new Array(17).fill(false);
            prev.axes = new Array(4).fill(0);
        } else if (!isConnected && wasConnected) {
            log('Controller ' + i + ' disconnected');

            if (typeof Module !== 'undefined' && Module.ccall) {
                try {
                    Module.ccall('OnJSGamepadDisconnected', 'void', ['number'], [i]);
                } catch (e) {
                    log('  -> C++ call failed: ' + e);
                }
            }

            gamepadStates[i] = {};
            continue;
        }

        if (!isConnected) continue;

        // Process buttons
        const numButtons = Math.min(gp.buttons.length, 17);
        for (let b = 0; b < numButtons; b++) {
            const pressed = gp.buttons[b].pressed;
            const value = gp.buttons[b].value;
            const wasPressed = prev.buttons[b] || false;

            if (pressed !== wasPressed) {
                // Log button changes for debugging
                if (pressed) {
                    log('Button ' + b + ' pressed (value: ' + value.toFixed(2) + ')');
                } else {
                    log('Button ' + b + ' released');
                }

                // Handle video skip: Start (9) or A (0) button
                if (pressed && (b === 0 || b === 9)) {
                    skipVideoIfPlaying();
                }

                // Send to C++
                if (typeof Module !== 'undefined' && Module.ccall) {
                    try {
                        if (pressed) {
                            Module.ccall('OnJSGamepadButtonDown', 'void',
                                ['number', 'number', 'number'], [i, b, value]);
                        } else {
                            Module.ccall('OnJSGamepadButtonUp', 'void',
                                ['number', 'number'], [i, b]);
                        }
                    } catch (e) {
                        log('C++ call failed: ' + e);
                    }
                }

                prev.buttons[b] = pressed;
            }
        }

        // Process axes (with deadzone)
        const numAxes = Math.min(gp.axes.length, 4);
        for (let a = 0; a < numAxes; a++) {
            let value = gp.axes[a];
            // Apply deadzone
            if (Math.abs(value) < 0.15) value = 0;

            const prevValue = prev.axes[a] || 0;

            if (Math.abs(value - prevValue) > 0.01) {
                if (typeof Module !== 'undefined' && Module.ccall) {
                    try {
                        Module.ccall('OnJSGamepadAxis', 'void',
                            ['number', 'number', 'number'], [i, a, value]);
                    } catch (e) {
                        // Don't spam logs for axis events
                    }
                }
                prev.axes[a] = value;
            }
        }
    }

    requestAnimationFrame(pollGamepads);
}

/**
 * Skip video if one is currently playing (for cutscenes)
 */
function skipVideoIfPlaying() {
    // Check for video player from WasmVideoPlayer
    if (typeof Module !== 'undefined' && Module.genericVideoPlayers) {
        for (let id in Module.genericVideoPlayers) {
            const player = Module.genericVideoPlayers[id];
            if (player && player.video && !player.video.paused && !player.video.ended) {
                log('Skipping video via gamepad');
                // Trigger the same stop as keyboard would
                if (typeof js_StopVideo === 'function') {
                    js_StopVideo(parseInt(id));
                } else {
                    // Fallback: directly manipulate the video
                    player.video.pause();
                    player.video.currentTime = player.video.duration;
                    if (player.video.parentNode) {
                        player.video.parentNode.removeChild(player.video);
                    }
                }
                return;
            }
        }
    }

    // Also check for any video element in the game container
    const gameContainer = document.getElementById('gameContainer');
    if (gameContainer) {
        const videos = gameContainer.getElementsByTagName('video');
        for (let v of videos) {
            if (!v.paused && !v.ended) {
                log('Skipping video element via gamepad');
                v.pause();
                v.currentTime = v.duration;
                return;
            }
        }
    }
}

/**
 * Start polling
 */
function startGamepadPolling() {
    if (pollingActive) return;

    pollingActive = true;
    log('Starting polling...');

    // Check if any gamepads are already connected
    const gamepads = navigator.getGamepads();
    let found = false;
    for (let i = 0; i < 4; i++) {
        if (gamepads[i] && gamepads[i].connected) {
            log('Found already-connected gamepad ' + i + ': ' + gamepads[i].id);
            found = true;
        }
    }
    if (!found) {
        log('No gamepads currently connected. Press a button on your gamepad.');
    }

    requestAnimationFrame(pollGamepads);
}

/**
 * Stop polling
 */
function stopGamepadPolling() {
    pollingActive = false;
    log('Stopped polling');
}

// Expose functions globally
window.startGamepadPolling = startGamepadPolling;
window.stopGamepadPolling = stopGamepadPolling;
window.gamepadDebug = function() {
    const gamepads = navigator.getGamepads();
    console.log('=== Gamepad Debug ===');
    console.log('Polling active:', pollingActive);
    console.log('Module available:', typeof Module !== 'undefined');
    console.log('Module.ccall available:', typeof Module !== 'undefined' && typeof Module.ccall === 'function');
    for (let i = 0; i < 4; i++) {
        const gp = gamepads[i];
        if (gp) {
            console.log('Gamepad ' + i + ':', gp.id, 'connected:', gp.connected);
        }
    }
};

// Listen for gamepad connection events (doesn't require user gesture)
window.addEventListener('gamepadconnected', function(e) {
    log('Gamepad connected event: ' + e.gamepad.id + ' at index ' + e.gamepad.index);
    if (!pollingActive) {
        startGamepadPolling();
    }
});

window.addEventListener('gamepaddisconnected', function(e) {
    log('Gamepad disconnected event: ' + e.gamepad.id);
});

// Auto-start on user gesture (backup in case gamepadconnected doesn't fire)
function onUserGesture() {
    if (!pollingActive) {
        log('User gesture detected');
        startGamepadPolling();
    }
}

document.addEventListener('click', onUserGesture, { once: true });
document.addEventListener('keydown', onUserGesture, { once: true });

log('Initialized. Waiting for gamepad connection or user gesture.');
log('Tip: Call gamepadDebug() in console to check status.');
