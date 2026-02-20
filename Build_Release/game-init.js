/**
 * Main ES Module Coordinator for OpenClaw
 * Imports all modules and exposes necessary functions to window for HTML compatibility
 */

import { AssetStorage } from './asset-storage.js';
import {
  validateClawRezFile,
  uploadClawRez,
  reuploadClawRez,
  getStorageStats,
  prepareAssetStorage,
  mountClawRezToFS
} from './asset-loader.js';
import { initResourceLoader, getLoadingState, updateLoadingUI } from './resource-loader.js';
import { WebGLBridge } from './graphics-bridge.js';
import { TextureBridge } from './texture-bridge.js';

// Expose functions needed by HTML event handlers
window.validateClawRezFile = validateClawRezFile;
window.uploadClawRez = uploadClawRez;
window.reuploadClawRez = reuploadClawRez;
window.getStorageStats = getStorageStats;

// Expose functions needed by inline scripts
window.prepareAssetStorage = prepareAssetStorage;
window.mountClawRezToFS = mountClawRezToFS;
window.getLoadingState = getLoadingState;
window.updateLoadingUI = updateLoadingUI;

// Initialize resource loader when Module is ready
// This is called from inline script after Module is defined
window.initResourceLoader = function(Module) {
  initResourceLoader(Module);
};

// Initialize game (called from inline script)
window.initGameWhenReady = async function() {
  console.log('[Game Init] Starting game initialization...');

  const success = await prepareAssetStorage();

  if (success) {
    console.log('[Game Init] Assets ready, loading game...');
    // loadGame() is defined in inline script and handles openclaw.js injection
    if (typeof window.loadGame === 'function') {
      window.loadGame();
    } else {
      console.error('[Game Init] loadGame() function not found');
    }
  } else {
    console.error('[Game Init] Failed to prepare assets');
  }
};

console.log('[Game Init] Module loaded and ready');
