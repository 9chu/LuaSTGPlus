cmake_minimum_required(VERSION 3.16)
project(LuaSTGPlus)

### 工程全局选项
# 请参照帮助定义下述选项

option(LSTG_SHIPPING "Build shipping mode binary" ON)
set(LSTG_APP_NAME "default" CACHE STRING "Specific the app name, will be used as the folder name in AppData for user data storage")

### 检测平台
include(cmake/Platform.cmake)

### 全局编译选项（影响包含第三方库）
set(CMAKE_CXX_STANDARD 17)
if(LSTG_PLATFORM_EMSCRIPTEN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fwasm-exceptions")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fwasm-exceptions")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s USE_PTHREADS=1")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_PTHREADS=1")
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

add_subdirectory(tool/LuaAutoBridgeTool)

add_subdirectory(src/Core)
add_subdirectory(src/v2)

# 调试用目录，不会引入 git 中进行管理
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/DevApp/CMakeLists.txt")
    add_subdirectory(src/DevApp)
endif()
