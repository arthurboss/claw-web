/**
 * Bridge between IndexedDB and Emscripten's virtual file system
 * Handles lazy loading of CLAW.REZ from browser storage
 */

import { AssetStorage } from './asset-storage.js';

let assetStorage = null;
let uploadResolve = null;

/**
 * Show offline (no cache) message
 */
function showOfflineNeedCache() {
  const offlineDiv = document.getElementById('offlineNeedCache');
  if (offlineDiv) {
    offlineDiv.classList.add('visible');
  }
}

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
// CLAW.REZ is a Monolith resource archive. Known-valid releases carry one of
// two header signatures near the start (confirmed against original discs):
//   - "RezMgr Version 1"  (118,033,115 bytes)
//   - "WinRez 2.4"         (119,321,886 bytes)
// A commonly mis-uploaded file carries "WinRez LT 3.0" and is only ~168 bytes,
// so the size gate rejects it — but we also reject the "LT" signature outright.
const REZ_VALID_SIGNATURES = ['RezMgr Version 1', 'WinRez 2.4'];
const REZ_MIN_SIZE = 100 * 1024 * 1024; // ~100MB; real files are ~113MB
const REZ_MAX_SIZE = 130 * 1024 * 1024; // generous upper bound

function showUploadError(message) {
  const el = document.getElementById('uploadError');
  if (el) { el.textContent = message; el.style.display = 'block'; }
}

function clearUploadError() {
  const el = document.getElementById('uploadError');
  if (el) { el.textContent = ''; el.style.display = 'none'; }
}

function rejectRezFile(message) {
  const fileInput = document.getElementById('clawRezFile');
  const uploadBtn = document.getElementById('uploadBtn');
  const drop = document.getElementById('fileDrop');
  const name = document.getElementById('fileDropName');
  if (fileInput) fileInput.value = '';
  if (uploadBtn) uploadBtn.disabled = true;
  if (drop) drop.classList.remove('has-file');
  if (name) name.textContent = '';
  showUploadError(message);
}

function acceptRezFile(file) {
  clearUploadError();
  const uploadBtn = document.getElementById('uploadBtn');
  const drop = document.getElementById('fileDrop');
  const name = document.getElementById('fileDropName');
  if (uploadBtn) uploadBtn.disabled = false;
  if (drop) drop.classList.add('has-file');
  if (name) name.textContent = file.name + ' ✓';
}

// Read the header and confirm it carries one of the known-valid signatures.
async function hasRezSignature(file) {
  try {
    const head = await file.slice(0, 64).arrayBuffer();
    const text = new TextDecoder('latin1').decode(new Uint8Array(head));
    return REZ_VALID_SIGNATURES.some(function (sig) { return text.indexOf(sig) !== -1; });
  } catch (e) {
    console.error('Failed to read CLAW.REZ header:', e);
    return false;
  }
}

// Validate the selected file. Hard-rejects anything that is not a real
// CLAW.REZ (wrong name, empty, wildly wrong size, or missing the RezMgr
// signature) — no "continue anyway" escape hatch. Async: enables the upload
// button only after the header check passes.
async function validateClawRezFile() {
  const fileInput = document.getElementById('clawRezFile');
  const uploadBtn = document.getElementById('uploadBtn');
  const file = fileInput.files[0];

  if (uploadBtn) uploadBtn.disabled = true;
  if (!file) return false;

  if (!file.name.match(/^CLAW\.REZ$/i)) {
    rejectRezFile('That file is not CLAW.REZ.\n\nThe file must be named CLAW.REZ (from the original Captain Claw game).\n\nYou selected: ' + file.name);
    return false;
  }

  if (file.size === 0) {
    rejectRezFile('That file is empty.\n\nPlease select the real CLAW.REZ from the original Captain Claw (1997) game.');
    return false;
  }

  const sizeMB = (file.size / 1024 / 1024).toFixed(1);
  if (file.size < REZ_MIN_SIZE || file.size > REZ_MAX_SIZE) {
    rejectRezFile('That does not look like CLAW.REZ.\n\nIts size is ' + sizeMB + 'MB but CLAW.REZ is about 113MB. Please select the correct file from the original Captain Claw (1997) game.');
    return false;
  }

  if (!(await hasRezSignature(file))) {
    rejectRezFile('That file is not a valid CLAW.REZ.\n\nIt is missing the expected archive header. Please select the correct CLAW.REZ from the original Captain Claw (1997) game.');
    return false;
  }

  acceptRezFile(file);
  return true;
}

/**
 * Detect the best available compression algorithm
 * Priority: zstd (best ratio) → brotli → gzip (universal fallback)
 * Note: As of Chrome 145, only gzip/deflate are supported in CompressionStream API
 * Future browser versions may support zstd/brotli, so we keep them in priority order
 * @returns {Object|null} Algorithm info {name, label, mimeType} or null if none available
 */
function detectBestCompressionAlgorithm() {
  const algorithms = [
    { name: 'zstd', label: 'Zstd', mimeType: 'application/zstd' },
    { name: 'br', label: 'Brotli', mimeType: 'application/br' },
    { name: 'gzip', label: 'Gzip', mimeType: 'application/gzip' }
  ];

  for (const algo of algorithms) {
    try {
      // Test if browser supports this algorithm
      new CompressionStream(algo.name);
      new DecompressionStream(algo.name);
      console.log(`✅ Compression algorithm selected: ${algo.label}`);
      return algo;
    } catch (e) {
      console.log(`⚠️  ${algo.label} not supported, trying next...`);
    }
  }

  console.warn('⚠️  No compression algorithm available, storing uncompressed');
  return null;
}

/**
 * Compress a Blob using specified algorithm
 * @param {Blob} blob - Blob to compress
 * @param {string} algorithm - Algorithm name ('zstd', 'br', 'gzip')
 * @param {string} mimeType - MIME type for compressed blob
 * @param {Function} progressCallback - Progress callback (0.0 to 1.0)
 * @returns {Promise<Blob>} Compressed Blob
 */
async function compressBlob(blob, algorithm, mimeType, progressCallback) {
  const readableStream = blob.stream();
  const compressionStream = new CompressionStream(algorithm);
  const compressedStream = readableStream.pipeThrough(compressionStream);

  const chunks = [];
  const reader = compressedStream.getReader();
  let bytesRead = 0;
  const totalBytes = blob.size;

  while (true) {
    const { done, value } = await reader.read();
    if (done) break;

    chunks.push(value);
    bytesRead += value.length;

    if (progressCallback) {
      progressCallback(Math.min(bytesRead / totalBytes, 0.99));
    }
  }

  if (progressCallback) progressCallback(1.0);

  return new Blob(chunks, { type: mimeType });
}

/**
 * Handle CLAW.REZ file upload with compression
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

  // Upload button click is a user gesture — pre-warm AudioContext now.
  if (typeof window.prewarmAudioContext === 'function') {
    window.prewarmAudioContext();
  }

  // Request durable storage on first upload. On iOS PWA the system may show
  // a permission prompt on the very first write; calling persist() here
  // (inside a user gesture) ensures the grant happens before we start writing
  // so the write doesn't silently fail.
  if (navigator.storage && navigator.storage.persist) {
    navigator.storage.persist().catch(() => {});
  }

  clearUploadError();

  // Show progress UI
  document.getElementById('uploadArea').style.display = 'none';
  const progressDiv = document.getElementById('uploadProgress');
  progressDiv.style.display = 'block';

  try {
    console.log(`Original file size: ${(file.size / 1024 / 1024).toFixed(2)}MB`);

    // Detect best compression algorithm
    const algo = detectBestCompressionAlgorithm();

    let blobToStore = file;

    if (algo) {
      // Compress file with progress tracking
      blobToStore = await compressBlob(file, algo.name, algo.mimeType, (progress) => {
        document.getElementById('uploadProgressBar').value = progress * 50;
        document.getElementById('uploadStatus').textContent =
          `Compressing (${algo.label}): ${(progress * 100).toFixed(1)}%`;
      });

      console.log(`Compressed size: ${(blobToStore.size / 1024 / 1024).toFixed(2)}MB`);
      console.log(`Compression ratio: ${((1 - blobToStore.size / file.size) * 100).toFixed(1)}%`);
    } else {
      console.log('Storing uncompressed (no compression support)');
    }

    // Store compressed file in IndexedDB
    await assetStorage.storeFile('CLAW.REZ', blobToStore, (loaded, total) => {
      const percent = 50 + (loaded / total) * 50;
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

    // If compression failed, try fallback to uncompressed
    if (error.message && error.message.includes('compress')) {
      console.warn('Compression failed, falling back to uncompressed storage');
      try {
        await assetStorage.storeFile('CLAW.REZ', file, (loaded, total) => {
          const percent = (loaded / total) * 100;
          document.getElementById('uploadProgressBar').value = percent;
          document.getElementById('uploadStatus').textContent =
            `Storing assets (uncompressed): ${percent.toFixed(1)}%`;
        });

        document.getElementById('uploadStatus').textContent = 'Upload complete! Starting game...';
        setTimeout(() => {
          hideAssetUpload();
          if (uploadResolve) {
            uploadResolve();
            uploadResolve = null;
          }
        }, 1000);
        return;
      } catch (fallbackError) {
        console.error('Fallback upload also failed:', fallbackError);
      }
    }

    // IndexedDB is disabled or quota-limited in some browsers' private/
    // incognito modes, which is the most common cause of storage failures.
    var failMsg = `Upload failed: ${error.message}`;
    var errName = (error && error.name) || '';
    if (/Quota|Security|InvalidState|Unknown/i.test(errName) ||
        /quota|storage|indexeddb|database/i.test((error && error.message) || '')) {
      failMsg += '\n\nTip: private / incognito browsing often blocks local '
               + 'storage. Try again in a normal browser window.';
    }
    showUploadError(failMsg);

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
 * Decompress a Blob using specified algorithm
 * @param {Blob} compressedBlob - Compressed Blob
 * @param {string} algorithm - Algorithm name ('zstd', 'br', 'gzip')
 * @returns {Promise<Blob>} Decompressed Blob
 */
async function decompressBlob(compressedBlob, algorithm) {
  const readableStream = compressedBlob.stream();
  const decompressionStream = new DecompressionStream(algorithm);
  const decompressedStream = readableStream.pipeThrough(decompressionStream);

  const chunks = [];
  const reader = decompressedStream.getReader();

  while (true) {
    const { done, value } = await reader.read();
    if (done) break;
    chunks.push(value);
  }

  return new Blob(chunks);
}

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
      console.log('CLAW.REZ not found in storage.');

      // Check if device is offline
      if (!navigator.onLine) {
        console.log('Device is offline and CLAW.REZ not cached. Showing offline message...');
        showOfflineNeedCache();
        throw new Error('Offline: CLAW.REZ not cached. Please connect to internet first.');
      }

      console.log('Showing upload UI...');
      showAssetUpload();
      await waitForUpload();
    } else {
      console.log('CLAW.REZ found in storage. Loading...');

      // Get file metadata
      const metadata = await assetStorage.getFileMetadata('CLAW.REZ');
      console.log(`Stored size: ${(metadata.size / 1024 / 1024).toFixed(2)}MB`);

      if (metadata.compressed) {
        console.log(`Compression: ${metadata.compressionAlgorithm || 'gzip'}`);
      }
    }

    // Retrieve CLAW.REZ from IndexedDB
    console.log('Retrieving CLAW.REZ from IndexedDB...');
    const storedBlob = await assetStorage.getFile('CLAW.REZ');
    if (!storedBlob) {
      throw new Error('Failed to retrieve CLAW.REZ from storage');
    }

    // Check if file is compressed
    const metadata = await assetStorage.getFileMetadata('CLAW.REZ');
    let clawRezBlob = storedBlob;

    if (metadata.compressed && metadata.compressionAlgorithm) {
      console.log(`Decompressing CLAW.REZ using ${metadata.compressionAlgorithm}...`);
      const startTime = performance.now();

      try {
        clawRezBlob = await decompressBlob(storedBlob, metadata.compressionAlgorithm);
        const decompressTime = performance.now() - startTime;

        console.log(`Decompressed size: ${(clawRezBlob.size / 1024 / 1024).toFixed(2)}MB`);
        console.log(`Decompression took: ${decompressTime.toFixed(0)}ms`);
        console.log(`Compression ratio: ${((1 - storedBlob.size / clawRezBlob.size) * 100).toFixed(1)}%`);
      } catch (decompressError) {
        console.error('Decompression failed:', decompressError);
        alert(
          `Failed to decompress CLAW.REZ using ${metadata.compressionAlgorithm}.\n\n` +
          `Error: ${decompressError.message}\n\n` +
          `Please clear browser storage and re-upload CLAW.REZ.`
        );
        return false;
      }
    }

    // CRITICAL: Convert Blob to ArrayBuffer and store BEFORE returning
    // This ensures clawRezData is populated before loadGame() is called
    console.log('Loading CLAW.REZ into memory...');
    const arrayBuffer = await clawRezBlob.arrayBuffer();
    clawRezData = new Uint8Array(arrayBuffer);
    console.log(`CLAW.REZ ready to mount (${(clawRezData.length / 1024 / 1024).toFixed(2)}MB)`);

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
