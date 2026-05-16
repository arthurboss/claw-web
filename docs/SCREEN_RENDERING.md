# Screen Rendering Architecture

This document maps the various game screens to their rendering code locations. Since there's no central configuration for screen rendering, this serves as a reference for understanding and modifying screen behavior.

## Screen Types and Source Locations

### 1. Main Menu
- **Game State:** `GameState_Menu`
- **Rendering Code:** `OpenClaw/Engine/UserInterface/UserInterface.cpp`
  - `ScreenElementMenu::VOnRender()` - Main menu rendering (line ~157)
  - `ScreenElementMenuItem::VOnRender()` - Individual menu items
- **Configuration:** `Build_Release/ASSETS/MENU.xml`
- **Background Image:** `/STATES/MENU/SCREENS/MENU.PCX` (from CLAW.REZ)
- **Widescreen Support:** Yes - uses `g_MenuScale` and `g_MenuOffset` for centering

### 2. In-Game (Gameplay)
- **Game State:** `GameState_IngameRunning`
- **Rendering Code:** `OpenClaw/Engine/UserInterface/HumanView.cpp`
  - `HumanView::VOnRender()` - Main render loop (line ~62)
  - Iterates through `m_ScreenElements` and renders each
- **Scene Rendering:** `OpenClaw/Engine/Scene/Scene.cpp`
  - `Scene::OnRender()` - Renders all scene nodes
- **Camera:** `OpenClaw/Engine/Scene/CameraNode.cpp`
  - Controls visible area, supports dynamic width for widescreen
- **Widescreen Support:** Yes - camera width adjusts based on window aspect ratio

### 3. In-Game HUD
- **Rendering Code:** `OpenClaw/Engine/UserInterface/GameHUD.cpp`
  - `ScreenElementHUD` class
- **Elements:** Score, health, lives, ammo, powerups, boss health bar
- **Widescreen Support:** Needs verification

### 4. Quick Menu (In-Game Pause Menu)
- **Game State:** `GameState_IngamePaused`
- **Rendering Code:** `OpenClaw/Engine/UserInterface/UserInterface.cpp`
  - Same `ScreenElementMenu` class as main menu
  - `m_MenuType == MenuType_IngameMenu` for pause menu specific behavior
- **Background:** Semi-transparent overlay (game visible behind)
- **Widescreen Support:** Yes - shares centering logic with main menu

### 5. Loading Screen (Level Load)
- **Game State:** `GameState_LoadingLevel`
- **Rendering Code:** `OpenClaw/Engine/GameApp/BaseGameLogic.cpp`
  - `RenderLoadingScreen()` function (line ~190)
- **Background Image:** `/LEVEL{N}/SCREENS/LOADING.PCX` (per-level from CLAW.REZ)
- **Progress Bar:** Rendered programmatically (red bar on black background)
- **Widescreen Support:** Yes - centers 4:3 content with black letterboxing

### 6. End Level Score Screen
- **Game State:** `GameState_ScoreScreen`
- **Rendering Code:** `OpenClaw/Engine/UserInterface/ScoreScreen/EndLevelScoreScreen.cpp`
  - `ScreenElementScoreScreen::VOnRender()` (line ~368)
- **Configuration:** `Build_Release/ASSETS/FINISHED_LEVEL_SCENES/LEVEL{N}.XML`
- **Background Images:**
  - Intro: Level-specific image
  - Score: `/STATES/BOOTY/SCREENS/BOOTY.PCX`
- **Widescreen Support:** Yes - centers 4:3 content with black letterboxing

### 7. Cutscenes
- **Game State:** `GameState_Cutscene`
- **Rendering Code:** `OpenClaw/Engine/Video/` directory
- **Video Files:** AVI format from CLAW.REZ
- **Widescreen Support:** Unknown - needs investigation

### 8. Console Overlay
- **Rendering Code:** `OpenClaw/Engine/UserInterface/Console.cpp`
  - `Console::OnRender()` (line ~641 in HumanView.cpp calls it)
- **Activation:** Press `~` key
- **Widescreen Support:** Renders at full window width

## Widescreen Implementation Details

### Design Principles
- **Base Resolution:** 640x480 (4:3 aspect ratio)
- **Height Fixed:** Always render at 480px logical height
- **Width Dynamic:** Adjusts based on screen aspect ratio
- **Centering:** UI screens (menus, loading, score) are centered with black letterboxing
- **Gameplay:** Shows more game world horizontally (not stretched)

### Key Variables
```cpp
// In UserInterface.cpp
Point g_MenuScale;   // Uniform scale factor (based on height)
Point g_MenuOffset;  // Horizontal offset for centering

// In EndLevelScoreScreen.cpp
Point g_ScreenScale;  // Uniform scale factor
Point g_ScreenOffset; // Horizontal offset for centering
```

### Scaling Formula
```cpp
double uniformScale = windowSize.y / 480.0;
double offsetX = (windowSize.x - 640.0 * uniformScale) / 2.0;
```

## File Quick Reference

| Screen | Primary Source File | Config File |
|--------|---------------------|-------------|
| Main Menu | `UserInterface.cpp` | `ASSETS/MENU.xml` |
| Quick Menu | `UserInterface.cpp` | `ASSETS/INGAMEMENU.xml` |
| In-Game | `HumanView.cpp`, `Scene.cpp` | Level WWD files |
| HUD | `GameHUD.cpp` | Hardcoded |
| Loading | `BaseGameLogic.cpp` | None (hardcoded) |
| Score Screen | `EndLevelScoreScreen.cpp` | `ASSETS/FINISHED_LEVEL_SCENES/` |
| Console | `Console.cpp` | `config.xml` (appearance) |

## Adding Widescreen Support to a Screen

1. Calculate uniform scale based on 480px height
2. Calculate horizontal offset for centering
3. Clear screen with black before rendering
4. Render background centered using offset
5. Apply offset to all positioned elements
6. Recalculate each frame (resolution can change dynamically in fullscreen)

Example pattern:
```cpp
void SomeScreen::VOnRender(uint32 msDiff) {
    Point windowSize = g_pApp->GetWindowSize();
    const double DESIGN_WIDTH = 640.0;
    const double DESIGN_HEIGHT = 480.0;

    double uniformScale = windowSize.y / DESIGN_HEIGHT;
    double offsetX = (windowSize.x - DESIGN_WIDTH * uniformScale) / 2.0;

    // Clear with black
    SDL_SetRenderDrawColor(m_pRenderer, 0, 0, 0, 255);
    SDL_RenderClear(m_pRenderer);

    // Render content centered
    SDL_Rect dst = {
        (int)offsetX,
        0,
        (int)(DESIGN_WIDTH * uniformScale),
        (int)(DESIGN_HEIGHT * uniformScale)
    };
    SDL_RenderCopy(m_pRenderer, texture, &src, &dst);
}
```
