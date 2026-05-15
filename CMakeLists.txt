cmake_minimum_required(VERSION 4.1.0)

# WASM-only build configuration
# This fork targets WebAssembly exclusively via Emscripten
option(Extern_Config "Do not embed config file" ON)

project(OpenClaw)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -g")

# CMake compiled directories with dependencies
add_subdirectory(Box2D)
add_subdirectory(libwap)
add_subdirectory(./ThirdParty/Tinyxml)

# Directories with headers
include_directories(./Box2D/)
include_directories(./libwap/)
include_directories(./ThirdParty/)
include_directories(./ThirdParty/Tinyxml/)

# Emscripten-specific configuration
set(EMSCRIPTEN_PATH "$ENV{EMSDK}")
include_directories("${EMSCRIPTEN_PATH}/upstream/emscripten/cache/sysroot/include/emscripten/")
add_definitions(-DSDL_VIDEO_OPENGL_ES2=1)
add_definitions(-DSDL_VIDEO_OPENGL_ES=1)
add_definitions(-DSDL_VIDEO_OPENGL=1)
add_definitions(-DSDL_VIDEO_OPENGL_ES2_DISABLE_EXTERNAL_TEXTURE=1)
add_definitions(-DSDL_VIDEO_OPENGL_ES2_DISABLE_ANDROID_TEXTURE=1)

# Directories with libs
link_directories(./libwap)
link_directories(./Box2D)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../Build_Release)

add_executable(openclaw "")

add_subdirectory(OpenClaw)

# Copy JavaScript graphics bridges for WASM builds
add_custom_command(TARGET openclaw POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_SOURCE_DIR}/OpenClaw/Engine/Graphics/WASM/js/graphics-bridge.js
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/graphics-bridge.js
    COMMENT "Copying WebGL graphics bridge JavaScript file"
)
add_custom_command(TARGET openclaw POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_SOURCE_DIR}/OpenClaw/Engine/Graphics/WASM/js/texture-bridge.js
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/texture-bridge.js
    COMMENT "Copying texture bridge JavaScript file"
)

# Linker settings
list(APPEND TARGET_LIBS
    libwap
    tinyxml
    Box2D
)

if (Extern_Config)
    set(CONFIG_PRELOAD_FILE "")
else (Extern_Config)
    set(CONFIG_PRELOAD_FILE "--preload-file ../Build_Release/config.xml@config.xml")
endif (Extern_Config)

# Emscripten linker flags
# Lazy loading architecture: CLAW.REZ loaded from IndexedDB at runtime
# Only bundle critical assets (ASSETS.ZIP, console, fonts) to minimize initial download
# EXPORTED_FUNCTIONS includes gamepad bridge callbacks for JavaScript to call via ccall
# EXPORTED_RUNTIME_METHODS includes ccall for JavaScript to invoke exported C functions
set_target_properties(openclaw PROPERTIES LINK_FLAGS "-s FETCH -s WASM=1 -s BINARYEN_METHOD='native-wasm' -s EXPORTED_FUNCTIONS='[_main,_OnJSGamepadConnected,_OnJSGamepadDisconnected,_OnJSGamepadButtonDown,_OnJSGamepadButtonUp,_OnJSGamepadAxis,_GetJSGameState]' -s EXPORTED_RUNTIME_METHODS='[ccall,cwrap]' -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='[\"pcx\",\"png\",\"tga\"]' -s USE_SDL_TTF=2 -s USE_SDL_GFX=2 -s USE_SDL_MIXER=2 -s ASYNCIFY=1 -s TOTAL_MEMORY=268435456 -s FULL_ES3=1 -s USE_WEBGL2=1 -s MIN_WEBGL_VERSION=2 -s MAX_WEBGL_VERSION=2 ${CONFIG_PRELOAD_FILE} --preload-file ../Build_Release/ASSETS.ZIP@ASSETS.ZIP --preload-file ../Build_Release/console02.tga@console02.tga --preload-file ../Build_Release/clacon.ttf@clacon.ttf")

target_link_libraries(openclaw ${TARGET_LIBS})
