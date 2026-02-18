# Graphics Engine CMakeLists.txt

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Common graphics data structures (used by all platforms)
target_sources(openclaw PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Data/MenuBackgroundData.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Data/MenuItemData.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Data/MenuTextData.h
    ${CMAKE_CURRENT_SOURCE_DIR}/GraphicsManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/GraphicsManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/GraphicsAdapter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/GraphicsAdapter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IRenderer.h
)

# Generic graphics module (cross-platform interface)
target_sources(openclaw PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Generic/IGenericRenderer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Generic/GenericGraphicsModule.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Generic/GenericGraphicsModule.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Generic/ParticleEmitter.h
)

# Platform-specific graphics implementations
if (EMSCRIPTEN)
           # WASM builds: Use pure WebGL/WebGPU renderers (no SDL dependencies)
       target_sources(openclaw PRIVATE
           ${CMAKE_CURRENT_SOURCE_DIR}/WASM/PureWebGLRenderer.h
           ${CMAKE_CURRENT_SOURCE_DIR}/WASM/PureWebGLRenderer.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/WASM/PureWebGPURenderer.h
           ${CMAKE_CURRENT_SOURCE_DIR}/WASM/PureWebGPURenderer.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/WASM/TextureManager.h
           ${CMAKE_CURRENT_SOURCE_DIR}/WASM/TextureManager.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/WASM/Sprite.h
           ${CMAKE_CURRENT_SOURCE_DIR}/WASM/Sprite.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/WASM/SpriteRenderer.h
           ${CMAKE_CURRENT_SOURCE_DIR}/WASM/SpriteRenderer.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/WASM/PostProcessor.h
           ${CMAKE_CURRENT_SOURCE_DIR}/WASM/PostProcessor.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/WASM/ParticleSystem.h
           ${CMAKE_CURRENT_SOURCE_DIR}/WASM/ParticleSystem.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/WASM/PostProcessShader.h
           ${CMAKE_CURRENT_SOURCE_DIR}/WASM/PostProcessShader.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/WASM/PostProcessFramebuffer.cpp
            # Generic module WASM renderers
            ${CMAKE_CURRENT_SOURCE_DIR}/Generic/WASM/PureWebGLRenderer.h
            ${CMAKE_CURRENT_SOURCE_DIR}/Generic/WASM/PureWebGLRenderer.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/Generic/WASM/PureWebGPURenderer.h
            ${CMAKE_CURRENT_SOURCE_DIR}/Generic/WASM/PureWebGPURenderer.cpp
        )
else()
    # Non-WASM builds: Use existing SDL-based renderers (unchanged)
    target_sources(openclaw PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/WebGL/WebGLRenderer.h
        ${CMAKE_CURRENT_SOURCE_DIR}/WebGL/WebGLRenderer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/WebGPU/WebGPURenderer.h
        ${CMAKE_CURRENT_SOURCE_DIR}/WebGPU/WebGPURenderer.cpp
    )
endif()
