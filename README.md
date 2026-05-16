# OpenClaw WASM

> Play Captain Claw (1997) in your web browser - no installation needed!

Browser-based version of the classic platformer. Based on [OpenClaw](https://github.com/pjasicek/OpenClaw) by pjasicek.

| Main Menu | Level 1 Gameplay |
|-----------|------------------|
| ![Game Menu](docs/screenshots/game-menu.png) | ![Level 1 Gameplay](docs/screenshots/gameplay-level1.png) |

## 🎮 Quick Start

### Requirements

You'll need **CLAW.REZ** from the original Captain Claw (1997) game.

- The game will ask you to upload this file on first run
- It's saved in your browser - you only upload once
- **Legal:** You must own the original game to use its assets

**Optional:** Video cutscenes from the original game are not included. The game works perfectly without them, but you can add them to `Build_Release/videos/` if you want cutscenes between levels.

### How to Play

1. **Start a local server by running the following in your computer's Terminal application** (pick one):

   ```bash
   # Recommended: HTTP/3 server (faster)
   ./scripts/start_http3_server.sh

   # Alternative: Python server
   cd Build_Release
   python3 -m http.server 8080
   ```

2. **Open in browser:**
   - HTTP/3: <https://localhost:8080/openclaw.html>
   - Python: <http://localhost:8080/openclaw.html>

3. **First time only:** Upload your CLAW.REZ file when prompted

4. **Play!** The game loads in 2-3 seconds

### Browser Requirements

- Chrome 105+ / Firefox 121+ / Safari 16.4+ / Edge 105+
- ~120MB free storage (for compressed game files and game states in IndexedDB)

## 💡 Features

- **Fast Loading:** Only 48MB download, levels load as you play
- **One-Time Setup:** Upload CLAW.REZ once, play forever
- **Full Game:** All 14 levels, original graphics and audio
- **Modern Browsers:** Hardware-accelerated WebGL rendering
- **No Installation:** Runs entirely in your browser

## 🐛 Troubleshooting

Having issues? See [Troubleshooting](docs/player/TROUBLESHOOTING.md) for:

- How to clear cached files and re-upload CLAW.REZ
- Common error messages and solutions
- Browser-specific problems

## 📋 Documentation

- **[Setup Guide](docs/player/SETUP.md)** - Detailed setup instructions
- **[Troubleshooting](docs/player/TROUBLESHOOTING.md)** - Common issues and fixes

### For Developers

See [docs/developer/](docs/developer/) for technical documentation:

- [Building](docs/developer/BUILDING.md) - Compiling from source
- [Architecture](docs/developer/ARCHITECTURE.md) - Lazy-loading and resource management

## 📝 License

GNU GPL v3 - See LICENSE file

Original game assets (CLAW.REZ) remain copyright Monolith Productions.

## 🙏 Credits

- **Original Game:** Monolith Productions (1997)
- **OpenClaw Engine:** [pjasicek](https://github.com/pjasicek/OpenClaw)
- **WASM port fix and modernisation:** Arthur Boss

---

**Note:** This is a WASM-only fork optimized for browsers. For native desktop builds, visit the [original OpenClaw repository](https://github.com/pjasicek/OpenClaw).
