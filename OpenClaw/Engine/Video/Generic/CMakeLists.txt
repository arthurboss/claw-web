cmake_minimum_required(VERSION 4.3.2)

target_sources(openclaw
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/GenericVideoModule.h
    ${CMAKE_CURRENT_SOURCE_DIR}/GenericVideoModule.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IGenericVideo.h
)

if (Emscripten)
    target_sources(openclaw
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/WASM/WasmVideoPlayer.h
        ${CMAKE_CURRENT_SOURCE_DIR}/WASM/WasmVideoPlayer.cpp
    )
endif (Emscripten)
