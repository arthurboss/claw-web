/**
 * IndexedDB wrapper for storing game save data
 * Provides persistent browser storage for save games
 */
export class SaveStorage {
  constructor() {
    this.dbName = 'OpenClawSaves';
    this.storeName = 'saves';
    this.db = null;
  }

  /**
   * Initialize IndexedDB connection
   * @returns {Promise<void>}
   */
  async init() {
    return new Promise((resolve, reject) => {
      const request = indexedDB.open(this.dbName, 1);

      request.onerror = () => reject(new Error('Failed to open SaveStorage IndexedDB'));

      request.onsuccess = (event) => {
        this.db = event.target.result;
        resolve();
      };

      request.onupgradeneeded = (event) => {
        const db = event.target.result;

        if (!db.objectStoreNames.contains(this.storeName)) {
          const objectStore = db.createObjectStore(this.storeName, { keyPath: 'key' });
          objectStore.createIndex('key', 'key', { unique: true });
          objectStore.createIndex('timestamp', 'timestamp', { unique: false });
        }
      };
    });
  }

  /**
   * Save game data to IndexedDB
   * @param {Object} saveData - The save game data (levels and checkpoints)
   * @returns {Promise<void>}
   */
  async saveSaveData(saveData) {
    if (!this.db) {
      throw new Error('Database not initialized. Call init() first.');
    }

    const record = {
      key: 'gameSaves',
      data: saveData,
      timestamp: Date.now()
    };

    return new Promise((resolve, reject) => {
      const transaction = this.db.transaction([this.storeName], 'readwrite');
      const objectStore = transaction.objectStore(this.storeName);
      const request = objectStore.put(record);

      request.onsuccess = () => {
        console.log('[SaveStorage] Game saved successfully');
        resolve();
      };

      request.onerror = () => {
        reject(new Error('Failed to save game data'));
      };
    });
  }

  /**
   * Load game save data from IndexedDB
   * @returns {Promise<Object|null>} The save data or null if not found
   */
  async loadSaveData() {
    if (!this.db) {
      throw new Error('Database not initialized. Call init() first.');
    }

    return new Promise((resolve, reject) => {
      const transaction = this.db.transaction([this.storeName], 'readonly');
      const objectStore = transaction.objectStore(this.storeName);
      const request = objectStore.get('gameSaves');

      request.onsuccess = (event) => {
        const result = event.target.result;
        if (result) {
          console.log('[SaveStorage] Loaded save data from', new Date(result.timestamp).toLocaleString());
          resolve(result.data);
        } else {
          console.log('[SaveStorage] No save data found');
          resolve(null);
        }
      };

      request.onerror = () => {
        reject(new Error('Failed to load save data'));
      };
    });
  }

  /**
   * Check if save data exists
   * @returns {Promise<boolean>}
   */
  async hasSaveData() {
    if (!this.db) {
      throw new Error('Database not initialized. Call init() first.');
    }

    return new Promise((resolve, reject) => {
      const transaction = this.db.transaction([this.storeName], 'readonly');
      const objectStore = transaction.objectStore(this.storeName);
      const request = objectStore.get('gameSaves');

      request.onsuccess = (event) => {
        resolve(!!event.target.result);
      };

      request.onerror = () => {
        reject(new Error('Failed to check save data existence'));
      };
    });
  }

  /**
   * Delete all save data
   * @returns {Promise<void>}
   */
  async deleteSaveData() {
    if (!this.db) {
      throw new Error('Database not initialized. Call init() first.');
    }

    return new Promise((resolve, reject) => {
      const transaction = this.db.transaction([this.storeName], 'readwrite');
      const objectStore = transaction.objectStore(this.storeName);
      const request = objectStore.delete('gameSaves');

      request.onsuccess = () => {
        console.log('[SaveStorage] Save data deleted');
        resolve();
      };

      request.onerror = () => {
        reject(new Error('Failed to delete save data'));
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

// Global instance for C++ bridge
let saveStorageInstance = null;

/**
 * Initialize save storage (called from C++ or game startup)
 */
export async function initSaveStorage() {
  if (!saveStorageInstance) {
    saveStorageInstance = new SaveStorage();
    await saveStorageInstance.init();
  }
  return saveStorageInstance;
}

/**
 * Get the global save storage instance
 */
export function getSaveStorage() {
  return saveStorageInstance;
}

// Bridge functions for C++ (using EM_ASM)
// These are called from C++ via EM_ASM and must be globally accessible

/**
 * Save game data to IndexedDB (called from C++)
 * @param {string} jsonData - JSON string of save data
 * @returns {Promise<boolean>} Success status
 */
window.saveGameToIndexedDB = async function(jsonData) {
  try {
    const storage = await initSaveStorage();
    const saveData = JSON.parse(jsonData);
    await storage.saveSaveData(saveData);
    return true;
  } catch (error) {
    console.error('[SaveStorage] Error saving game:', error);
    return false;
  }
};

/**
 * Load game data from IndexedDB (called from C++)
 * @returns {Promise<string|null>} JSON string of save data or null
 */
window.loadGameFromIndexedDB = async function() {
  try {
    const storage = await initSaveStorage();
    const saveData = await storage.loadSaveData();
    if (saveData) {
      return JSON.stringify(saveData);
    }
    return null;
  } catch (error) {
    console.error('[SaveStorage] Error loading game:', error);
    return null;
  }
};

/**
 * Check if save data exists (called from C++)
 * @returns {Promise<boolean>}
 */
window.hasSaveDataInIndexedDB = async function() {
  try {
    const storage = await initSaveStorage();
    return await storage.hasSaveData();
  } catch (error) {
    console.error('[SaveStorage] Error checking save data:', error);
    return false;
  }
};

/**
 * Delete save data (called from C++)
 * @returns {Promise<boolean>}
 */
window.deleteSaveDataFromIndexedDB = async function() {
  try {
    const storage = await initSaveStorage();
    await storage.deleteSaveData();
    return true;
  } catch (error) {
    console.error('[SaveStorage] Error deleting save data:', error);
    return false;
  }
};

// Keep window global for backward compatibility
if (typeof window !== 'undefined') {
  window.SaveStorage = SaveStorage;
  window.initSaveStorage = initSaveStorage;
  window.getSaveStorage = getSaveStorage;
}
