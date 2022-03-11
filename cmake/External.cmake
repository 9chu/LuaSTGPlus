### 第三方依赖定义

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

# libfmt
CPMAddPackage(
    NAME fmt
    GITHUB_REPOSITORY fmtlib/fmt
    GIT_TAG 8.1.1
)

# spdlog
CPMAddPackage(
    NAME spdlog
    GITHUB_REPOSITORY gabime/spdlog
    VERSION 1.9.2
    OPTIONS
    "SPDLOG_FMT_EXTERNAL ON"
)

# zlib-ng
if(LSTG_PLATFORM_EMSCRIPTEN)
    CPMAddPackage(
        NAME zlib-ng
        GITHUB_REPOSITORY 9chu/zlib-ng
        GIT_TAG patch_name_merge_wasm32
        OPTIONS
        "ZLIB_ENABLE_TESTS OFF"
        "BUILD_SHARED_LIBS OFF"
        "CMAKE_C_COMPILER_TARGET wasm32"
        "ZLIB_COMPAT OFF"
    )
else()
    CPMAddPackage(
        NAME zlib-ng
        GITHUB_REPOSITORY 9chu/zlib-ng
        GIT_TAG patch_name_merge_wasm32
        OPTIONS
        "ZLIB_ENABLE_TESTS OFF"
        "BUILD_SHARED_LIBS OFF"
        "ZLIB_COMPAT OFF"
    )
endif()

# SDL
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

# lua or luajit
if(LSTG_PLATFORM_EMSCRIPTEN)
    CPMAddPackage(
        NAME lua
        GITHUB_REPOSITORY 9chu/lua
        GIT_TAG lua-5.1-emscripten
    )
    add_library(liblua-static ALIAS liblua_static)  # 与 luajit 保持相同
else()
    CPMAddPackage(
        NAME luajit
        GITHUB_REPOSITORY 9chu/LuaJIT-cmake
        GIT_TAG master
    )
endif()

# imgui
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

CPMAddPackage(
    NAME implot
    GITHUB_REPOSITORY epezent/implot
    VERSION 0.13
    DOWNLOAD_ONLY
)
if(${implot_ADDED})
    file(GLOB implot_SOURCES ${implot_SOURCE_DIR}/implot.cpp ${implot_SOURCE_DIR}/implot_items.cpp)
    add_library(implot STATIC ${implot_SOURCES})
    target_include_directories(implot PUBLIC ${implot_SOURCE_DIR})
    target_link_libraries(implot PUBLIC imgui)
endif()

# DiligentCore
CPMAddPackage(
    NAME DiligentCore
    GITHUB_REPOSITORY DiligentGraphics/DiligentCore
    VERSION 2.5.1
)
