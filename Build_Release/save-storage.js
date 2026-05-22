/**
 * Save data export/import helpers (called from C++ via EM_ASM)
 * Save progress is stored in localStorage under 'openclaw:saves'.
 */

window.exportSaveData = function() {
  try {
    var jsonStr = localStorage.getItem('openclaw:saves');
    if (!jsonStr) {
      console.warn('[SaveStorage] No save data to export');
      return;
    }
    var blob = new Blob([jsonStr], { type: 'application/json' });
    var url = URL.createObjectURL(blob);
    var a = document.createElement('a');
    a.href = url;
    a.download = 'openclaw_save.json';
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
    URL.revokeObjectURL(url);
    console.log('[SaveStorage] Save data exported');
  } catch (e) {
    console.error('[SaveStorage] Export error:', e);
  }
};

window.importSaveData = function() {
  var input = document.getElementById('importSaveInput');
  if (input) input.click();
};
