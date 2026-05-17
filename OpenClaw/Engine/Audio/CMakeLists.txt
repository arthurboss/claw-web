cmake_minimum_required(VERSION 4.3.2)

target_sources(openclaw
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Audio.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Audio.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IAudioSystem.h
    ${CMAKE_CURRENT_SOURCE_DIR}/AudioSystemFactory.h
    ${CMAKE_CURRENT_SOURCE_DIR}/AudioSystemFactory.cpp
)

if (Emscripten)
    target_sources(openclaw
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/WASM/AudioWorkletSystem.h
        ${CMAKE_CURRENT_SOURCE_DIR}/WASM/AudioWorkletSystem.cpp
    )
endif (Emscripten)

if (WIN32)
    target_sources(openclaw PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/midiproc_c.c)
endif (WIN32)