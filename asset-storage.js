/**
 * IndexedDB wrapper for storing game assets (CLAW.REZ)
 * Provides persistent browser storage for large binary files
 */
export class AssetStorage {
  constructor() {
    this.dbName = 'OpenClawAssets';
    this.storeName = 'files';
    this.db = null;
    this.chunkSize = 1024 * 1024 * 5; // 5MB chunks to prevent memory issues
  }

  /**
   * Initialize IndexedDB connection
   * @returns {Promise<void>}
   */
  async init() {
    return new Promise((resolve, reject) => {
      const request = indexedDB.open(this.dbName, 1);

      request.onerror = () => reject(new Error('Failed to open IndexedDB'));

      request.onsuccess = (event) => {
        this.db = event.target.result;
        resolve();
      };

      request.onupgradeneeded = (event) => {
        const db = event.target.result;

        // Create object store if it doesn't exist
        if (!db.objectStoreNames.contains(this.storeName)) {
          const objectStore = db.createObjectStore(this.storeName, { keyPath: 'name' });
          objectStore.createIndex('name', 'name', { unique: true });
          objectStore.createIndex('timestamp', 'timestamp', { unique: false });
        }
      };
    });
  }

  /**
   * Store a file in IndexedDB with progress tracking
   * @param {string} name - File name
   * @param {Blob} blob - File data
   * @param {Function} onProgress - Progress callback (loaded, total)
   * @returns {Promise<void>}
   */
  async storeFile(name, blob, onProgress = null) {
    if (!this.db) {
      throw new Error('Database not initialized. Call init() first.');
    }

    // Detect compression algorithm from MIME type
    const compressionMap = {
      'application/zstd': 'zstd',
      'application/br': 'br',
      'application/gzip': 'gzip',
      'application/x-gzip': 'gzip'
    };

    const isCompressed = compressionMap.hasOwnProperty(blob.type);
    const compressionAlgorithm = compressionMap[blob.type] || null;

    // Store metadata and blob
    const fileData = {
      name: name,
      blob: blob,
      size: blob.size,
      type: blob.type,
      timestamp: Date.now(),
      compressed: isCompressed,
      compressionAlgorithm: compressionAlgorithm
    };

    return new Promise((resolve, reject) => {
      const transaction = this.db.transaction([this.storeName], 'readwrite');
      const objectStore = transaction.objectStore(this.storeName);

      const request = objectStore.put(fileData);

      request.onsuccess = () => {
        if (onProgress) {
          onProgress(blob.size, blob.size);
        }
        resolve();
      };

      request.onerror = () => {
        reject(new Error(`Failed to store file: ${name}`));
      };
    });
  }

  /**
   * Retrieve a file from IndexedDB
   * @param {string} name - File name
   * @returns {Promise<Blob|null>}
   */
  async getFile(name) {
    if (!this.db) {
      throw new Error('Database not initialized. Call init() first.');
    }

    return new Promise((resolve, reject) => {
      const transaction = this.db.transaction([this.storeName], 'readonly');
      const objectStore = transaction.objectStore(this.storeName);
      const request = objectStore.get(name);

      request.onsuccess = (event) => {
        const result = event.target.result;
        resolve(result ? result.blob : null);
      };

      request.onerror = () => {
        reject(new Error(`Failed to retrieve file: ${name}`));
      };
    });
  }

  /**
   * Check if a file exists in IndexedDB
   * @param {string} name - File name
   * @returns {Promise<boolean>}
   */
  async hasFile(name) {
    if (!this.db) {
      throw new Error('Database not initialized. Call init() first.');
    }

    return new Promise((resolve, reject) => {
      const transaction = this.db.transaction([this.storeName], 'readonly');
      const objectStore = transaction.objectStore(this.storeName);
      const request = objectStore.get(name);

      request.onsuccess = (event) => {
        resolve(!!event.target.result);
      };

      request.onerror = () => {
        reject(new Error(`Failed to check file existence: ${name}`));
      };
    });
  }

  /**
   * Delete a file from IndexedDB
   * @param {string} name - File name
   * @returns {Promise<void>}
   */
  async deleteFile(name) {
    if (!this.db) {
      throw new Error('Database not initialized. Call init() first.');
    }

    return new Promise((resolve, reject) => {
      const transaction = this.db.transaction([this.storeName], 'readwrite');
      const objectStore = transaction.objectStore(this.storeName);
      const request = objectStore.delete(name);

      request.onsuccess = () => resolve();
      request.onerror = () => reject(new Error(`Failed to delete file: ${name}`));
    });
  }

  /**
   * Get metadata about a file
   * @param {string} name - File name
   * @returns {Promise<Object|null>}
   */
  async getFileMetadata(name) {
    if (!this.db) {
      throw new Error('Database not initialized. Call init() first.');
    }

    return new Promise((resolve, reject) => {
      const transaction = this.db.transaction([this.storeName], 'readonly');
      const objectStore = transaction.objectStore(this.storeName);
      const request = objectStore.get(name);

      request.onsuccess = (event) => {
        const result = event.target.result;
        if (result) {
          resolve({
            name: result.name,
            size: result.size,
            type: result.type,
            timestamp: result.timestamp,
            compressed: result.compressed || false,
            compressionAlgorithm: result.compressionAlgorithm || null
          });
        } else {
          resolve(null);
        }
      };

      request.onerror = () => {
        reject(new Error(`Failed to retrieve metadata: ${name}`));
      };
    });
  }

  /**
   * List all stored files
   * @returns {Promise<Array>}
   */
  async listFiles() {
    if (!this.db) {
      throw new Error('Database not initialized. Call init() first.');
    }

    return new Promise((resolve, reject) => {
      const transaction = this.db.transaction([this.storeName], 'readonly');
      const objectStore = transaction.objectStore(this.storeName);
      const request = objectStore.getAllKeys();

      request.onsuccess = (event) => {
        resolve(event.target.result);
      };

      request.onerror = () => {
        reject(new Error('Failed to list files'));
      };
    });
  }

  /**
   * Get total storage size used
   * @returns {Promise<number>}
   */
  async getStorageSize() {
    if (!this.db) {
      throw new Error('Database not initialized. Call init() first.');
    }

    return new Promise((resolve, reject) => {
      const transaction = this.db.transaction([this.storeName], 'readonly');
      const objectStore = transaction.objectStore(this.storeName);
      const request = objectStore.getAll();

      request.onsuccess = (event) => {
        const files = event.target.result;
        const totalSize = files.reduce((sum, file) => sum + (file.size || 0), 0);
        resolve(totalSize);
      };

      request.onerror = () => {
        reject(new Error('Failed to calculate storage size'));
      };
    });
  }

  /**
   * Close database connection
   */
  close() {
    if (this.db) {
      this.db.close();
      this.db = null;
    }
  }
}

// Keep window global for backward compatibility during transition
if (typeof window !== 'undefined') {
  window.AssetStorage = AssetStorage;
}
