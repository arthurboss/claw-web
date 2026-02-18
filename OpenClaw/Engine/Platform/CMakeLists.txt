cmake_minimum_required(VERSION 4.1.0)

target_sources(openclaw
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Events/AppEvent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Events/AppEventQueue.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Events/AppEventQueue.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Video/VideoPlatform.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Video/Null/NullVideoPlatform.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Video/WASM/WasmVideoPlatform.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Video/WASM/WasmVideoPlatform.cpp
)
