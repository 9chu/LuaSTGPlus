### 明确第三方库引用
set(LSTG_CORE_DEPS_PUBLIC fmt liblua-static)
set(LSTG_CORE_DEPS_PRIVATE spdlog SDL2-static imgui zlib-ng)

# 平台相关库
if(LSTG_PLATFORM_MACOS)
    list(APPEND LSTG_CORE_DEPS_PUBLIC "-framework Cocoa")
endif()
if(LSTG_PLATFORM_EMSCRIPTEN)
    list(APPEND LSTG_CORE_DEPS_PUBLIC GL)
endif()

# Diligent
get_supported_backends(DILIGENT_ENGINE_LIBRARIES "static")
list(APPEND DILIGENT_ENGINE_LIBRARIES
    Diligent-Common
    Diligent-GraphicsTools
    Diligent-TargetPlatform
    Diligent-PublicBuildSettings)
message("[LSTG] Diligent libraries: ${DILIGENT_ENGINE_LIBRARIES}")
list(APPEND LSTG_CORE_DEPS_PRIVATE ${DILIGENT_ENGINE_LIBRARIES})

message("[LSTG] Public libraries: ${LSTG_CORE_DEPS_PUBLIC}")
message("[LSTG] Private libraries: ${LSTG_CORE_DEPS_PRIVATE}")

### 目标
# 跨平台源码
file(GLOB_RECURSE LSTG_CORE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../../include/lstg/Core/*.hpp)

# 平台定制代码
if(LSTG_PLATFORM_MACOS)
    list(APPEND LSTG_CORE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/Subsystem/Render/detail/OSX/GLView.mm)
    list(APPEND LSTG_CORE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/Subsystem/Render/detail/OSX/MetalView.mm)
endif()

# 目标
add_library(LuaSTGPlusCore STATIC ${LSTG_CORE_SOURCES})
target_include_directories(LuaSTGPlusCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../include)
target_link_libraries(LuaSTGPlusCore PUBLIC ${LSTG_CORE_DEPS_PUBLIC} PRIVATE ${LSTG_CORE_DEPS_PRIVATE})

# 宏（公开的）
set(LSTG_CORE_DEFS_PUBLIC LSTG_APP_NAME="${LSTG_APP_NAME}")
if(LSTG_SHIPPING)  # Shipping/Development 开关
    list(APPEND LSTG_CORE_DEFS_PUBLIC LSTG_SHIPPING)
else()
    list(APPEND LSTG_CORE_DEFS_PUBLIC LSTG_DEVELOPMENT)
endif()
if(LSTG_PLATFORM_WIN32)
    list(APPEND LSTG_CORE_DEFS_PUBLIC LSTG_PLATFORM_WIN32)
endif()
if(LSTG_PLATFORM_LINUX)
    list(APPEND LSTG_CORE_DEFS_PUBLIC LSTG_PLATFORM_LINUX)
endif()
if(LSTG_PLATFORM_MACOS)
    list(APPEND LSTG_CORE_DEFS_PUBLIC LSTG_PLATFORM_MACOS)
endif()
if(LSTG_PLATFORM_EMSCRIPTEN)
    list(APPEND LSTG_CORE_DEFS_PUBLIC LSTG_PLATFORM_EMSCRIPTEN)
endif()
if(LSTG_PLATFORM_MACOS)  # Apple = MacOS | iOS
    list(APPEND LSTG_CORE_DEFS_PUBLIC LSTG_PLATFORM_APPLE)
endif()
message("[LSTG] Public compiler definitions: ${LSTG_CORE_DEFS_PUBLIC}")
target_compile_definitions(LuaSTGPlusCore PUBLIC ${LSTG_CORE_DEFS_PUBLIC})
