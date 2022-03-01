cmake_minimum_required(VERSION 3.16)
project(LuaSTGPlus)

## 工程全局选项
# 请参照帮助定义下述选项

option(LSTG_SHIPPING "Build shipping mode binary" ON)
set(LSTG_APP_NAME "default" CACHE STRING "Specific the app name, will be used as the folder name in AppData for user data storage")

## CPM 包管理器

set(CPM_DOWNLOAD_VERSION 0.34.3)
set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")

if(NOT (EXISTS ${CPM_DOWNLOAD_LOCATION}))
    message(STATUS "Downloading CPM.cmake")
    file(DOWNLOAD
        https://github.com/TheLartians/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake
        ${CPM_DOWNLOAD_LOCATION})
endif()

include(${CPM_DOWNLOAD_LOCATION})

## 第三方依赖

set(CMAKE_CXX_STANDARD 17)

CPMAddPackage(
    NAME fmt
    GITHUB_REPOSITORY fmtlib/fmt
    GIT_TAG 8.1.1
)

CPMAddPackage(
    NAME spdlog
    GITHUB_REPOSITORY gabime/spdlog
    VERSION 1.9.2
    OPTIONS
        "SPDLOG_FMT_EXTERNAL ON"
)

if(EMSCRIPTEN)
    # FIXME: 该 PR 尚未合入上游，在 zlib-ng 分支稳定后切换
    CPMAddPackage(
        NAME zlib-ng
        GITHUB_REPOSITORY PiotrSikora/zlib-ng
        GIT_TAG wasm32
        OPTIONS
            "BUILD_SHARED_LIBS OFF"
            "CMAKE_C_COMPILER_TARGET wasm32"
    )
else()
    CPMAddPackage(
        NAME zlib-ng
        GITHUB_REPOSITORY zlib-ng/zlib-ng
        VERSION 2.0.0-RC2
    )
endif()

CPMAddPackage(
    NAME SDL
    GITHUB_REPOSITORY libsdl-org/SDL
    #GIT_TAG release-2.0.20
    GIT_TAG main
    OPTIONS
        "SDL2_DISABLE_UNINSTALL ON"
        "SDL_ATOMIC OFF"
        "SDL_AUDIO OFF"
        "SDL_RENDER OFF"
        "SDL_HAPTIC OFF"
        "SDL_HIDAPI OFF"
        "SDL_POWER OFF"
        "SDL_CPUINFO OFF"
        "SDL_SENSOR OFF"
        "SDL_LOCALE OFF"
        "SDL_MISC OFF"
)

CPMAddPackage(
    NAME bgfx
    GITHUB_REPOSITORY bkaradzic/bgfx.cmake
    VERSION 1.115.8065-9426a2f
    OPTIONS
        "BGFX_BUILD_EXAMPLES OFF"
        "BGFX_INSTALL OFF"
)

CPMAddPackage(
    NAME imgui
    GITHUB_REPOSITORY ocornut/imgui
    VERSION 1.87
    DOWNLOAD_ONLY
)
if(${imgui_ADDED})
    file(GLOB imgui_SOURCES ${imgui_SOURCE_DIR}/*.cpp)
    add_library(imgui STATIC ${imgui_SOURCES})
    target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})
endif()

## 工程配置

# Shipping/Development 开关
if(LSTG_SHIPPING)
    add_compile_definitions(LSTG_SHIPPING=1)
else()
    add_compile_definitions(LSTG_DEVELOPMENT=1)
endif()

add_compile_definitions(LSTG_APP_NAME="${LSTG_APP_NAME}")

# 全局编译选项
if(MSVC)
    add_compile_definitions(_WIN32_WINNT=0x0600 _GNU_SOURCE _CRT_SECURE_NO_WARNINGS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /utf-8")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wextra -Wno-implicit-fallthrough -Wno-unused-parameter -Wno-noexcept-type")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Wextra -Wno-implicit-fallthrough -Wno-unused-parameter -Wno-noexcept-type")
endif()

add_subdirectory(src/Core)
add_subdirectory(src/GameApp)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/DevApp/CMakeLists.txt")
    add_subdirectory(src/DevApp)
endif()
