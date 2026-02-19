#ifndef __BASEGAMEAPP_H__
#define __BASEGAMEAPP_H__

#include "../SharedDefines.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <tinyxml.h>


#include "../Graphics/GraphicsAdapter.h"
#include "../UserInterface/Console.h"
#include "../UserInterface/Touch/TouchManager.h"
#include "../Video/Generic/GenericVideoModule.h"
#include "CommandHandler.h"


#include "../Platform/Events/AppEventQueue.h"
#include "../Platform/Video/VideoPlatform.h"

const int DEFAULT_SCREEN_WIDTH = 1280;
const int DEFAULT_SCREEN_HEIGHT = 768;

#ifndef FAIL
#define FAIL(reason)                                                           \
  do {                                                                         \
    LOG_ERROR(reason);                                                         \
    IEventDataPtr pQuitEvent(new EventData_Quit_Game());                       \
    IEventMgr::Get()->VTriggerEvent(pQuitEvent);                               \
  } while (0);
#endif

struct GameOptions {
  void SetDefaults() {
    windowWidth = 1280;
    windowHeight = 780;
    scale = 1.0f;
    useVerticalSync = true;
    isFullscreen = false;
    isFullscreenDesktop = false;

    frequency = 44100;
    soundChannels = 2;
    mixingChannels = 24;
    chunkSize = 2048;
    soundVolume = 50; // In percents
    musicVolume = 50; // In percents
    soundOn = true;
    musicOn = true;
    midiRpcServerPath = "MidiProc.exe";

    fontNames.push_back("clacon.ttf");
    consoleFontName = "clacon.ttf";
    consoleFontSize = 20;

    rezArchive = "CLAW.REZ";
    customArchive = "ASSETS.ZIP";
    resourceCacheSize = 50;
    tempDir = ".";
    savesFile = "SAVES.XML";
    userDirectory = "";

    startupCommandsFile = "startup_commands.txt";
  }

  GameOptions() { SetDefaults(); }

  void SetSavesFilePath(std::string savesPath) { savesFile = savesPath; }

  // Display options
  int windowWidth;
  int windowHeight;
  double scale;
  bool useVerticalSync;
  bool isFullscreen;
  bool isFullscreenDesktop;

  // Audio
  unsigned frequency;
  unsigned soundChannels;
  unsigned mixingChannels;
  unsigned chunkSize;
  int soundVolume;
  int musicVolume;
  bool soundOn;
  bool musicOn;
  std::string midiRpcServerPath;

  // Font
  std::vector<const char *> fontNames;
  std::string consoleFontName;
  unsigned consoleFontSize;

  // Assets
  std::string assetsFolder;
  std::string rezArchive;
  std::string customArchive;
  unsigned resourceCacheSize;
  std::string tempDir;
  std::string savesFile;
  // For LINUX ONLY - this is generally ~/.config/openclaw/
  std::string userDirectory;

  // Console config
  ConsoleConfig consoleConfig;

  // File with prewritten commands which are executed upon startup of the game
  std::string startupCommandsFile;
};

// Cheats and stuff
struct GameCheats {
  GameCheats() {
    showPhysicsDebug = false;

    clawInfiniteAmmo = false;
    clawInvincible = false;
    clawInfiniteJump = false;
  }

  // Environment
  bool showPhysicsDebug;

  // Claw
  bool clawInfiniteAmmo;
  bool clawInvincible;
  bool clawInfiniteJump;
};

// Put everything you want to be configurable here without
// worrying about parsing from XML first. Used mainly by console for fast
// iteration
struct GlobalOptions {
  GlobalOptions() {
    maxJumpSpeed = 8.8;
    maxFallSpeed = 14.0;
    idleSoundQuoteIntervalMs = 15000;
    platformSpeedModifier = 0.015;
    runSpeed = 4.5f;
    powerupRunSpeed = 5.5f;
    maxJumpHeight = 150;
    powerupMaxJumpHeight = 200;
    startLookUpOrDownTime = 1500;
    freezeTime = 2000;
    maxLookUpOrDownDistance = 250;
    lookUpOrDownSpeed = 250;
    clawRunningSpeed = 5.0;
    // springBoardSpringHeight = 450;
    springBoardSpringSpeed = 11;
    clawMinFallHeight = 500.0f;
    loadAllLevelSaves = false;
    showFps = true;
    showPosition = true;
    gameLogicFps = 60;  // Game logic update rate (independent of display FPS)
  }

  double maxJumpSpeed;
  double maxFallSpeed;
  int idleSoundQuoteIntervalMs;
  double platformSpeedModifier;
  float runSpeed;
  float powerupRunSpeed;
  float maxJumpHeight;
  float powerupMaxJumpHeight;
  int startLookUpOrDownTime;
  int freezeTime;
  int maxLookUpOrDownDistance;
  int lookUpOrDownSpeed;
  std::string scoreScreenPalPath;
  double clawRunningSpeed;
  // int springBoardSpringHeight;
  double springBoardSpringSpeed;
  float clawMinFallHeight;
  bool loadAllLevelSaves;
  bool showFps;
  bool showPosition;
  int gameLogicFps;  // Target game logic update rate (default: 60)
};

struct ControlOptions {
  ControlOptions() {
    useAlternateControls = false;
    touchScreen.enable = false;
    touchScreen.distanceThreshold = 0.1;
    touchScreen.timeThreshold = 100;
  }
  bool useAlternateControls;
  struct {
    bool enable;
    float distanceThreshold;
    unsigned int timeThreshold;
  } touchScreen;
};

struct DebugOptions {
  DebugOptions() {
    cpuDelayMs = 0;
    bSkipBossFightIntro = false;
    bSkipMenu = false;
    lastImplementedLevel = 7;
    skipMenuToLevel = 9;
  }

  int cpuDelayMs;
  bool bSkipMenu;
  int lastImplementedLevel;
  int skipMenuToLevel;
  bool bSkipBossFightIntro;
};

struct LevelMetadata {
  int levelNumber;
  std::string levelName;
  std::map<std::string, ActorPrototype> logicToActorPrototypeMap;
  std::map<int, Point> checkpointNumberToSpawnPositionMap;
  std::map<int, Point> tileIdToTopLadderEndMap;
  std::string tileDeathEffectType;
  Point tileDeathEffectOffset;
};

class EventMgr;
class BaseGameLogic;
class HumanView;
class ResourceCache;
class IResourceMgr;
class Audio;

typedef std::map<std::string, std::string> LocalizedStringsMap;
typedef std::map<std::string, TTF_Font *> FontMap;
typedef std::map<ActorPrototype, const TiXmlElement *> ActorXmlPrototypeMap;
typedef std::map<int, shared_ptr<LevelMetadata>> LevelMetadataMap;

class BaseGameApp {
  // Command handler should have unlimited access
  friend class CommandHandler;
  friend void Loop(void *instance);

public:
  BaseGameApp();

  // Must be defined in inherited class
  virtual const char *VGetGameTitle() = 0;
  virtual const char *VGetGameAppDirectory() = 0;
  virtual BaseGameLogic *VCreateGameAndView() = 0;

  // Icon ?

  virtual bool Initialize(int argc, char **argv);
  virtual void VPostInitialize() {}
  virtual void Terminate();

  // HW Events
  void OnEvent(SDL_Event &event);
  void OnAppEvent(const AppEvent &event);
  void OnDisplayChange(int newWidth, int newHeight);
  void VOnRestore();
  void VOnMinimized();

  // Main loop
  int32 Run();
  void StepLoop();

  // This is provided to be used the engine
  bool LoadStrings(std::string language);
  std::string GetString(std::string stringId);
  Point GetScale();
  void SetScale(Point scale);
  uint32 GetWindowFlags();

  inline SDL_Renderer *GetRenderer() const { return m_pRenderer; }
  // TODO: Memory leak most likely
  inline WapPal *GetCurrentPalette() const { return m_pPalette; }
  void SetCurrentPalette(WapPal *palette) { m_pPalette = palette; }
  // Deprecated. Use GetResourceMgr()
  std::shared_ptr<ResourceCache> GetResourceCache() const;
  inline IResourceMgr *GetResourceMgr() const { return m_pResourceMgr; }

  BaseGameLogic *GetGameLogic() const { return m_pGame; }
  HumanView *GetHumanView() const;

  SDL_Window *GetWindow() const { return m_pWindow; }
  Point GetWindowSize() { return m_WindowSize; }
  void SetWindowSize(int width, int height, double scale);
  Point GetWindowSizeScaled() {
    return Point(m_WindowSize.x / GetScale().x, m_WindowSize.y / GetScale().y);
  }
  void RequestWindowSizeChange(Point newSize, bool fullscreen);

  inline EventMgr *GetEventMgr() const { return m_pEventMgr; }

  TTF_Font *GetConsoleFont() const { return m_pConsoleFont; }

  Audio *GetAudio() const { return m_pAudio; }

  // Graphics system access
  GraphicsAdapter *GetGraphicsAdapter() { return m_graphicsAdapter.get(); }
  GenericVideoModule *GetVideoModule() { return m_videoModule.get(); }
  bool InitializeGraphicsSystem();
  void ShutdownGraphicsSystem();

  bool LoadGameOptions(const char *inConfigFile = "config.xml");
  void SaveGameOptions(const char *outConfigFile = "config.xml");

  bool LoadLevel(const char *levelResource);

  const GameCheats *GetGameCheats() const { return &m_GameCheats; }
  const ConsoleConfig *GetConsoleConfig() const {
    return &m_GameOptions.consoleConfig;
  }
  GameOptions *GetGameConfig() { return &m_GameOptions; }
  const GlobalOptions *GetGlobalOptions() const { return &m_GlobalOptions; }
  const ControlOptions *GetControlOptions() const { return &m_ControlOptions; }
  const DebugOptions *GetDebugOptions() const { return &m_DebugOptions; }

  // FPS tracking
  uint32 GetLogicFPS() const { return m_lastLogicFPS; }
  uint32 GetRenderFPS() const { return m_lastRenderFPS; }

  TiXmlElement *GetActorPrototypeElem(ActorPrototype proto);

  const shared_ptr<LevelMetadata> GetLevelMetadata(int levelNumber) const;

  void RegisterTouchRecognizers(ITouchHandler &touchHandler);

protected:
  virtual void VRegisterGameEvents() {}
  virtual bool VPerformStartupTests();

  BaseGameLogic *m_pGame;
  IResourceMgr *m_pResourceMgr;
  EventMgr *m_pEventMgr;
  TTF_Font *m_pConsoleFont;
  Audio *m_pAudio;
  TouchManager *m_pTouchManager;

  // Graphics system
  std::unique_ptr<GraphicsAdapter> m_graphicsAdapter;
  std::unique_ptr<GenericVideoModule> m_videoModule;

  // Stage 2a: platform event pipeline (WASM first).
  AppEventQueue m_appEventQueue;
  std::unique_ptr<VideoPlatform> m_videoPlatform;

  LocalizedStringsMap m_LocalizedStringsMap;
  FontMap m_FontMap;

  GameOptions m_GameOptions;

private:
  bool InitializeDisplay(GameOptions &gameOptions);
  bool InitializeAudio(GameOptions &gameOptions);
  bool InitializeResources(GameOptions &gameOptions);
  bool InitializeFont(GameOptions &gameOptions);
  bool InitializeLocalization(GameOptions &gameOptions);
  bool InitializeTouchManager(GameOptions &gameOptions);
  bool InitializeEventMgr();
  bool ReadConsoleConfig();
  bool ReadActorXmlPrototypes(GameOptions &gameOptions);
  bool ReadLevelMetadata(GameOptions &gameOptions);

  void RegisterEngineEvents();

  // Event delegates
  void RegisterAllDelegates();
  void RemoveAllDelegates();

  void QuitGameDelegate(IEventDataPtr pEventData);

  TiXmlDocument CreateAndReturnDefaultConfig(const char *inConfigFile);

  SDL_Window *m_pWindow;
  SDL_Renderer *m_pRenderer;
  WapPal *m_pPalette;

  bool m_IsRunning;
  bool m_QuitRequested;
  bool m_IsQuitting;

  Point m_WindowSize;

  GameCheats m_GameCheats;
  GlobalOptions m_GlobalOptions;
  ControlOptions m_ControlOptions;
  DebugOptions m_DebugOptions;

  ActorXmlPrototypeMap m_ActorXmlPrototypeMap;
  LevelMetadataMap m_LevelMetadataMap;

  // Fixed timestep variables for frame-rate independent game logic
  double m_accumulator;         // Accumulated time for fixed updates
  double m_fixedTimestep;       // Target physics/logic timestep in seconds (e.g., 1/60 for 60Hz)
  double m_alpha;               // Interpolation factor for rendering between updates

  // FPS tracking
  uint32 m_logicUpdateCount;    // Number of logic updates in current second
  uint32 m_renderFrameCount;    // Number of render frames in current second
  uint32 m_fpsCounterMs;        // Time accumulator for FPS calculation
  uint32 m_lastLogicFPS;        // Last measured logic update rate
  uint32 m_lastRenderFPS;       // Last measured render frame rate
};

extern BaseGameApp *g_pApp;

#endif
