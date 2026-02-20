/**
 * Bridge between IndexedDB and Emscripten's virtual file system
 * Handles lazy loading of CLAW.REZ from browser storage
 */

import { AssetStorage } from './asset-storage.js';

let assetStorage = null;
let uploadResolve = null;

/**
 * Show asset upload UI
 */
function showAssetUpload() {
  const uploadDiv = document.getElementById('assetUpload');
  if (uploadDiv) {
    uploadDiv.style.display = 'flex';
  }
}

/**
 * Hide asset upload UI
 */
function hideAssetUpload() {
  const uploadDiv = document.getElementById('assetUpload');
  if (uploadDiv) {
    uploadDiv.style.display = 'none';
  }
}

/**
 * Validate CLAW.REZ file selection and enable/disable upload button
 */
function validateClawRezFile() {
  const fileInput = document.getElementById('clawRezFile');
  const uploadBtn = document.getElementById('uploadBtn');
  const file = fileInput.files[0];

  if (!file) {
    uploadBtn.disabled = true;
    return false;
  }

  // Validate file name (case-insensitive) - must be exactly CLAW.REZ
  if (!file.name.match(/^CLAW\.REZ$/i)) {
    alert('Error: File must be named CLAW.REZ (case-insensitive)\n\nYou selected: ' + file.name);
    fileInput.value = ''; // Clear the selection
    uploadBtn.disabled = true;
    return false;
  }

  // Validate file size (approximately 113MB)
  const expectedSize = 113 * 1024 * 1024; // 113MB
  const tolerance = 10 * 1024 * 1024; // 10MB tolerance
  const sizeMB = (file.size / 1024 / 1024).toFixed(2);

  if (Math.abs(file.size - expectedSize) > tolerance) {
    const proceed = window.confirm(
      `Warning: File size is ${sizeMB}MB. Expected ~113MB.\n\n` +
      `This might not be the correct CLAW.REZ file.\n\n` +
      `Continue anyway?`
    );
    if (!proceed) {
      fileInput.value = ''; // Clear the selection
      uploadBtn.disabled = true;
      return false;
    }
  }

  // File is valid, enable upload button
  uploadBtn.disabled = false;
  return true;
}

/**
 * Handle CLAW.REZ file upload
 */
async function uploadClawRez() {
  const fileInput = document.getElementById('clawRezFile');
  const file = fileInput.files[0];

  if (!file) {
    alert('Please select a file first');
    return;
  }

  // Revalidate before upload (in case button was enabled programmatically)
  if (!file.name.match(/^CLAW\.REZ$/i)) {
    alert('Error: File must be named CLAW.REZ');
    return;
  }

  // Show progress UI
  document.getElementById('uploadArea').style.display = 'none';
  const progressDiv = document.getElementById('uploadProgress');
  progressDiv.style.display = 'block';

  try {
    // Store in IndexedDB with progress tracking
    await assetStorage.storeFile('CLAW.REZ', file, (loaded, total) => {
      const percent = (loaded / total) * 100;
      document.getElementById('uploadProgressBar').value = percent;
      document.getElementById('uploadStatus').textContent =
        `Storing assets: ${percent.toFixed(1)}%`;
    });

    // Update status
    document.getElementById('uploadStatus').textContent = 'Upload complete! Starting game...';

    // Hide upload UI
    setTimeout(() => {
      hideAssetUpload();
      if (uploadResolve) {
        uploadResolve();
        uploadResolve = null;
      }
    }, 1000);

  } catch (error) {
    console.error('Upload failed:', error);
    alert(`Upload failed: ${error.message}`);

    // Reset UI
    progressDiv.style.display = 'none';
    document.getElementById('uploadArea').style.display = 'block';
  }
}

/**
 * Allow user to re-upload CLAW.REZ (for troubleshooting)
 */
async function reuploadClawRez() {
  if (!assetStorage) return;

  const confirm = window.confirm(
    'This will delete your current CLAW.REZ and require re-upload.\n\n' +
    'Continue?'
  );

  if (confirm) {
    try {
      await assetStorage.deleteFile('CLAW.REZ');
      window.location.reload();
    } catch (error) {
      console.error('Failed to delete CLAW.REZ:', error);
      alert(`Failed to delete file: ${error.message}`);
    }
  }
}

/**
 * Wait for user to complete upload
 */
function waitForUpload() {
  return new Promise((resolve) => {
    uploadResolve = resolve;
  });
}

// Global to store CLAW.REZ data until Emscripten FS is ready
let clawRezData = null;

/**
 * Prepare CLAW.REZ from IndexedDB (but don't write to FS yet)
 */
async function prepareAssetStorage() {
  try {
    // Initialize IndexedDB storage
    assetStorage = new AssetStorage();
    await assetStorage.init();

    // Check if CLAW.REZ exists in IndexedDB
    const hasClawRez = await assetStorage.hasFile('CLAW.REZ');

    if (!hasClawRez) {
      console.log('CLAW.REZ not found in storage. Showing upload UI...');
      showAssetUpload();
      await waitForUpload();
    } else {
      console.log('CLAW.REZ found in storage. Loading...');

      // Get file metadata
      const metadata = await assetStorage.getFileMetadata('CLAW.REZ');
      console.log(`CLAW.REZ size: ${(metadata.size / 1024 / 1024).toFixed(2)}MB`);
    }

    // Retrieve CLAW.REZ from IndexedDB
    const clawRezBlob = await assetStorage.getFile('CLAW.REZ');
    if (!clawRezBlob) {
      throw new Error('Failed to retrieve CLAW.REZ from storage');
    }

    // Convert Blob to ArrayBuffer and store for later
    console.log('Loading CLAW.REZ into memory...');
    const arrayBuffer = await clawRezBlob.arrayBuffer();
    clawRezData = new Uint8Array(arrayBuffer);
    console.log('CLAW.REZ ready to mount');

    return true;

  } catch (error) {
    console.error('Failed to prepare asset storage:', error);
    alert(
      `Failed to load game assets: ${error.message}\n\n` +
      `Please check browser console for details.`
    );
    return false;
  }
}

/**
 * Mount CLAW.REZ to Emscripten FS (called from Module.preRun)
 */
function mountClawRezToFS() {
  if (!clawRezData) {
    console.error('CLAW.REZ data not prepared!');
    return false;
  }

  try {
    console.log('Writing CLAW.REZ to virtual file system...');
    FS.writeFile('/CLAW.REZ', clawRezData);
    console.log('CLAW.REZ mounted successfully');

    // Clear the data from memory
    clawRezData = null;

    return true;
  } catch (error) {
    console.error('Failed to mount CLAW.REZ to FS:', error);
    return false;
  }
}

/**
 * Get storage statistics (for debugging)
 */
async function getStorageStats() {
  if (!assetStorage) return null;

  try {
    const files = await assetStorage.listFiles();
    const totalSize = await assetStorage.getStorageSize();

    return {
      files: files,
      totalSize: totalSize,
      totalSizeMB: (totalSize / 1024 / 1024).toFixed(2)
    };
  } catch (error) {
    console.error('Failed to get storage stats:', error);
    return null;
  }
}

// Export functions for ES modules
export {
  validateClawRezFile,
  uploadClawRez,
  reuploadClawRez,
  getStorageStats,
  prepareAssetStorage,
  mountClawRezToFS
};

// Keep window globals for HTML event handlers (permanent)
window.validateClawRezFile = validateClawRezFile;
window.uploadClawRez = uploadClawRez;
window.reuploadClawRez = reuploadClawRez;
window.getStorageStats = getStorageStats;
