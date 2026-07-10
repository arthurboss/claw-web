/**
 * Gamepad Bridge for OpenClaw WASM
 *
 * Behavior changes based on game state:
 * - Menu/Paused: Simulate keyboard for navigation (single press)
 * - In-game: Send to C++ for continuous input handling
 *
 * Button mapping (Xbox controller):
 * Menu: A=Enter, B=Escape, D-pad/Stick=Arrows, Start=Enter
 * Game: A=Jump, B=Fire, X=Attack, Y=ChangeAmmo, Start=Pause, D-pad/Stick=Move
 */

const gamepadStates = [{}, {}, {}, {}];
let pollingActive = false;
let debugMode = false;
let hapticEnabled = true;

// Haptic feedback presets (duration in ms, weakMagnitude/strongMagnitude 0-1).
// dual-rumble maps strongMagnitude -> the big low-frequency motor (the "thud"
// you actually feel) and weakMagnitude -> the small high-frequency motors
// (subtle). Every preset needs a real strong component, and rumble motors have
// spin-up latency so pulses shorter than ~80ms barely register — durations are
// floored accordingly.
// Haptic feedback presets (duration in ms, weakMagnitude/strongMagnitude 0-1).
// dual-rumble maps strongMagnitude -> the big low-frequency motor (the "thud"
// you actually feel) and weakMagnitude -> the small high-frequency motors.
const HAPTIC_PRESETS = {
    light:      { duration: 110, weak: 0.6, strong: 0.6 },
    medium:     { duration: 130, weak: 0.6, strong: 0.75 },
    heavy:      { duration: 180, weak: 0.9, strong: 0.95 },
    damage:     { duration: 240, weak: 0.7, strong: 1.0 },
    death:      { duration: 1000, weak: 1.0, strong: 1.0 },
    explosion:  { duration: 340, weak: 1.0, strong: 1.0 },
    pickup:     { duration: 120, weak: 0.6, strong: 0.65 },
    attack:     { duration: 130, weak: 0.6, strong: 0.85 },
    jump:       { duration: 120, weak: 0.6, strong: 0.7 },
    // Footsteps — same feel as a regular (low) landing.
    step:       { duration: 110, weak: 0.6, strong: 0.6 },
};

// Game states from C++
const GAME_STATE = { UNKNOWN: 0, MENU: 1, INGAME: 2, PAUSED: 3, CUTSCENE: 4 };

// Menu navigation keys
const MENU_BUTTON_TO_KEY = {
    0: 'Enter',      // A -> Select
    1: 'Escape',     // B -> Back
    9: 'Enter',      // Start -> Select
    12: 'ArrowUp',
    13: 'ArrowDown',
    14: 'ArrowLeft',
    15: 'ArrowRight'
};

const KEY_CODES = {
    'Enter': 13, 'Escape': 27,
    'ArrowUp': 38, 'ArrowDown': 40, 'ArrowLeft': 37, 'ArrowRight': 39
};

function log(msg) {
    if (debugMode) console.log('[Gamepad] ' + msg);
}

// Haptic feedback functions
function triggerHaptic(gamepadIndex, preset) {
    if (!hapticEnabled) return false;

    const config = HAPTIC_PRESETS[preset];
    if (!config) {
        log('Unknown haptic preset: ' + preset);
        return false;
    }

    return triggerHapticCustom(gamepadIndex, config.duration, config.weak, config.strong);
}

// Vibrate the touchscreen device via the Web Vibration API. This is the touch
// equivalent of gamepad rumble; it only fires when the last input was touch so
// a phone buzzes but a mouse/gamepad user is unaffected. The Vibration API has
// no magnitude, so the preset duration carries the intensity (matching the
// gamepad preset table).
function triggerDeviceVibration(durationMs) {
    if (!window.__lastPointerWasTouch) return false;
    if (!('vibrate' in navigator)) return false;
    try {
        navigator.vibrate(Math.max(1, Math.round(durationMs)));
        return true;
    } catch (e) {
        log('Vibration error: ' + e);
        return false;
    }
}

// Resolve the gamepad to rumble. The requested index may be empty — in-game
// haptics come from C++ with a hardcoded index 0, but the browser can place
// the controller at any slot (e.g. a phantom null sits at 0 and the real Xbox
// controller is at 1). Fall back to the first connected gamepad in that case.
function resolveGamepad(gamepadIndex) {
    const gamepads = navigator.getGamepads();
    let gp = gamepads[gamepadIndex];
    if (gp && gp.connected) return gp;
    for (let i = 0; i < gamepads.length; i++) {
        if (gamepads[i] && gamepads[i].connected) return gamepads[i];
    }
    return null;
}

function triggerHapticCustom(gamepadIndex, durationMs, weakMagnitude, strongMagnitude) {
    if (!hapticEnabled) return false;

    // Touch device vibration, independent of any connected gamepad.
    const vibrated = triggerDeviceVibration(durationMs);

    const gp = resolveGamepad(gamepadIndex);
    if (!gp) return vibrated;

    // Try vibrationActuator (standard Gamepad API)
    if (gp.vibrationActuator) {
        gp.vibrationActuator.playEffect('dual-rumble', {
            startDelay: 0,
            duration: durationMs,
            weakMagnitude: Math.min(1, Math.max(0, weakMagnitude)),
            strongMagnitude: Math.min(1, Math.max(0, strongMagnitude))
        }).catch(e => log('Vibration error: ' + e));
        return true;
    }

    // Try hapticActuators (older API, some browsers)
    if (gp.hapticActuators && gp.hapticActuators.length > 0) {
        const intensity = Math.max(weakMagnitude, strongMagnitude);
        gp.hapticActuators[0].pulse(intensity, durationMs).catch(e => log('Haptic error: ' + e));
        return true;
    }

    return vibrated;
}

function setHapticEnabled(enabled) {
    hapticEnabled = enabled;
    log('Haptic feedback ' + (enabled ? 'enabled' : 'disabled'));
}

// Expose haptic functions for C++ calls
window.triggerHaptic = triggerHaptic;
window.triggerHapticCustom = triggerHapticCustom;
window.setHapticEnabled = setHapticEnabled;

// Check if C++ WASM functions are available (they use underscore prefix)
function isCppReady() {
    return typeof Module !== 'undefined' &&
           typeof Module._OnJSGamepadButtonDown === 'function';
}

function getGameState() {
    if (typeof Module === 'undefined' || typeof Module._GetJSGameState !== 'function') {
        return GAME_STATE.UNKNOWN;
    }
    try {
        return Module._GetJSGameState();
    } catch (e) {
        return GAME_STATE.UNKNOWN;
    }
}

function isMenuState() {
    const state = getGameState();
    // Main menu or unknown (before game loads) = use keyboard simulation
    // Paused state (quick menu) sends to C++ for proper handling
    return state === GAME_STATE.MENU || state === GAME_STATE.UNKNOWN;
}

function simulateKeyPress(key) {
    const target = document.getElementById('canvas') || document;
    target.dispatchEvent(new KeyboardEvent('keydown', {
        key, code: key, keyCode: KEY_CODES[key] || 0, which: KEY_CODES[key] || 0,
        bubbles: true, cancelable: true
    }));
    setTimeout(() => {
        target.dispatchEvent(new KeyboardEvent('keyup', {
            key, code: key, keyCode: KEY_CODES[key] || 0, which: KEY_CODES[key] || 0,
            bubbles: true, cancelable: true
        }));
    }, 50);
}

function pollGamepads() {
    if (!pollingActive) return;

    const gamepads = navigator.getGamepads();
    const cppReady = isCppReady();
    const inMenu = isMenuState();

    for (let i = 0; i < 4; i++) {
        const gp = gamepads[i];
        const prev = gamepadStates[i];

        const isConnected = gp && gp.connected;
        const wasConnected = prev.connected || false;

        if (isConnected && !wasConnected) {
            log('Controller ' + i + ' connected: ' + gp.id);
            if (cppReady) {
                try { Module._OnJSGamepadConnected(i); } catch (e) { /* ignore */ }
            }
            prev.connected = true;
            prev.buttons = new Array(18).fill(false);
            prev.axes = [0, 0, 0, 0];
        } else if (!isConnected && wasConnected) {
            log('Controller ' + i + ' disconnected');
            if (cppReady) {
                try { Module._OnJSGamepadDisconnected(i); } catch (e) { /* ignore */ }
            }
            gamepadStates[i] = {};
            continue;
        }

        if (!isConnected) continue;

        // Process buttons
        for (let b = 0; b < Math.min(gp.buttons.length, 18); b++) {
            const pressed = gp.buttons[b].pressed;
            const wasPressed = prev.buttons[b] || false;

            if (pressed !== wasPressed) {
                if (pressed) log('Button ' + b + ' pressed (menu=' + inMenu + ', cpp=' + cppReady + ')');

                // Video skip on A or Start
                if (pressed && (b === 0 || b === 9)) {
                    skipVideoIfPlaying();
                }

                if (inMenu) {
                    // Menu mode: simulate keyboard on press only
                    if (pressed && MENU_BUTTON_TO_KEY[b]) {
                        simulateKeyPress(MENU_BUTTON_TO_KEY[b]);
                        // A, B, Start = select/back = medium haptic; D-pad = navigation = light haptic
                        const isSelectOrBack = (b === 0 || b === 1 || b === 9);
                        triggerHaptic(i, isSelectOrBack ? 'medium' : 'light');
                    }
                } else if (cppReady) {
                    // In-game mode: send to C++
                    try {
                        if (pressed) {
                            Module._OnJSGamepadButtonDown(i, b, gp.buttons[b].value);
                        } else {
                            Module._OnJSGamepadButtonUp(i, b);
                        }
                    } catch (e) { log('C++ call error: ' + e); }
                }

                prev.buttons[b] = pressed;
            }
        }

        // Process axes
        for (let a = 0; a < Math.min(gp.axes.length, 4); a++) {
            let value = gp.axes[a];
            if (Math.abs(value) < 0.15) value = 0;

            const prevValue = prev.axes[a] || 0;

            if (inMenu) {
                // Menu: single key press when crossing threshold
                const threshold = 0.5;
                const wasOver = Math.abs(prevValue) >= threshold;
                const nowOver = Math.abs(value) >= threshold;

                if (nowOver && !wasOver) {
                    if (a === 0) simulateKeyPress(value < 0 ? 'ArrowLeft' : 'ArrowRight');
                    else if (a === 1) simulateKeyPress(value < 0 ? 'ArrowUp' : 'ArrowDown');
                    triggerHaptic(i, 'light');
                }
            } else if (cppReady) {
                // In-game: send continuous axis values to C++
                if (Math.abs(value - prevValue) > 0.01) {
                    try { Module._OnJSGamepadAxis(i, a, value); } catch (e) { /* ignore */ }
                }
            }

            prev.axes[a] = value;
        }
    }

    requestAnimationFrame(pollGamepads);
}

function skipVideoIfPlaying() {
    if (typeof Module !== 'undefined' && Module.genericVideoPlayers) {
        for (let id in Module.genericVideoPlayers) {
            const player = Module.genericVideoPlayers[id];
            if (player?.video && !player.video.paused && !player.video.ended) {
                log('Skipping video');
                player.video.pause();
                player.video.currentTime = player.video.duration;
                player.ended = true;
                if (player.video.parentNode) player.video.remove();
                return;
            }
        }
    }
}

function startGamepadPolling() {
    if (pollingActive) return;
    pollingActive = true;
    log('Polling started');
    requestAnimationFrame(pollGamepads);
}

function stopGamepadPolling() {
    pollingActive = false;
    log('Polling stopped');
}

// Debug
window.startGamepadPolling = startGamepadPolling;
window.stopGamepadPolling = stopGamepadPolling;
window.gamepadDebug = function() {
    console.log('=== Gamepad Debug ===');
    console.log('Polling active:', pollingActive);
    console.log('Haptic enabled:', hapticEnabled);
    console.log('Module exists:', typeof Module !== 'undefined');
    if (typeof Module !== 'undefined') {
        console.log('Module._OnJSGamepadButtonDown:', typeof Module._OnJSGamepadButtonDown);
        console.log('Module._OnJSGamepadButtonUp:', typeof Module._OnJSGamepadButtonUp);
        console.log('Module._GetJSGameState:', typeof Module._GetJSGameState);
        console.log('Module.calledRun:', Module.calledRun);
    }
    console.log('C++ ready:', isCppReady());
    console.log('Game state:', getGameState(), '(0=unknown, 1=menu, 2=ingame, 3=paused)');
    console.log('Is menu mode:', isMenuState());
    const gamepads = navigator.getGamepads();
    for (let i = 0; i < 4; i++) {
        if (gamepads[i]) {
            const gp = gamepads[i];
            console.log('Gamepad ' + i + ':', gp.id, '- connected:', gp.connected);
            console.log('  Vibration support:', gp.vibrationActuator ? 'yes (vibrationActuator)' :
                (gp.hapticActuators?.length ? 'yes (hapticActuators)' : 'no'));
        }
    }
};

// Test haptic feedback manually
window.testHaptic = function(preset = 'medium') {
    console.log('Testing haptic preset:', preset);
    return triggerHaptic(0, preset);
};

// Auto-start
window.addEventListener('gamepadconnected', e => {
    log('Gamepad connected: ' + e.gamepad.id);
    startGamepadPolling();
});
document.addEventListener('click', () => startGamepadPolling(), { once: true });
document.addEventListener('keydown', () => startGamepadPolling(), { once: true });

// Also try to start when Module is ready (for cases where gamepad is already connected)
function waitForModule() {
    if (isCppReady()) {
        log('Module ready, C++ functions available');
        startGamepadPolling();
    } else if (typeof Module !== 'undefined' && Module.calledRun) {
        log('Module loaded but C++ functions not found - check EXPORTED_FUNCTIONS');
    } else {
        setTimeout(waitForModule, 500);
    }
}
setTimeout(waitForModule, 1000);

log('Initialized');
