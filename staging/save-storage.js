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
  if (!input) {
    input = document.createElement('input');
    input.type = 'file';
    input.id = 'importSaveInput';
    input.accept = '.json';
    input.style.display = 'none';
    input.addEventListener('change', function(e) {
      var file = e.target.files[0];
      if (!file) return;
      var reader = new FileReader();
      reader.onload = function(ev) {
        try {
          JSON.parse(ev.target.result);
          localStorage.setItem('openclaw:saves', ev.target.result);
          console.log('[SaveStorage] Save data imported');
          if (Module && Module._OnJSSaveDataImported) {
            Module._OnJSSaveDataImported();
          }
        } catch (err) {
          console.error('[SaveStorage] Import error: invalid JSON', err);
        }
      };
      reader.readAsText(file);
      input.value = '';
    });
    document.body.appendChild(input);
  }
  input.click();
};
