################################################################################
# External dependencies
################################################################################

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

################################################################################
# CPM Package manager
################################################################################

set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM.cmake")
if(NOT (EXISTS ${CPM_DOWNLOAD_LOCATION}))
    file(DOWNLOAD https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.38.3/CPM.cmake ${CPM_DOWNLOAD_LOCATION}
        EXPECTED_HASH SHA256=cc155ce02e7945e7b8967ddfaff0b050e958a723ef7aad3766d368940cb15494)
endif()
include(${CPM_DOWNLOAD_LOCATION})

################################################################################
# Core dependencies
################################################################################

# icu4c
CPMAddPackage(
    NAME icu
    GITHUB_REPOSITORY GameDevDeps/icu4c
    GIT_TAG icu/release-73-2
)
if(${icu_ADDED})
    find_package(Python3 COMPONENTS Interpreter)
    if(NOT Python3_Interpreter_FOUND)
        message(FATAL_ERROR "Python3 is required to build ICU data")
    endif()

    icu_generate_data(TARGET lstg.IcuDataBuild
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
        DEPENDS lstg.IcuDataBuild
        COMMENT "Converting icu data"
        VERBATIM)
    add_library(lstg.IcuData STATIC "${CMAKE_CURRENT_BINARY_DIR}/icudata/${ICU_DATA_FILENAME}.cpp")
    add_library(lstg::IcuData ALIAS lstg.IcuData)
endif()

# libfmt
CPMAddPackage(
    NAME fmt
    GITHUB_REPOSITORY GameDevDeps/fmt
    GIT_TAG fmt/10.2.1
)

# spdlog
CPMAddPackage(
    NAME spdlog
    GITHUB_REPOSITORY GameDevDeps/spdlog
    GIT_TAG spdlog/v1.13.0
)

# zlib-ng
CPMAddPackage(
    NAME zlib
    GITHUB_REPOSITORY GameDevDeps/zlib-ng
    GIT_TAG zlib-ng/2.1.5
)

# glm
# TODO: enable GLM_ENABLE_SIMD_SSE3 as default when targeting x86/64
CPMAddPackage(
    NAME glm
    GITHUB_REPOSITORY GameDevDeps/glm
    GIT_TAG glm/1.0.0
    OPTIONS
        "GLM_DISABLE_AUTO_DETECTION ON"
        "GLM_FORCE_DEPTH_ZERO_TO_ONE ON"
        "GLM_FORCE_LEFT_HANDED ON"
)

# nlohmann/json
CPMAddPackage(
    NAME nlohmann_json
    GITHUB_REPOSITORY GameDevDeps/nlohmann_json
    GIT_TAG json/v3.11.3
)

# ryu
# TODO: switch to Google/double-conversion
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

lstg_group_deps_into_ide_folder(FOLDER "deps/Core"
    TARGETS
        icu.prepare_genbrk
        icu.prepare_gencfu
        icu.prepare_gencnval
        icu.prepare_gendict
        icu.prepare_genrb
        icu.prepare_gensprep
        icu.prepare_icupkg
        icu.prepare_makeconv
        icu.prepare_pkgdata
        icu.libsicuuc
        icu.libsicuin
        icu.libsicuio
        icu.libsicutu
        icu.libsicudata
        icu.genbrk
        icu.gencfu
        icu.gencnval
        icu.gendict
        icu.genrb
        icu.gensprep
        icu.icupkg
        icu.makeconv
        icu.pkgdata
        fmt.fmt
        spdlog.spdlog
        zlibng.zlib
        glm.glm
        nlohmann_json.nlohmann_json
        brotli.brotli
        brotli.brotlicommon
        brotli.brotlidec
        brotli.brotlienc
        bzip2.bz2
        bzip2.bzip2
        bzip2.bzip2recover
        ryu
)

################################################################################
# Platform dependencies
################################################################################

# SDL
CPMAddPackage(
    NAME sdl
    GITHUB_REPOSITORY GameDevDeps/SDL
    GIT_TAG SDL/release-2.30.0
    OPTIONS
        "SDL_ATOMIC OFF"
        "SDL_RENDER OFF"
        "SDL_POWER OFF"
        "SDL_SENSOR OFF"
        "SDL_LOCALE OFF"
        "SDL_MISC OFF"
        "SDL_TEST OFF"
)

# DiligentCore
CPMAddPackage(
    NAME DiligentCore
    GITHUB_REPOSITORY DiligentGraphics/DiligentCore
    VERSION 2.5.4
    OPTIONS
        "DILIGENT_CLANG_COMPILE_OPTIONS "
)

lstg_group_deps_into_ide_folder(FOLDER "deps/Platform"
    TARGETS
        sdl2.SDL2-static
        sdl2.SDL2main
        sdl2.sdl_headers_copy
)

################################################################################
# Graphics dependencies
################################################################################

# imgui
CPMAddPackage(
    NAME imgui
    GITHUB_REPOSITORY GameDevDeps/imgui
    GIT_TAG imgui/v1.90.2
)

# implot
CPMAddPackage(
    NAME implot
    GITHUB_REPOSITORY GameDevDeps/implot
    GIT_TAG implot/v0.16
)

# stb
CPMAddPackage(
    NAME stb
    GITHUB_REPOSITORY GameDevDeps/stb
    GIT_TAG stb/b7cf124
)

# freetype
CPMAddPackage(
    NAME freetype
    GITHUB_REPOSITORY GameDevDeps/freetype
    GIT_TAG freetype/v2.13.2
)

# harfbuzz
CPMAddPackage(
    NAME harfbuzz
    GITHUB_REPOSITORY GameDevDeps/harfbuzz
    GIT_TAG harfbuzz/8.3.0
)

lstg_group_deps_into_ide_folder(FOLDER "deps/Graphics"
    TARGETS
        imgui.imgui
        implot.implot
        stb.stb
        freetype.freetype
        harfbuzz.harfbuzz
        harfbuzz.harfbuzz-icu
        harfbuzz.harfbuzz-subset
        libpng.png_genfiles
        libpng.png_genprebuilt
        libpng.png_gensym
        libpng.png_genvers
        libpng.png_scripts_intprefix_out
        libpng.png_scripts_pnglibconf_c
        libpng.png_scripts_prefix_out
        libpng.png_scripts_sym_out
        libpng.png_scripts_symbols_chk
        libpng.png_scripts_symbols_out
        libpng.png_scripts_vers_out
        libpng.png_static
        libpng.pnglibconf_c
        libpng.pnglibconf_h
        libpng.pnglibconf_out
        libpng.pngprefix_h
)

################################################################################
# Audio dependencies
################################################################################

# mojoAL
if(NOT LSTG_PLATFORM_EMSCRIPTEN)
    CPMAddPackage(
        NAME mojoal
        GITHUB_REPOSITORY GameDevDeps/mojoAL
        GIT_TAG mojoAL/1adfdf5
    )
endif()

# SDL_sound
CPMAddPackage(
    NAME sdl_sound
    GITHUB_REPOSITORY GameDevDeps/SDL_sound
    GIT_TAG SDL_sound/v2.0.2
)

lstg_group_deps_into_ide_folder(FOLDER "deps/Audio"
    TARGETS
        mojoal.mojoal
        sdl_sound.SDL2_sound-static
)

################################################################################
# Scripting dependencies
################################################################################

# lua or luajit
if(LSTG_PLATFORM_EMSCRIPTEN)
    CPMAddPackage(
        NAME lua51
        GITHUB_REPOSITORY GameDevDeps/lua51
        GIT_TAG lua/5.1.5
    )

    CPMAddPackage(
        NAME luabitop
        GITHUB_REPOSITORY GameDevDeps/luabitop
        GIT_TAG luabitop/1.0.2
    )

    add_library(lstg::liblua-static ALIAS lua51.liblua-static)
else()
    CPMAddPackage(
        NAME luajit
        GITHUB_REPOSITORY GameDevDeps/luajit
        GIT_TAG luajit2/v2.1-20231117
        OPTIONS
            "LUAJIT_DISABLE_FFI ON"
            "LUAJIT_DISABLE_BUFFER ON"
    )

    add_library(lstg::liblua-static ALIAS luajit.liblua-static)
endif()

# lua-cjson
CPMAddPackage(
    NAME lua-cjson
    GITHUB_REPOSITORY GameDevDeps/lua-cjson
    GIT_TAG lua-cjson/2.1.0.13
    OPTIONS
        "LUA_LIBRARY_TARGET lstg::liblua-static"
        "ENABLE_CJSON_GLOBAL ON"
)

lstg_group_deps_into_ide_folder(FOLDER "deps/Scripting"
    TARGETS
        lua51.liblua
        lua51.liblua-static
        lua51.lua
        lua51.wlua
        lua51.luac
        luabitop.luabitop-static
        luajit.liblua
        luajit.liblua-static
        luajit.lua
        luajit.lua-static
        luajit.buildvm
        luajit.minilua
        luacjson.cjson
)
