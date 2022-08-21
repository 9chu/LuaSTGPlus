cmake_minimum_required(VERSION 3.19)
project(LuaSTGPlus)

### 工程全局选项
# 请参照帮助定义下述选项

option(LSTG_SHIPPING "Build shipping mode binary" ON)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    option(LSTG_ENABLE_ASSERTIONS "Enable assertions" ON)
else()
    option(LSTG_ENABLE_ASSERTIONS "Enable assertions" OFF)
endif()
set(LSTG_APP_NAME "default" CACHE STRING "Specific the app name, will be used as the folder name in AppData for user data storage")
option(LSTG_DISABLE_HOT_RELOAD "Disable hot reload support" OFF)
option(LSTG_ASYNC_LOADING_AS_DEFAULT "Enable asynchronous asset loading as default" OFF)
option(LSTG_V2_CMDLINE_INDIRECT_PASS "v2 - Commandline arguments after '--' will be passed to scripts" OFF)
option(LSTG_V2_ENABLE_PRELOAD_PACKAGE_FROM_CMDLINE "v2 - Enable preloading asset package from commandline" OFF)

### 检测平台
include(cmake/Platform.cmake)

### 全局编译选项（影响包含第三方库）
set(CMAKE_CXX_STANDARD 17)
if(LSTG_PLATFORM_EMSCRIPTEN)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fexceptions")
endif()

# https://stackoverflow.com/questions/22140520/how-to-enable-assert-in-cmake-release-mode
if(LSTG_ENABLE_ASSERTIONS)
    if(NOT MSVC)
        add_definitions(-D_DEBUG)
    endif()
    # On non-Debug builds cmake automatically defines NDEBUG, so we explicitly undefine it:
    if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
        # NOTE: use `add_compile_options` rather than `add_definitions` since
        # `add_definitions` does not support generator expressions.
        add_compile_options($<$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>:-UNDEBUG>)

        # Also remove /D NDEBUG to avoid MSVC warnings about conflicting defines.
        foreach (flags_var_to_scrub
            CMAKE_CXX_FLAGS_RELEASE
            CMAKE_CXX_FLAGS_RELWITHDEBINFO
            CMAKE_CXX_FLAGS_MINSIZEREL
            CMAKE_C_FLAGS_RELEASE
            CMAKE_C_FLAGS_RELWITHDEBINFO
            CMAKE_C_FLAGS_MINSIZEREL)
            string (REGEX REPLACE "(^| )[/-]D *NDEBUG($| )" " "
                "${flags_var_to_scrub}" "${${flags_var_to_scrub}}")
        endforeach()
    endif()
endif()

### 第三方依赖
include(cmake/External.cmake)

### 工程配置

# 全局编译选项
if(MSVC)
    add_compile_definitions(_WIN32_WINNT=0x0600 _GNU_SOURCE _CRT_SECURE_NO_WARNINGS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /utf-8")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wextra -Wno-implicit-fallthrough -Wno-unused-parameter -Wno-noexcept-type")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Wextra -Wno-implicit-fallthrough -Wno-unused-parameter -Wno-noexcept-type")
endif()

# 工具
add_subdirectory(tool/LuaAutoBridgeTool)
add_subdirectory(tool/PerfectHashTool)

# 库 & 主程序
add_subdirectory(src/Core)
add_subdirectory(src/v2)

# 调试用目录，不会引入 git 中进行管理
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/DevApp/CMakeLists.txt")
    add_subdirectory(src/DevApp)
endif()
