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

# icu4c
CPMAddPackage(
    NAME icu
    GITHUB_REPOSITORY GameDevDeps/icu4c
    GIT_TAG icu-release_73_2
)
if(${icu_ADDED})
    find_package(Python3 COMPONENTS Interpreter)
    if(NOT Python3_Interpreter_FOUND)
        message(FATAL_ERROR "Python3 is required to build data")
    endif()

    icu_generate_data(TARGET icu_data
        WORKING_DIR "${CMAKE_CURRENT_BINARY_DIR}/icudata/tmp"
        OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/icudata"
        FILENAME_OUT ICU_DATA_FILENAME
        FILTER "brkitr/*")

    add_custom_command(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/icudata/${ICU_DATA_FILENAME}.cpp"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/icudata/"
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tool/BinaryToCode/BinaryToCode.py
            -i "${CMAKE_CURRENT_BINARY_DIR}/icudata/${ICU_DATA_FILENAME}"
            -n "kIcuDataContent"
            -o "${CMAKE_CURRENT_BINARY_DIR}/icudata/${ICU_DATA_FILENAME}.cpp"
        DEPENDS icu_data
        COMMENT "Converting icu data"
        VERBATIM)
    add_library(IcuData STATIC "${CMAKE_CURRENT_BINARY_DIR}/icudata/${ICU_DATA_FILENAME}.cpp")
endif()

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
        "SPDLOG_WCHAR_FILENAMES ON"
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
    NAME sdl2
    GITHUB_REPOSITORY libsdl-org/SDL
    GIT_TAG release-2.28.2
    # GIT_TAG main
    PATCH_COMMAND git restore cmake/sdlchecks.cmake src/core/android/SDL_android.c
    COMMAND git apply --ignore-whitespace ${CMAKE_CURRENT_SOURCE_DIR}/patch/sdl2-sdlchecks-patch.patch
    COMMAND git apply --ignore-whitespace ${CMAKE_CURRENT_SOURCE_DIR}/patch/sdl_android-patch.patch
    OPTIONS
        "SDL2_DISABLE_UNINSTALL ON"
        "SDL_ATOMIC OFF"
        "SDL_RENDER OFF"
        "SDL_POWER OFF"
        "SDL_SENSOR OFF"
        "SDL_LOCALE OFF"
        "SDL_MISC OFF"
        "SDL_TEST OFF"
)

# lua or luajit
if(LSTG_PLATFORM_EMSCRIPTEN)
    CPMAddPackage(
        NAME lua
        GITHUB_REPOSITORY 9chu/lua
        GIT_TAG lua-5.1-emscripten
    )
    if(${lua_ADDED})
        add_library(liblua-static ALIAS liblua_static)  # 与 luajit 保持相同
    endif()

    CPMAddPackage(
        NAME luabitop
        GITHUB_REPOSITORY LuaDist/luabitop
        GIT_TAG master
        DOWNLOAD_ONLY ON
    )
    if(${luabitop_ADDED})
        add_library(luabitop STATIC ${luabitop_SOURCE_DIR}/bit.c)
        target_link_libraries(luabitop PUBLIC liblua-static)
    endif()
else()
    CPMAddPackage(
        NAME luajit
        GITHUB_REPOSITORY 9chu/LuaJIT-cmake
        GIT_TAG master
        OPTIONS
            "LUAJIT_DISABLE_FFI ON"
            "LUAJIT_DISABLE_BUFFER ON"
    )
endif()
get_target_property(LUA_INCLUDE_DIR liblua-static INCLUDE_DIRECTORIES)
get_target_property(LUA_INCLUDE_DIR_INTERFACE liblua-static INTERFACE_INCLUDE_DIRECTORIES)
list(APPEND LUA_INCLUDE_DIR ${LUA_INCLUDE_DIR_INTERFACE})
get_target_property(LUA_BUILD_DIR liblua-static BINARY_DIR)
list(JOIN LUA_INCLUDE_DIR "\\\\;" LUA_INCLUDE_DIR_ESCAPED)

# imgui
CPMAddPackage(
    NAME imgui
    GITHUB_REPOSITORY ocornut/imgui
    VERSION 1.87
    DOWNLOAD_ONLY ON
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
    DOWNLOAD_ONLY ON
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
    VERSION 2.5.2
    PATCH_COMMAND git restore Graphics/HLSL2GLSLConverterLib/src/HLSL2GLSLConverterImpl.cpp
    COMMAND git restore Graphics/GraphicsEngineVulkan/src/VulkanUtilities/VulkanInstance.cpp
    COMMAND git apply --ignore-whitespace ${CMAKE_CURRENT_SOURCE_DIR}/patch/diligent-std-move-patch.patch
    COMMAND git apply --ignore-whitespace ${CMAKE_CURRENT_SOURCE_DIR}/patch/diligent-vk-device-select-patch.patch
)

# glm
CPMAddPackage(
    NAME glm
    GITHUB_REPOSITORY g-truc/glm
    GIT_TAG 0.9.9.8
)
if(${glm_ADDED})
    # 我们使用 DX 左手系，深度范围 [0, 1]，这里需要额外给 glm 设置编译选项
    target_compile_definitions(glm INTERFACE GLM_FORCE_DEPTH_ZERO_TO_ONE=1 GLM_FORCE_LEFT_HANDED=1)
endif()

# stb
CPMAddPackage(
    NAME stb
    GITHUB_REPOSITORY nothings/stb
    GIT_TAG master
    DOWNLOAD_ONLY ON
)
if(${stb_ADDED})
    file(GLOB stb_SOURCES ${stb_SOURCE_DIR}/*.c)
    add_library(stb STATIC ${stb_SOURCES})
    target_include_directories(stb PUBLIC ${stb_SOURCE_DIR})
endif()

# lua-cjson
CPMAddPackage(
    NAME lua-cjson
    GITHUB_REPOSITORY 9chu/lua-cjson
    GIT_TAG patch-static-link
    OPTIONS
        "STATIC_LINK ON"
        "LUA_INCLUDE_DIR ${LUA_INCLUDE_DIR_ESCAPED}"
        "LUA_LIBRARY ${LUA_BUILD_DIR}"
        "ENABLE_CJSON_GLOBAL ON"
)

# nlohmann/json
CPMAddPackage(
    NAME nlohmann_json
    GITHUB_REPOSITORY nlohmann/json
    VERSION 3.10.5
)

# freetype
CPMAddPackage(
    NAME freetype
    GITHUB_REPOSITORY freetype/freetype
    GIT_TAG VER-2-12-1
    OPTIONS
        "CMAKE_DISABLE_FIND_PACKAGE_HarfBuzz TRUE"
)

# harfbuzz
CPMAddPackage(
    NAME harfbuzz
    GITHUB_REPOSITORY 9chu/harfbuzz
    GIT_TAG patch-cmake  # 4.3.0
    OPTIONS
        "SKIP_INSTALL_ALL ON"
        "HB_BUILD_SUBSET OFF"
)
if(${harfbuzz_ADDED})
    # 手动增加 harfbuzz-icu 目标
    add_library(harfbuzz-icu ${harfbuzz_SOURCE_DIR}/src/hb-icu.cc ${harfbuzz_SOURCE_DIR}/src/hb-icu.h)
    add_dependencies(harfbuzz-icu harfbuzz)
    target_link_libraries(harfbuzz-icu harfbuzz icu_libsicuuc)
    target_compile_definitions(harfbuzz-icu PUBLIC -DHAVE_ICU -DHAVE_ICU_BUILTIN -DHB_NO_UCD -DHB_NO_DRAW)
endif()

# ryu
CPMAddPackage(
    NAME ryu
    GITHUB_REPOSITORY ulfjack/ryu
    GIT_TAG master
    DOWNLOAD_ONLY ON
)
if(${ryu_ADDED})
    # file(GLOB ryu_SOURCES ${ryu_SOURCE_DIR}/ryu/*.c)
    add_library(ryu STATIC ${ryu_SOURCE_DIR}/ryu/d2fixed.c ${ryu_SOURCE_DIR}/ryu/d2s.c ${ryu_SOURCE_DIR}/ryu/f2s.c
        ${ryu_SOURCE_DIR}/ryu/s2d.c ${ryu_SOURCE_DIR}/ryu/s2f.c)
    target_include_directories(ryu PUBLIC ${ryu_SOURCE_DIR})
endif()

# mojoAL
if(NOT LSTG_PLATFORM_EMSCRIPTEN)
    CPMAddPackage(
        NAME mojoal
        GITHUB_REPOSITORY icculus/mojoAL
        GIT_TAG main
        DOWNLOAD_ONLY ON
    )
    if(${mojoal_ADDED})
        add_library(mojoal STATIC ${mojoal_SOURCE_DIR}/mojoal.c)
        target_include_directories(mojoal PUBLIC ${mojoal_SOURCE_DIR}/AL)
        target_link_libraries(mojoal PRIVATE SDL2-static)
        target_compile_definitions(mojoal PUBLIC AL_LIBTYPE_STATIC)
    endif()
endif()

# SDL_sound
CPMAddPackage(
    NAME sdl_sound
    GITHUB_REPOSITORY icculus/SDL_sound
    VERSION 2.0.1
    PATCH_COMMAND git restore CMakeLists.txt src/SDL_sound.c
    COMMAND git apply --ignore-whitespace ${CMAKE_CURRENT_SOURCE_DIR}/patch/sdl_sound-patch.patch
    OPTIONS
        "SDLSOUND_BUILD_TEST OFF"
        "SDLSOUND_BUILD_SHARED OFF"
)
if(${sdl_sound_ADDED})
    target_include_directories(SDL2_sound-static PUBLIC ${sdl_sound_SOURCE_DIR}/src)
    target_link_libraries(SDL2_sound-static SDL2-static)
endif()

### 优化 IDE 展示

# 将第三方依赖合并到指定文件夹，优化 IDE 中展示
function(lstg_group_deps_into_ide_folder)
    set(ONE_VALUE_ARGS FOLDER)
    set(MULTI_VALUE_ARGS TARGETS)
    cmake_parse_arguments(GROUP_DEPS "" "${ONE_VALUE_ARGS}" "${MULTI_VALUE_ARGS}" ${ARGN})

    foreach(GROUP_DEPS_TARGET ${GROUP_DEPS_TARGETS})
        if(TARGET ${GROUP_DEPS_TARGET})
            set_target_properties(${GROUP_DEPS_TARGET} PROPERTIES FOLDER ${GROUP_DEPS_FOLDER})
        endif()
    endforeach()
endfunction()

# FIXME: 优化下列三方依赖分类

lstg_group_deps_into_ide_folder(FOLDER "deps/SDL2"
    TARGETS
        sdl_headers_copy
        SDL2
        SDL2main
        SDL2-static
        mojoal
        SDL2_sound-static
)

lstg_group_deps_into_ide_folder(FOLDER "deps/imgui"
    TARGETS
        imgui
        implot
)

if(LSTG_PLATFORM_EMSCRIPTEN)
    lstg_group_deps_into_ide_folder(FOLDER "deps/lua"
        TARGETS
            liblua_static
            luabitop
            cjson
    )
else()
    lstg_group_deps_into_ide_folder(FOLDER "deps/lua"
        TARGETS
            minilua
            buildvm
            liblua-shared
            liblua-static
            lua
            lua-static
            cjson
    )
endif()

lstg_group_deps_into_ide_folder(FOLDER "deps/compose"
    TARGETS
        freetype
        harfbuzz
        harfbuzz-icu
)

lstg_group_deps_into_ide_folder(FOLDER "deps/misc"
    TARGETS
        fmt
        ryu
        spdlog
        stb
        zlib-ng
)

lstg_group_deps_into_ide_folder(FOLDER "deps/icu"
    TARGETS
        icu_libsicuuc
        icu_libsicuin
        icu_libsicuio
        icu_libsicutu
        IcuData
        icu_gencnval
        icu_gencfu
        icu_makeconv
        icu_genbrk
        icu_gensprep
        icu_gendict
        icu_icupkg
        icu_genrb
        icu_pkgdata
        icu_prepare_gencnval
        icu_prepare_gencfu
        icu_prepare_makeconv
        icu_prepare_genbrk
        icu_prepare_gensprep
        icu_prepare_gendict
        icu_prepare_icupkg
        icu_prepare_genrb
        icu_prepare_pkgdata
)
