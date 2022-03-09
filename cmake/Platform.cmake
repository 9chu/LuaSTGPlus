### 平台相关宏定义
# 定义下述平台相关宏
#   LSTG_PLATFORM_WIN32 [TRUE|FALSE]
#   LSTG_PLATFORM_LINUX [TRUE|FALSE]
#   LSTG_PLATFORM_MACOS [TRUE|FALSE]
#   LSTG_PLATFORM_EMSCRIPTEN [TRUE|FALSE]
#   LSTG_PLATFORM_ARCH [32|64]

## 判断平台
set(LSTG_PLATFORM_WIN32 FALSE CACHE INTERNAL "")
set(LSTG_PLATFORM_LINUX FALSE CACHE INTERNAL "")
set(LSTG_PLATFORM_MACOS FALSE CACHE INTERNAL "")
set(LSTG_PLATFORM_EMSCRIPTEN FALSE CACHE INTERNAL "")

if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
    set(LSTG_PLATFORM_ARCH 64 CACHE INTERNAL "64-bit architecture")
else()
    set(LSTG_PLATFORM_ARCH 32 CACHE INTERNAL "32-bit architecture")
endif()

if(WIN32)
    if(${CMAKE_SYSTEM_NAME} STREQUAL "WindowsStore")
        message(FATAL_ERROR "WindowsStore is not supported yet")
    else()
        set(LSTG_PLATFORM_WIN32 TRUE CACHE INTERNAL "Target platform: Win32")
        message("[LSTG] Target platform: Win32. SDK Version: " ${CMAKE_SYSTEM_VERSION})
    endif()
else()
    if(${CMAKE_SYSTEM_NAME} STREQUAL "Android")
        message(FATAL_ERROR "Android is not supported yet")
    elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        set(LSTG_PLATFORM_LINUX TRUE CACHE INTERNAL "Target platform: Linux")
        message("[LSTG] Target Platform: Linux")
    elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
        if(IOS)
            message(FATAL_ERROR "iOS is not supported yet")
        else()
            set(LSTG_PLATFORM_MACOS TRUE CACHE INTERNAL "Target platform: MacOS")
            message("[LSTG] Target Platform: MacOS")
        endif()
    elseif(${CMAKE_SYSTEM_NAME} STREQUAL "iOS")
        message(FATAL_ERROR "iOS is not supported yet")
    elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")
        set(LSTG_PLATFORM_EMSCRIPTEN TRUE CACHE INTERNAL "Target platform: Emscripten")
        message("[LSTG] Target Platform: Emscripten")
    else()
        message(FATAL_ERROR "Unsupported platform")
    endif()
endif(WIN32)
