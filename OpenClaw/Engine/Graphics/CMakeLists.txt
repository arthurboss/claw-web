cmake_minimum_required(VERSION 4.1.0)

# Conditional compilation for WASM builds
if (EMSCRIPTEN)
    # WASM: Include pure WebGL renderer (no SDL dependencies)
    target_sources(openclaw
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/IRenderer.h
        ${CMAKE_CURRENT_SOURCE_DIR}/GraphicsManager.h
        ${CMAKE_CURRENT_SOURCE_DIR}/GraphicsManager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/GraphicsAdapter.h
        ${CMAKE_CURRENT_SOURCE_DIR}/GraphicsAdapter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Data/MenuBackgroundData.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Data/MenuItemData.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Data/MenuTextData.h
        ${CMAKE_CURRENT_SOURCE_DIR}/WASM/PureWebGLRenderer.h
        ${CMAKE_CURRENT_SOURCE_DIR}/WASM/PureWebGLRenderer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/WASM/PureWebGPURenderer.h
        ${CMAKE_CURRENT_SOURCE_DIR}/WASM/PureWebGPURenderer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/WASM/TextureManager.h
        ${CMAKE_CURRENT_SOURCE_DIR}/WASM/TextureManager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/GraphicsTest.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/IntegrationTest.cpp
    )
    
    # Add WASM-specific definitions
    target_compile_definitions(openclaw PRIVATE USE_PURE_WEBGL=1)
    
else (EMSCRIPTEN)
    # Non-WASM: Include existing SDL-dependent renderers
    target_sources(openclaw
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/IRenderer.h
        ${CMAKE_CURRENT_SOURCE_DIR}/GraphicsManager.h
        ${CMAKE_CURRENT_SOURCE_DIR}/GraphicsManager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/GraphicsAdapter.h
        ${CMAKE_CURRENT_SOURCE_DIR}/GraphicsAdapter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Data/MenuBackgroundData.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Data/MenuItemData.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Data/MenuTextData.h
        ${CMAKE_CURRENT_SOURCE_DIR}/WebGL/WebGLRenderer.h
        ${CMAKE_CURRENT_SOURCE_DIR}/WebGL/WebGLRenderer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/WebGPU/WebGPURenderer.h
        ${CMAKE_CURRENT_SOURCE_DIR}/WebGPU/WebGPURenderer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/GraphicsTest.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/IntegrationTest.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/WebGPUStatusTest.cpp
    )
endif (EMSCRIPTEN)
