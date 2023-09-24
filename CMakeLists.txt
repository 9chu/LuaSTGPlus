cmake_minimum_required(VERSION 3.19)
project(LuaSTGPlus)

# 允许某些 IDE 以文件夹的方式组织目标（比如 Visual Studio）
# NOTE: 考虑升级到 CMake 3.26 使之默认打开，https://cmake.org/cmake/help/latest/policy/CMP0143.html
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

### 工程全局选项
# 请参照帮助定义下述选项

option(LSTG_SHIPPING "Build shipping mode binary" ON)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    option(LSTG_ENABLE_ASSERTIONS "Enable assertions" ON)
else()
    option(LSTG_ENABLE_ASSERTIONS "Enable assertions" OFF)
endif()
set(LSTG_APP_NAME "default" CACHE STRING "Specific the app name, will be used as the folder name in AppData for user data storage")
option(LSTG_PARSE_CMDLINE "Determine whether to parse the command line for advanced options" ON)
option(LSTG_DISABLE_HOT_RELOAD "Disable hot reload support" OFF)
option(LSTG_CROSSCOMPILING_EARLY_BUILD "Enable automatically two-stage build when cross-compiling" ON)
set(LSTG_EARLY_BUILD_GENERATOR "" CACHE STRING "Specific generator when in early build stage")

### 检测平台
include(cmake/Platform.cmake)

### Early build stage
# Early build 阶段用于在 Cross-compiling 情形下生成 native toolset
# https://stackoverflow.com/questions/36084785/building-a-tool-immediately-so-it-can-be-used-later-in-same-cmake-run
set(LSTG_EARLY_BUILD OFF CACHE INTERNAL "Enable for first stage build (for internal usage)")
if(CMAKE_CROSSCOMPILING AND LSTG_CROSSCOMPILING_EARLY_BUILD)
    if(NOT LSTG_EARLY_BUILD_COMPLETED)
        message(STATUS "[LSTG] Start early build stage")

        set(LSTG_EARLY_BUILD_WORKDIR "${CMAKE_CURRENT_BINARY_DIR}/NativeBuild/")
        execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory "${LSTG_EARLY_BUILD_WORKDIR}")

        if(LSTG_EARLY_BUILD_GENERATOR STREQUAL "")
            execute_process(COMMAND ${CMAKE_COMMAND}
                -E env "CC=" "CXX=" "LD="  # using host default compiler, FIXME: customize?
                ${CMAKE_COMMAND}
                -DCMAKE_BUILD_TYPE=RelWithDebInfo
                -DLSTG_EARLY_BUILD=ON
                ${CMAKE_CURRENT_SOURCE_DIR}
                WORKING_DIRECTORY "${LSTG_EARLY_BUILD_WORKDIR}"
                RESULT_VARIABLE EXEC_RET)
        else()
            execute_process(COMMAND ${CMAKE_COMMAND}
                -E env "CC=" "CXX=" "LD="  # using host default compiler, FIXME: customize?
                ${CMAKE_COMMAND}
                -G "${LSTG_EARLY_BUILD_GENERATOR}"
                -DCMAKE_BUILD_TYPE=RelWithDebInfo
                -DLSTG_EARLY_BUILD=ON
                ${CMAKE_CURRENT_SOURCE_DIR}
                WORKING_DIRECTORY "${LSTG_EARLY_BUILD_WORKDIR}"
                RESULT_VARIABLE EXEC_RET)
        endif()

        if(EXEC_RET AND NOT EXEC_RET EQUAL 0)
            message(FATAL_ERROR "[LSTG] Generate early build cmake failed")
        endif()

        execute_process(COMMAND ${CMAKE_COMMAND}
            --build . -j
            WORKING_DIRECTORY "${LSTG_EARLY_BUILD_WORKDIR}"
            RESULT_VARIABLE EXEC_RET)
        if(EXEC_RET AND NOT EXEC_RET EQUAL 0)
            message(FATAL_ERROR "[LSTG] Early build failed")
        endif()

        # 设置 Early build 完成
        set(LSTG_EARLY_BUILD_COMPLETED ON CACHE INTERNAL
            "Determine if first stage build finished (for internal usage)")
        set(LSTG_EARLY_BUILD_BINARY_DIR "${LSTG_EARLY_BUILD_WORKDIR}" CACHE INTERNAL
            "First stage build directory (for internal usage)")
    else()
        message(STATUS "[LSTG] Using cached early build")

        set(LSTG_EARLY_BUILD_WORKDIR "${LSTG_EARLY_BUILD_BINARY_DIR}")
    endif()

    # 引用工具
    set(IcuBuildTools_DIR "${LSTG_EARLY_BUILD_WORKDIR}")
endif()

### 全局编译选项（影响包含第三方库）
set(CMAKE_CXX_STANDARD 17)
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
endif()
if(LSTG_PLATFORM_EMSCRIPTEN)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s USE_SDL=0")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions -s USE_SDL=0")
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

### 构建工具
add_subdirectory(tool/BinaryToCode)
add_subdirectory(tool/LuaAutoBridgeTool)
add_subdirectory(tool/PerfectHashTool)

### 第三方依赖
include(cmake/External.cmake)

if(LSTG_EARLY_BUILD)
    # Early build 阶段仅执行到工具生成为止
    return()
endif()

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

# 库 & 主程序
add_subdirectory(src/Core)
add_subdirectory(src/v2)

# 调试用目录，不会引入 git 中进行管理
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/DevApp/CMakeLists.txt")
    add_subdirectory(src/DevApp)
endif()
