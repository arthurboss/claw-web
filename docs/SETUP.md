# OpenClaw WASM Setup Guide

## Overview

OpenClaw is a web-based reimplementation of Captain Claw (1997), a classic platformer game. This guide explains how to set up and run the game in your browser with modern lazy loading architecture.

## Requirements

### 1. CLAW.REZ (Required)

**What:** Original game assets from Captain Claw (1997)
**Size:** ~113 MB
**Legal:** You must own the original game to use this file

#### Where to obtain CLAW.REZ

- **Original CD/Installation:** You must own a legitimate copy of Captain Claw (1997)
- The game is no longer available for digital purchase
- You must own the original physical release

#### How to extract from original game

1. Install Captain Claw (1997) from your original disc
2. Navigate to installation directory (usually `C:\Program Files\Monolith\Claw`)
3. Copy `CLAW.REZ` file (~113 MB)
4. Upload to OpenClaw web interface (see First-Time Setup below)

### 2. Browser Requirements

- **Modern browser with WebAssembly support:**
  - Chrome 105+ (recommended)
  - Firefox 121+
  - Safari 16.4+
  - Edge 105+
- **IndexedDB enabled** (for asset storage)
- **~150 MB available storage space** (browser local storage)

## First-Time Setup

1. **Open OpenClaw in your browser**
   - Navigate to the game URL or run local HTTP server
   - You'll see a "First Time Setup" screen

2. **Upload CLAW.REZ**
   - Click "Choose File" and select your CLAW.REZ
   - Click "Upload CLAW.REZ"
   - Wait for upload to complete (progress bar shows status)

3. **Asset storage**
   - CLAW.REZ is stored in your browser's IndexedDB
   - You only need to upload it once
   - It persists across browser sessions

4. **Game starts automatically**
   - After upload completes, the game loads
   - Subsequent visits load instantly from cache

## Architecture (2026 Modern Web Standards)

OpenClaw uses a lazy loading architecture optimized for WASM and modern browsers:

- **Minimal initial download:** ~5 MB (vs 160 MB before)
  - `openclaw.wasm` - Game engine (~47 MB)
  - `openclaw.data` - Critical assets only (<1 MB)
  - `openclaw.js` - JavaScript glue (~400 KB)

- **User-provided assets:** CLAW.REZ stored in IndexedDB
  - Loaded once from user's file
  - Cached forever in browser storage
  - No re-downloading needed

- **Progressive loading:** Assets load on-demand
  - Menu/UI assets load first
  - Level assets load when entering level
  - Faster startup, lower bandwidth usage

## Legal Notice

### OpenClaw Code

OpenClaw is an open-source game engine licensed under **GNU GPL v3**.

- Source code: <https://github.com/arthurboss/OpenClaw>
- You may modify and distribute under GPL v3 terms

### Captain Claw Assets (CLAW.REZ)

The original Captain Claw game assets are **copyrighted by Monolith Productions**.

- You must own a legitimate copy of Captain Claw (1997)
- Assets are NOT included with OpenClaw
- Users must provide their own CLAW.REZ file
- Do not share or distribute CLAW.REZ files

**Summary:** OpenClaw provides the engine; you provide the legally-owned assets.

## Troubleshooting

### "Upload failed"

**Possible causes:**

- File is not named CLAW.REZ (case-insensitive)
- File size is incorrect (~113 MB expected)
- Browser storage quota exceeded

**Solutions:**

1. Verify file is exactly named `CLAW.REZ`
2. Check file size is ~113 MB
3. Clear browser cache and try again
4. Check browser storage settings (may be disabled)

### "Out of storage"

**Possible causes:**

- Browser storage quota too low
- Other sites using storage space

**Solutions:**

1. Check browser storage quota:
   - Chrome: Settings → Privacy and security → Site Settings → Storage
   - Firefox: about:preferences#privacy → Cookies and Site Data
2. Clear other site data to free space
3. Try different browser

### "Assets not loading"

**Possible causes:**

- IndexedDB disabled in browser
- Corrupted asset storage
- Browser compatibility issue

**Solutions:**

1. Open browser console (F12) for error messages
2. Check IndexedDB is enabled:
   - Chrome: chrome://settings/content/all → IndexedDB
   - Firefox: about:config → dom.indexedDB.enabled
3. Try re-uploading CLAW.REZ:
   - Open browser console
   - Run: `reuploadClawRez()`
   - Upload file again

### "Slow performance"

**Possible causes:**

- Browser hardware acceleration disabled
- Insufficient system resources
- Too many background tabs

**Solutions:**

1. Enable hardware acceleration:
   - Chrome: Settings → System → Use hardware acceleration
   - Firefox: about:preferences → Performance
2. Close other tabs/applications
3. Use recommended browsers (Chrome/Edge perform best)

### "WebGL context lost"

**Possible causes:**

- GPU driver crash
- Browser tab suspended
- System resource exhaustion

**Solutions:**

1. Reload the page (CLAW.REZ still cached)
2. Update graphics drivers
3. Close other GPU-intensive applications

## Advanced: Storage Management

### View storage stats

Open browser console (F12) and run:

```javascript
getStorageStats().then(stats => console.log(stats))
```

Output:

```
{
  files: ["CLAW.REZ"],
  totalSize: 118489600,
  totalSizeMB: "113.00"
}
```

### Clear cached assets

To re-upload CLAW.REZ (if corrupted):

```javascript
reuploadClawRez()
```

### Manually delete storage

1. Open browser DevTools (F12)
2. Go to "Application" tab (Chrome) or "Storage" tab (Firefox)
3. Expand "IndexedDB"
4. Find "OpenClawAssets"
5. Delete database

## Building from Source

See main [README.md](README.md) for build instructions.

**Important:** The WASM build does NOT bundle CLAW.REZ. Users provide it at runtime via the upload UI.

## Support

- **Issues:** <https://github.com/arthurboss/OpenClaw/issues>
- **Original game:** Captain Claw (1997) by Monolith Productions
- **This fork:** WASM-only implementation with modern lazy loading

## Credits

- **Original OpenClaw:** <https://github.com/pjasicek/OpenClaw>
- **Captain Claw:** Monolith Productions (1997)
- **WASM Fork Maintainer:** Arthur Boss
