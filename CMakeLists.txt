cmake_minimum_required(VERSION 3.15)
project(rpg_engine VERSION 0.1.0)

# C++17 is required for pybind11 and good practices
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# 1. Fetch Raylib
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.0
)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
set(CUSTOMIZE_BUILD ON CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(CMAKE_POSITION_INDEPENDENT_CODE ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(raylib)

# 2. Fetch Pybind11
FetchContent_Declare(
    pybind11
    GIT_REPOSITORY https://github.com/pybind/pybind11.git
    GIT_TAG v2.11.1
)
FetchContent_MakeAvailable(pybind11)

# 3. Fetch EnTT
FetchContent_Declare(
    entt
    GIT_REPOSITORY https://github.com/skypjack/entt.git
    GIT_TAG v3.13.0
)
FetchContent_MakeAvailable(entt)

# 4. Fetch nlohmann/json
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

# 5. Create the Python module
set(SOURCES
    engine/bindings/bindings.cpp
    engine/core/engine.cpp
    engine/graphics/texture_manager.cpp
    engine/graphics/camera.cpp
    engine/ecs/scene.cpp
    engine/assets/asset_manager.cpp
)
pybind11_add_module(engine ${SOURCES})

# Link Raylib, EnTT, and json to the Python module
target_link_libraries(engine PRIVATE raylib EnTT::EnTT nlohmann_json::nlohmann_json)

# Include directories
target_include_directories(engine PRIVATE engine)

# Ensure the output goes to the game directory so python can find it
set_target_properties(engine PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/game
    LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/game
    LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/game
)
