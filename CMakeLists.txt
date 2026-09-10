cmake_minimum_required(VERSION 3.16)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)

project(raylibgame CXX)

include(FetchContent)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib
    GIT_TAG        6.0
)

FetchContent_Declare(
    steamworks
    GIT_REPOSITORY https://github.com/rlabrecque/SteamworksSDK
    GIT_TAG "df2baabf574a738ef1ea90a7e89339107fc0a279"
)

set(BUILD_EXAMPLES OFF CACHE INTERNAL "")

FetchContent_MakeAvailable(raylib)
FetchContent_MakeAvailable(steamworks)

file(GLOB_RECURSE SRC_FILES
    "src/*.cpp"
    "src/*.c"
    "src/*.hpp"
    "src/*.h"
)

add_executable(${PROJECT_NAME} ${SRC_FILES})

target_link_libraries(${PROJECT_NAME} PRIVATE raylib)

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)
