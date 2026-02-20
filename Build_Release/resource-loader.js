/**
 * JavaScript bridge for resource loading progress
 * Connects C++ ResourceCache to browser UI
 */

let currentResourceLoadingPhase = 'Initializing';
let currentResourceName = '';
let resourceLoadProgress = 0;

/**
 * Update loading UI with current progress
 */
function updateLoadingUI(phase, resourceName, loaded, total) {
  currentResourceLoadingPhase = phase;
  currentResourceName = resourceName;

  if (total > 0) {
    resourceLoadProgress = (loaded / total) * 100;
  }

  // Update DOM if loading indicator exists
  const loadingIndicator = document.getElementById('loadingIndicator');
  if (loadingIndicator) {
    loadingIndicator.style.display = 'block';
  }

  const loadingText = document.getElementById('loadingText');
  if (loadingText) {
    if (resourceName) {
      loadingText.textContent = `${phase}: ${resourceName}`;
    } else {
      loadingText.textContent = phase;
    }
  }

  const loadingBar = document.getElementById('loadingBar');
  if (loadingBar) {
    loadingBar.value = resourceLoadProgress;
  }

  const loadingPercent = document.getElementById('loadingPercent');
  if (loadingPercent) {
    loadingPercent.textContent = `${Math.floor(resourceLoadProgress)}%`;
  }

  console.log(`[Resource Loading] ${phase}: ${resourceName} (${Math.floor(resourceLoadProgress)}%)`);
}

/**
 * Hide loading UI when complete
 */
function hideLoadingUI() {
  const loadingIndicator = document.getElementById('loadingIndicator');
  if (loadingIndicator) {
    loadingIndicator.style.display = 'none';
  }

  console.log('[Resource Loading] Complete');
}

/**
 * Show loading UI
 */
function showLoadingUI() {
  const loadingIndicator = document.getElementById('loadingIndicator');
  if (loadingIndicator) {
    loadingIndicator.style.display = 'block';
  }
}

/**
 * Called from C++ when resource loading starts
 */
Module.onResourceLoadStart = function(phase) {
  updateLoadingUI(phase, '', 0, 100);
};

/**
 * Called from C++ during resource loading
 */
Module.onResourceLoadProgress = function(resourceName, loaded, total) {
  updateLoadingUI(currentResourceLoadingPhase, resourceName, loaded, total);
};

/**
 * Called from C++ when resource loading completes
 */
Module.onResourceLoadComplete = function() {
  hideLoadingUI();
};

/**
 * Get current loading state (for debugging)
 */
function getLoadingState() {
  return {
    phase: currentResourceLoadingPhase,
    resource: currentResourceName,
    progress: resourceLoadProgress
  };
}

// Expose for debugging
window.getLoadingState = getLoadingState;
window.updateLoadingUI = updateLoadingUI;

console.log('[Resource Loader] Bridge initialized');
