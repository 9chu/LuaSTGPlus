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

## 第三方依赖（Early build）

# libicu
CPMAddPackage(
    NAME icu
    GITHUB_REPOSITORY unicode-org/icu
    GIT_TAG release-71-1
    DOWNLOAD_ONLY ON
)
if(${icu_ADDED})
    # icu common 库
    file(GLOB icu_COMMON_SOURCES
        ${icu_SOURCE_DIR}/icu4c/source/common/*.cpp
        ${icu_SOURCE_DIR}/icu4c/source/stubdata/*.cpp)
    add_library(icuuc STATIC ${icu_COMMON_SOURCES})
    target_include_directories(icuuc PRIVATE
        ${icu_SOURCE_DIR}/icu4c/source/common
        ${icu_SOURCE_DIR}/icu4c/source/stubdata
        ${icu_SOURCE_DIR}/icu4c/source/common/unicode)
    target_include_directories(icuuc PUBLIC ${icu_SOURCE_DIR}/icu4c/source/common)
    set(icu_COMMON_PUBLIC_BUILD_FLAGS "-DU_STATIC_IMPLEMENTATION=1" "-DU_ENABLE_DYLOAD=0")
    set(icu_COMMON_PRIVATE_BUILD_FLAGS "-DU_COMMON_IMPLEMENTATION")
    if(WIN32)
        # set minimal version to Win7 to support LOCALE_ALLOW_NEUTRAL_NAMES
        list(APPEND icu_COMMON_PRIVATE_BUILD_FLAGS -DWINVER=0x0601 -D_WIN32_WINNT=0x0601)
    endif()
    target_compile_definitions(icuuc PUBLIC ${icu_COMMON_PUBLIC_BUILD_FLAGS} PRIVATE ${icu_COMMON_PRIVATE_BUILD_FLAGS})

    # icu i18n 库
    file(GLOB_RECURSE icu_i18n_SOURCES
        ${icu_SOURCE_DIR}/icu4c/source/i18n/*.cpp
        ${icu_SOURCE_DIR}/icu4c/source/i18n/*.cpp)
    add_library(icuin STATIC ${icu_i18n_SOURCES})
    target_link_libraries(icuin PUBLIC icuuc)
    target_include_directories(icuin PUBLIC ${icu_SOURCE_DIR}/icu4c/source/i18n)
    set(icu_i18n_PRIVATE_BUILD_FLAGS "-DU_ATTRIBUTE_DEPRECATED=" "-DU_I18N_IMPLEMENTATION")
    if(WIN32)
        # set minimal version to Win7 to support ResolveLocalName
        list(APPEND icu_i18n_PRIVATE_BUILD_FLAGS -DWINVER=0x0601 -D_WIN32_WINNT=0x0601)
    endif()
    target_compile_definitions(icuin PRIVATE ${icu_i18n_PRIVATE_BUILD_FLAGS})

    # icu io 库
    file(GLOB_RECURSE icu_io_SOURCES
        ${icu_SOURCE_DIR}/icu4c/source/io/*.cpp
        ${icu_SOURCE_DIR}/icu4c/source/io/*.cpp)
    add_library(icuio STATIC ${icu_io_SOURCES})
    target_link_libraries(icuio PUBLIC icuin)
    target_include_directories(icuio PUBLIC ${icu_SOURCE_DIR}/icu4c/source/io)
    target_compile_definitions(icuio PRIVATE "-DU_IO_IMPLEMENTATION")

    # icu tool utils 库
    file(GLOB icu_TOOL_UTILS_SOURCES
        ${icu_SOURCE_DIR}/icu4c/source/tools/toolutil/*.c
        ${icu_SOURCE_DIR}/icu4c/source/tools/toolutil/*.cpp)
    add_library(icutu STATIC ${icu_TOOL_UTILS_SOURCES})
    target_link_libraries(icutu PUBLIC icuin icuio)
    target_include_directories(icutu PUBLIC ${icu_SOURCE_DIR}/icu4c/source/tools/toolutil)
    target_compile_definitions(icutu PUBLIC "-DU_DISABLE_OBJ_CODE")
    target_compile_definitions(icutu PRIVATE "-DU_TOOLUTIL_IMPLEMENTATION")

    # 构建工具
    file(GLOB icu_TOOL_DIRS
        ${icu_SOURCE_DIR}/icu4c/source/tools/*)
    set(icu_TOOLS gencnval gencfu makeconv genbrk gensprep gendict icupkg genrb pkgdata)
    if(CMAKE_CROSSCOMPILING)
        # https://cmake.org/cmake/help/book/mastering-cmake/chapter/Cross%20Compiling%20With%20CMake.html
        find_package(IcuBuildTools)
    else()
        foreach(icu_TMP_FILENAME ${icu_TOOL_DIRS})
            if(IS_DIRECTORY ${icu_TMP_FILENAME})
                get_filename_component(icu_TMP_TOOL_NAME ${icu_TMP_FILENAME} NAME)
                if("${icu_TMP_TOOL_NAME}" IN_LIST icu_TOOLS)
                    file(GLOB icu_TMP_TOOL_SOURCES ${icu_TMP_FILENAME}/*.c ${icu_TMP_FILENAME}/*.cpp)
                    # 特殊处理 genrb
                    foreach(icu_TMP_TOOL_SRC ${icu_TMP_TOOL_SOURCES})
                        if("${icu_TMP_TOOL_SRC}" MATCHES ".*derb.cpp")
                            list(REMOVE_ITEM icu_TMP_TOOL_SOURCES "${icu_TMP_TOOL_SRC}")
                        endif()
                    endforeach()
                    add_executable(${icu_TMP_TOOL_NAME} ${icu_TMP_TOOL_SOURCES})
                    set(icu_TMP_TOOL_LINKS icutu)
                    target_link_libraries(${icu_TMP_TOOL_NAME} ${icu_TMP_TOOL_LINKS})
                    if(LSTG_PLATFORM_LINUX)
                        # 需要 Force link 到 pthread，否额 call_once 会抛出异常
                        target_link_options(${icu_TMP_TOOL_NAME} PRIVATE "SHELL:-Wl,--no-as-needed" "SHELL:-lpthread"
                            "SHELL:-Wl,--as-needed")
                    endif()
                    if(WIN32)
                        set_target_properties(${icu_TMP_TOOL_NAME} PROPERTIES TOOL_PLATFORM "WIN32")
                    else()
                        set_target_properties(${icu_TMP_TOOL_NAME} PROPERTIES TOOL_PLATFORM "UNIX")
                    endif()
                    set_property(TARGET ${icu_TMP_TOOL_NAME} APPEND PROPERTY EXPORT_PROPERTIES TOOL_PLATFORM)
                endif()
            endif()
        endforeach()
        export(TARGETS ${icu_TOOLS} FILE "${CMAKE_BINARY_DIR}/IcuBuildToolsConfig.cmake")
    endif()

    # 移动构建工具到目录
    set(icu_PREPARE_TOOLS)
    foreach(icu_TMP_FILENAME ${icu_TOOL_DIRS})
        if(IS_DIRECTORY ${icu_TMP_FILENAME})
            get_filename_component(icu_TMP_TOOL_NAME ${icu_TMP_FILENAME} NAME)
            if("${icu_TMP_TOOL_NAME}" IN_LIST icu_TOOLS)
                get_target_property(icu_TMP_TOOL_PLATFORM ${icu_TMP_TOOL_NAME} TOOL_PLATFORM)

                # 构建后移动到固定目录
                if(icu_TMP_TOOL_PLATFORM STREQUAL "WIN32")
                    set(icu_DATA_GEN_MODE "windows-exec")  # 工具的平台应该是一样的
                    add_custom_target(PrepareTool_${icu_TMP_TOOL_NAME} COMMAND
                        "${CMAKE_COMMAND}" -E copy_if_different
                        "$<TARGET_FILE:${icu_TMP_TOOL_NAME}>"
                        "${CMAKE_BINARY_DIR}/icutools/${icu_TMP_TOOL_NAME}/${icu_TMP_TOOL_NAME}.exe")
                else()
                    set(icu_DATA_GEN_MODE "unix-exec")
                    add_custom_target(PrepareTool_${icu_TMP_TOOL_NAME} COMMAND
                        "${CMAKE_COMMAND}" -E copy_if_different
                        "$<TARGET_FILE:${icu_TMP_TOOL_NAME}>"
                        "${CMAKE_BINARY_DIR}/icutools/${icu_TMP_TOOL_NAME}")
                endif()
                list(APPEND icu_PREPARE_TOOLS PrepareTool_${icu_TMP_TOOL_NAME})
            endif()
        endif()
    endforeach()

    # 构建数据
    find_package(Python3 COMPONENTS Interpreter)
    if(NOT Python3_Interpreter_FOUND)
        message(FATAL "Python3 is required to build this project")
    endif()

    file(READ "${icu_SOURCE_DIR}/icu4c/source/common/unicode/uvernum.h" icu_VER_NUM_FILE_CONTENT)
    string(REGEX MATCH "U_ICU_VERSION_MAJOR_NUM ([0-9]*)" _ ${icu_VER_NUM_FILE_CONTENT})
    set(icu_VERSION_MAJOR ${CMAKE_MATCH_1})

    set(icu_DATA_NAME "icudt${icu_VERSION_MAJOR}")
    if("${CMAKE_CXX_BYTE_ORDER}" STREQUAL "BIG_ENDIAN")
        set(icu_DATA_ENDIAN_SUFFIX "b")
    else()
        set(icu_DATA_ENDIAN_SUFFIX "l")
    endif()
    set(icu_DATA_NAME_FULL "${icu_DATA_NAME}${icu_DATA_ENDIAN_SUFFIX}")

    set(icu_DATA_SOURCE_DIR ${icu_SOURCE_DIR}/icu4c/source/data)

    # 这里，我们只引入 brkitr 数据，如果有其他需要再进行追加
    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/icudata/")
    set(icu_DATA_OUTPUT "${CMAKE_BINARY_DIR}/icudata/icudata.cpp")
    add_custom_command(
        OUTPUT "${icu_DATA_OUTPUT}"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/icudata/"
        COMMAND ${CMAKE_COMMAND} -E env "PYTHONPATH=${icu_SOURCE_DIR}/icu4c/source/python" ${Python3_EXECUTABLE} -B -m icutools.databuilder
            --mode ${icu_DATA_GEN_MODE}
            --src_dir "${icu_DATA_SOURCE_DIR}"
            --tool_dir "${CMAKE_BINARY_DIR}/icutools"
            --tool_cfg ""
            --out_dir "${CMAKE_BINARY_DIR}/icudata/${icu_DATA_NAME_FULL}"
            --tmp_dir "${CMAKE_BINARY_DIR}/icudata/tmp"
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tool/GenerateFileList.py
            -o ${CMAKE_BINARY_DIR}/icudata/pkg_file_list.txt
            -s "${CMAKE_BINARY_DIR}/icudata/${icu_DATA_NAME_FULL}"
            "brkitr/*"
        COMMAND $<TARGET_FILE:pkgdata>
            -m common
            -p ${icu_DATA_NAME_FULL}
            -s "${CMAKE_BINARY_DIR}/icudata/${icu_DATA_NAME_FULL}"
            "${CMAKE_BINARY_DIR}/icudata/pkg_file_list.txt"
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tool/BinaryToCode.py
            -i "${CMAKE_BINARY_DIR}/icudata/${icu_DATA_NAME_FULL}.dat"
            -n "kIcuDataContent"
            -o "${CMAKE_BINARY_DIR}/icudata/icudata.cpp"
        DEPENDS ${icu_TOOLS} ${icu_PREPARE_TOOLS}
        COMMENT "Running icu data builder" VERBATIM)
    add_library(IcuData STATIC ${icu_DATA_OUTPUT})
endif()

if(LSTG_EARLY_BUILD)
    message(STATUS "[LSTG] DEPS: Return from early build stage")

    # 仅生成 Early build 阶段第三方依赖
    return()
endif()

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
    #PATCH_COMMAND git restore cmake/sdlchecks.cmake CMakeLists.txt
    #COMMAND git apply --ignore-whitespace ${CMAKE_CURRENT_SOURCE_DIR}/patch/sdl2-sdlchecks-patch.patch
    #COMMAND git apply --ignore-whitespace ${CMAKE_CURRENT_SOURCE_DIR}/patch/sdl2-cmake-patch.patch
    OPTIONS
        "SDL2_DISABLE_UNINSTALL ON"
        "SDL_ATOMIC OFF"
        "SDL_RENDER OFF"
        "SDL_HIDAPI OFF"
        "SDL_POWER OFF"
        "SDL_SENSOR OFF"
        "SDL_LOCALE OFF"
        "SDL_MISC OFF"
        "SDL_TEST OFF"
)
if(${sdl2_ADDED})
    add_custom_target(UpdateSDLConfig
        COMMAND
            "${CMAKE_COMMAND}" -E copy_if_different
            "${sdl2_BINARY_DIR}/include-config-$<LOWER_CASE:$<CONFIG>>/SDL2/SDL_config.h"
            "${sdl2_SOURCE_DIR}/include/SDL2/SDL_config.h"
        COMMAND
            "${CMAKE_COMMAND}" -E copy_if_different
            "${sdl2_BINARY_DIR}/include-config-$<LOWER_CASE:$<CONFIG>>/SDL2/SDL_config.h"
            "${sdl2_BINARY_DIR}/include/SDL2/SDL_config.h"
        DEPENDS "${sdl2_BINARY_DIR}/include-config-$<LOWER_CASE:$<CONFIG>>/SDL2/SDL_config.h"
    )
    add_dependencies(SDL2-static UpdateSDLConfig)
endif()

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
    target_link_libraries(harfbuzz-icu harfbuzz icuuc)
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
