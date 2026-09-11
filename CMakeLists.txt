cmake_minimum_required(VERSION 3.16)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)

project(raylibgame CXX)

include(FetchContent)

# --- Fetch Dependencies ---
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib
    GIT_TAG        6.0
)
set(BUILD_EXAMPLES OFF CACHE INTERNAL "")

FetchContent_Declare(
    steamworks
    GIT_REPOSITORY https://github.com/rlabrecque/SteamworksSDK
    GIT_TAG        df2baabf574a738ef1ea90a7e89339107fc0a279
)

# Added nlohmann/json dependency
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json
    GIT_TAG v3.12.0
)

FetchContent_MakeAvailable(raylib steamworks json)

# --- Source Files ---
file(GLOB_RECURSE SRC_FILES
    "src/*.cpp"
    "src/*.c"
    "src/*.hpp"
    "src/*.h"
)

add_executable(${PROJECT_NAME} ${SRC_FILES})

# --- Include Directories ---
target_include_directories(${PROJECT_NAME} PRIVATE 
    "${steamworks_SOURCE_DIR}/public"
)

# --- Platform Specific Linking & Post-Build Copy ---
if(WIN32)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(STEAM_LIB_PATH "${steamworks_SOURCE_DIR}/redistributable_bin/win64/steam_api64.lib")
        set(STEAM_DLL_PATH "${steamworks_SOURCE_DIR}/redistributable_bin/win64/steam_api64.dll")
    else()
        set(STEAM_LIB_PATH "${steamworks_SOURCE_DIR}/redistributable_bin/steam_api.lib")
        set(STEAM_DLL_PATH "${steamworks_SOURCE_DIR}/redistributable_bin/steam_api.dll")
    endif()

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${STEAM_DLL_PATH}"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    )
elseif(APPLE)
    set(STEAM_LIB_PATH "${steamworks_SOURCE_DIR}/redistributable_bin/osx/libsteam_api.dylib")

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${STEAM_LIB_PATH}"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    )

    set_target_properties(${PROJECT_NAME} PROPERTIES
        BUILD_RPATH "@executable_path"
        INSTALL_RPATH "@executable_path"
    )
elseif(UNIX)
    set(STEAM_LIB_PATH "${steamworks_SOURCE_DIR}/redistributable_bin/linux64/libsteam_api.so")

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${STEAM_LIB_PATH}"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    )

    set_target_properties(${PROJECT_NAME} PROPERTIES
        BUILD_RPATH "$ORIGIN"
        INSTALL_RPATH "$ORIGIN"
    )
endif()

# --- Link Executable Libraries ---
# Note: Added nlohmann_json::nlohmann_json to the link list
target_link_libraries(${PROJECT_NAME} PRIVATE 
    raylib
    nlohmann_json::nlohmann_json
    "${STEAM_LIB_PATH}"
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)
