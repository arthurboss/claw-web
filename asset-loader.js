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
      console.log('CLAW.REZ not found in storage. Showing upload UI...');
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
